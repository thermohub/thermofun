<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Add data to ThermoFun

This page tells you how the data of ThermoFun is arranged, and how you add your own data.

**You need:** ThermoFun built from the branch `v2`, and Python. You do not need other packages.

`pytests/test_add_data_guide.py` runs every example of this page and checks the method tables against the code.

## 1. How the data is arranged

ThermoFun keeps all data in **records**. A record says what a data file says. All values in a record are in SI units.

| Record | What it holds | Key in a file |
|---|---|---|
| Element | molar mass, standard entropy | `elements` |
| Substance | standard-state properties at the reference T and P, and the methods that give them at other T and P | `substances` |
| Reaction | stoichiometry, log K or reaction properties, and their methods | `reactions` |
| Dataset | name, version, conventions, solvent | `thermodataset` |
| Method | your own description of a method, checked against the built-in method | `methods` |
| Parameter set | parameter values that many records share | `parameter_sets` |
| Data source | a citation | `data_sources` |

A **method** is one model, for example `HKF88` or `CpPolynomial`. A substance or a reaction names its methods. The engine
runs them at the T and P that you ask for. Section 9 lists all methods.

Data has one of two formats:

- **v2** is the format for new data. It is YAML or JSON, and it has named values with units. Use it for new data.
- **Legacy** is the JSON format of the ThermoFun 0.x databases. ThermoFun reads it, and you can still add legacy files.

ThermoFun finds the format itself. A file with the key `thermohub` at the top is in the v2 format.

## 2. Choose how to add data

| You have | Do this |
|---|---|
| Many records | Write a v2 file. Load it with `Database("file.yaml")` or `ThermoEngine("file.yaml")`. |
| A second file for a database | Call `database.appendData("file.yaml")`. |
| One new record, as text | Call `database.addSubstance(text)`. It also accepts a `Substance`. The call does not replace a record. |
| One record to replace | Call `database.setSubstance(text)`. |
| A legacy database file | Load it the same way. |
| A dataset in many files | Make a dataset folder, or use `include` (Section 7.1). |
| A record that you change in a program | Use the setters of `Substance`, `Reaction` or `Element`. They edit a copy of the record. |

`addReaction`, `setReaction`, `addElement` and `setElement` do the same for the other records.

The Python record builders are not available yet.

**Save a database.** `database.save("file.yaml")` writes all records in the v2 format, as YAML or as JSON by the name of the file. It also converts a
legacy file to v2:

```python
import thermofun as tf

legacy = tf.Database('{"substances": [{"symbol": "A", "formula": "Ca", "sm_gibbs_energy": {"values": [-56.484001159668]}}]}')
legacy.save("converted.yaml")                    # short decimals, for people
assert "-56.484\n" in open("converted.yaml").read() or "G: -56.484" in open("converted.yaml").read()
exact = tf.Database(legacy.toJson(readable=False))
assert exact.getSubstance("A").record().standard_state_properties[tf.PropertyName.G].value == -56.484001159668
```

`database.save("old.json", format=tf.DataFormat.Legacy)` writes the legacy format. `database.legacyWriteProblems()` lists what that format cannot hold.

By default, a value from a legacy file is written as its short decimal: `-56.484`, not `-56.484001159668`. These values were single-precision
numbers. A database read back from such a file can differ by a small amount from the legacy file: in the bundled databases, at most 0.46 J/mol in G and
3.6e-5 in log K. Use `readable=False` to read back the same numbers.

No information is lost. A short decimal gives the same single-precision number as the long one. A value in another unit, for example cal, is written as the shortest
number that reads back as the same value in SI units. A number in a file always has a point as the decimal mark, in all locales.

## 3. The smallest file

Write this file:

<!-- file: small.yaml -->
```yaml
thermohub: 2.0.0-draft.1
substances:
  Corundum:
    formula: Al2O3
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: -1582000, H: -1675700, S: 50.92, Cp: 79.0, V: 2.558e-5}
    methods:
      - method: ConstantCp
      - method: ConstantVolume
```

Then calculate with it:

```python
engine = tf.ThermoEngine("small.yaml")
result = engine.thermoPropertiesSubstance(373.15, 1e5, "Corundum")
assert result.heat_capacity_cp.val == 79.0                   # J/(mol K)
assert abs(result.gibbs_energy.val + 1586508.58) < 0.01      # J/mol
```

The results of the engine have volumes in J/bar: `result.volume.val` is 2.558 for this record.

A record with no method and no reaction has its reference properties at all T and P.

Add more data to this database:

```python
db = tf.Database("small.yaml")
one = '{"symbol": "Corundum", "formula": "X", "standard_state_properties": {"G": 1}}'
db.addSubstance(one)                                         # the symbol is there: nothing changes
assert db.getSubstance("Corundum").formula() == "Al2O3"
db.setSubstance(one)                                         # the record is replaced
assert db.getSubstance("Corundum").formula() == "X"
```

A record given as text is JSON. It is in the v2 format if it has a key such as `standard_state_properties` or `methods`.

## 4. Write a value

Every number in a file can have one of these forms:

| You write | It means |
|---|---|
| `-237183` | The value in the default unit. |
| `{value: -237.183, unit: kJ/mol}` | The value in the unit that you give. |
| `"-237.183 kJ/mol"` | The same, as text. |
| `{value: -237183, uncertainty: 100}` | A standard uncertainty of 100, in the unit of the value. |
| `{value: -237183, uncertainty: {expanded: 200, coverage_factor: 2}}` | An expanded uncertainty and its coverage factor. |
| `{value: -237183, quality: core, origin: calculated, source: 2018LOT/MAT}` | Data about the value. |

**Default units.** Standard-state properties are in J/mol, J/(mol K) and m3/mol. Reaction properties use the same units. Molar
mass is in g/mol. Temperature is in K. Pressure is in Pa. A method parameter has the SI unit of Section 9.

**Other units.** A `units` block sets the default units of everything inside its level. It can be at the top of a file, in a record,
or in a method. The inner block replaces the outer block for the same dimension. The dimensions are `energy`, `pressure`,
`temperature`, `length`, `mass` and `quantity`.

<!-- file: units.yaml -->
```yaml
thermohub: 2.0.0-draft.1
units: {energy: cal}                 # G, H in cal/mol and S, Cp in cal/(mol K) in all this file
substances:
  Lime:
    formula: CaO
    class: component
    aggregate_state: crystal
    standard_state_properties:
      G: -144400
      H: -151790
      S: 9.5
      Cp: 10.2
      V: {value: 16.76, unit: cm3/mol}
    methods:
      - method: ConstantCp
      - method: ConstantVolume
```

```python
lime = tf.Database("units.yaml").getSubstance("Lime").record()
assert abs(lime.standard_state_properties[tf.PropertyName.G].value + 604169.6) < 1e-6    # J/mol
assert abs(lime.standard_state_properties[tf.PropertyName.V].value - 16.76e-6) < 1e-15   # m3/mol
```

A molar volume can also be in J/bar. That is the unit of the legacy data: 1 J/bar is 1e-5 m3.

**Missing is not zero.** Leave out a value that you do not know. A value of 0 means that the value is known to be 0.

## 5. Write a substance

| Field | What it holds |
|---|---|
| `formula` | The chemical formula, for example `Al2O3` or `Ca+2`. Required by the methods that read the formula. |
| `name`, `sourcetdb` | Free text. |
| `charge` | The formula charge, a whole number. |
| `class` | `component`, `gas_fluid`, `aqueous_solute`, `water_solvent`, `surface_species`, `site_moiety` or `other` (the default). |
| `aggregate_state` | `gas`, `liquid`, `crystal`, `aqueous`, `solid` and more; `other` is the default. An unknown word gives a warning with the allowed words. |
| `molar_mass` | The molar mass. |
| `reference_conditions` | `{T: 298.15, P: 1.0e5}`. The default comes from the dataset, else it is 298.15 K and 1e5 Pa. |
| `standard_state_properties` | `G`, `H`, `S`, `Cp`, `V` at the reference conditions. |
| `methods` | The list of methods. |
| `defining_reaction` | The symbol of a reaction that gives the properties. |
| `preferred_property_source` | `direct` (the methods) or `reaction` (the defining reaction). |
| `applicability` | `{t_min, t_max, p_min, p_max}`. |
| `data_source`, `data_sources` | The symbols of the citations. |
| `covariances` | The correlations between values (Section 8). |
| `structural` | Other named values, for example an ionic radius. |

The **symbol** is the key of the record in the map. In a list of records, each record has the key `symbol`.

### 5.1 Choose the methods

Each substance has **one base method** and any number of corrections. The base method makes all properties. A correction changes
some of them. You can list the methods in any order, because the engine finds the order.

| Substance | Base method | Corrections |
|---|---|---|
| Mineral, simple | `ConstantCp` or `CpPolynomial` | `ConstantVolume`, or `MurnaghanHP98`, `TaitHP11`, `BirchMurnaghan47`, `VolumeBerman88`; `LandauHP98` for a phase transition |
| Mineral, other forms | `NASA9`, `CpExtendedUNIQUAC`, `TPTable` | as above |
| Gas | `CpPolynomial` | `IdealGas`, or an equation of state: `PRSV86`, `PengRobinson78`, `SoaveRedlichKwong72`, `SternerPitzer94`, `ChurakovGottschalk03`, `CorkHP91` |
| Aqueous solute | `HKF88`, `SoluteHP98` or `AndersonEtAl91` | `AkinfievDiamond03` |
| Water, the solvent | `WaterHGK84`, `WaterIAPWS95` or `WaterZhangDuan05` | one dielectric model: `JohnsonNorton91`, `SverjenskyEtAl14` or `FernandezEtAl97` |
| No properties of its own | none | Give `defining_reaction` (Section 5.5). |

A method has this form:

```text
- method: HKF88               # the id of the method
  implementation: gems        # optional, if the method has more than one
  units: {...}                # optional
  parameters: {...}           # or segments, or table
  applicability: {...}        # optional
```

Give only one of `parameters`, `segments` and `table`.

### 5.2 A mineral with a heat capacity in T intervals

`CpPolynomial` has one **segment** for each T interval. Phase transitions go in `transitions`.

<!-- file: mineral.yaml -->
```yaml
thermohub: 2.0.0-draft.1
substances:
  Quartz:
    formula: SiO2
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: -856288, H: -910700, S: 41.46, Cp: 44.59, V: 2.2688e-5}
    methods:
      - method: CpPolynomial
        segments:
          - {t_min: 273.15, t_max: 844, parameters: {a0: 44.603, a1: 0.03778, a2: -1.0018e6}}
          - {t_min: 844, t_max: 2000, parameters: {a0: 58.928, a1: 0.01003}}
        transitions:
          - {T: 844, dS: 0.7, dH: 600, dV: 0.0}
      - method: ConstantVolume
```

```python
quartz = tf.ThermoEngine("mineral.yaml").thermoPropertiesSubstance(500.0, 1e5, "Quartz")
assert abs(quartz.heat_capacity_cp.val - (44.603 + 0.03778 * 500 - 1.0018e6 / 500**2)) < 1e-6
```

### 5.3 An aqueous solute and its solvent

`HKF88` needs the solvent. The solvent is the record with the symbol `H2O@`, or the symbol that `thermodataset: {solvent: ...}`
gives. Parameter values are usually in cal and bar, so give a `units` block in the method.

<!-- file: aqueous.yaml -->
```yaml
thermohub: 2.0.0-draft.1
substances:
  H2O@:
    formula: H2O@
    class: water_solvent
    aggregate_state: aqueous
    standard_state_properties: {G: -237181, H: -285881, S: 69.92, Cp: 75.36, V: {value: 1.807, unit: J/bar}}
    methods:
      - {method: WaterHGK84, implementation: reaktoro}
      - {method: JohnsonNorton91, implementation: reaktoro}
  Ca+2:
    formula: Ca+2
    charge: 2
    class: aqueous_solute
    aggregate_state: aqueous
    standard_state_properties: {G: -552821, H: -543100, S: -56.484, Cp: -30.9225, V: {value: -1.84387, unit: J/bar}}
    methods:
      - method: HKF88
        implementation: reaktoro
        units: {energy: cal, pressure: bar}
        parameters: {a1: -0.01947, a2: -725.2, a3: 5.2966, a4: -24792, c1: 9.0, c2: -25220, omega: 123660}
```

```python
engine = tf.ThermoEngine("aqueous.yaml")
ca = engine.thermoPropertiesSubstance(373.15, 1e6, "Ca+2")
assert engine.explain(373.15, 1e6, "Ca+2")[0]["method"] == "HKF88"
```

**Conventions.** Give `convention` in a record, or in the dataset, if the values are not in the defaults (`benson_helgeson`, `1bar`, `molality`, `geochemical`). For example: `convention: {energy_reference: berman_brown, standard_pressure: 1atm}`.

- `energy_reference`: `benson_helgeson`, `berman_brown`, `ser`, `assigned_enthalpy`, `gibbs_helmholtz_formation`, `dft_formation_0K`.
- `standard_pressure`: `1bar`, `1atm`. It changes gases only.
- `concentration_scale`: `molality`, `mole_fraction`, `molarity`. It changes aqueous solutes only. `molarity` needs the water solvent in the database.
- `water_reference`: `geochemical`, `steam_tables`. It changes the water solvent only.

The engine converts the result of the base method. It does not convert corrections. Species with different conventions can be in one reaction. A change of the energy reference needs the
entropies of the elements. Put them in `elements`. If they are not there, the engine uses the table of ChemicalFun and gives a warning (`CNV-006`). If no source has them, the engine gives an error (`CNV-001`).
`NASA9` is always in `assigned_enthalpy` and `CpExtendedUNIQUAC` in `gibbs_helmholtz_formation`, whatever the record says. Results come in the defaults. To ask for others, set `EngineConventions`.

**The hydrogen ion.** The geochemical convention (`benson_helgeson`, the default) defines all properties of the hydrogen ion as 0, at all T and P. A record with the class
`aqueous_solute` and the formula `H+` needs no methods and no values in this convention. In another convention, the properties of H+ can be different: give `convention` in
the record, or in the dataset, with the values and the methods. ThermoFun then uses them. It also uses values other than 0 in the geochemical convention, and `validate` gives a warning (`CNV-007`).

```python
hydrogen = tf.Database("thermohub: 2.0.0-draft.1\nsubstances:\n  H+: {formula: H+, class: aqueous_solute, aggregate_state: aqueous}\n")
assert tf.ThermoEngine(hydrogen).thermoPropertiesSubstance(400.0, 1e6, "H+").gibbs_energy.val == 0.0
other = tf.Database("""thermohub: 2.0.0-draft.1
substances:
  H+:
    formula: H+
    class: aqueous_solute
    aggregate_state: aqueous
    convention: {energy_reference: ser}
    standard_state_properties: {G: -1000, H: -1000, S: 10, Cp: 20, V: 0}
    methods:
      - {method: ConstantCp}
""")
assert tf.ThermoEngine(other).thermoPropertiesSubstance(298.15, 1e5, "H+").gibbs_energy.val == -1000.0
```

### 5.4 A gas

A record decides by its methods: a fluid equation of state or `IdealGas` makes a gas depend on pressure. The class and the aggregate state of the record do not decide this. Without such a method, a gas gets no pressure term.

By default `G(T, P) = G(T, Pr) + RT ln(P/Pr) + RT ln(phi)`, so `dG/dP = V`. Two preferences of the engine switch off one term each:

| Preference | Default | With the other value |
|---|---|---|
| `apply_pressure_correction_to_gas_props` | `True` | `False`: no `RT ln(P/Pr)` in G, and no `-R ln(P/Pr)` in S. |
| `cancel_fugacity_coefficient_in_gas_props` | `False` | `True`: no `RT ln(phi)` in G, as for an ideal gas. |

With `False` and `True`, G stays G(T, Pr), the standard state. H and S keep the residual terms of the equation of state. The two preferences hold on every path: a gas has the same values alone, as a reactant, and in a substance defined by a reaction.

<!-- file: gas.yaml -->
```yaml
thermohub: 2.0.0-draft.1
substances:
  CO2:
    formula: CO2
    class: gas_fluid
    aggregate_state: gas
    standard_state_properties: {G: -394393, H: -393510, S: 213.785, Cp: 37.135}
    methods:
      - method: CpPolynomial
        segments:
          - {t_min: 273.15, t_max: 2000, parameters: {a0: 87.82, a1: -0.0026, a2: 706400, a3: -998.9}}
      - method: PengRobinson78
        units: {pressure: bar}
        parameters: {Tc: 304.2, Pc: 73.8, omega: 0.225}
```

```python
engine = tf.ThermoEngine("gas.yaml")
low, high = (engine.thermoPropertiesSubstance(400.0, p, "CO2").volume.val for p in (1e5, 1e7))
assert high < low
assert engine.thermoPropertiesSubstance(400.0, 1e7, "CO2").gibbs_energy.val > engine.thermoPropertiesSubstance(400.0, 1e5, "CO2").gibbs_energy.val
engine.preferences.apply_pressure_correction_to_gas_props = False
engine.preferences.cancel_fugacity_coefficient_in_gas_props = True
assert engine.thermoPropertiesSubstance(400.0, 1e7, "CO2").gibbs_energy.val == engine.thermoPropertiesSubstance(400.0, 1e5, "CO2").gibbs_energy.val
```

### 5.5 A substance from a reaction

Give `defining_reaction`, and leave out `methods`. The engine calculates the reaction, and takes the properties of the substance from
it and from the other substances of the reaction. The reaction must name the substance.

The pressure term comes from one place only:

| The reaction | The substance | What happens |
|---|---|---|
| Changes with P: a model of log K with P, a volume method with a volume that is not 0, or its reactants | any | The reaction and the other substances at P. The method of the substance adds no second term. |
| Does not change with P | has a method for the pressure, for example `ConstantVolume`, `IdealGas` or a fluid such as `PengRobinson78` | All at the pressure of the log K data, then that method, so dG/dP = V. This pressure is the largest of the reference pressure, the saturation pressure of water (the critical pressure above the critical temperature) and the pressure at which water has the density `rho_min` of `DataPressure.yaml` (420 kg/m3). So the other substances stay in dense water, also above the critical temperature, and G has no jump. |
| Does not change with P | no such method | The other substances at P. Log K does not depend on P. |

The catalog says for each method if it depends on T, P or both: `tf.methods.catalog()`, key `depends_on`.

<!-- file: defined.yaml -->
```yaml
thermohub: 2.0.0-draft.1
substances:
  A:
    formula: Ca
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: -1000, H: -1000, S: 10, Cp: 20, V: 1.0e-5}
    methods:
      - {method: ConstantCp}
      - {method: ConstantVolume}
  B:
    formula: Ca
    class: component
    aggregate_state: crystal
    defining_reaction: makes-B
reactions:
  makes-B:
    equation: A = B
    reaction_properties: {logK: 1.0, dH: 0, dS: 0, dCp: 0, dV: 0}
    methods:
      - method: LogKConstant
```

```python
engine = tf.ThermoEngine("defined.yaml")
b = engine.thermoPropertiesSubstance(298.15, 1e5, "B")
assert abs(b.gibbs_energy.val - (-1000 - 8.31451 * 298.15 * 2.302585093)) < 1
assert engine.thermoPropertiesSubstanceDetailed(298.15, 1e5, "B")["source"] == "reaction"
```

### 5.6 A table of properties

`TPTable` interpolates in T and P. Values are in order: `value[i + j * nT]` for temperature `i` and pressure `j`.

<!-- file: table.yaml -->
```yaml
thermohub: 2.0.0-draft.1
substances:
  T:
    formula: Ca
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: -1000, H: -900, S: 10, Cp: 20, V: 1.0e-5}
    methods:
      - method: TPTable
        table:
          temperatures: [298.15, 400, 500, 600]
          pressures: [1.0e5]
          G: [-1000, -1100, -1200, -1300]
```

```python
t = tf.ThermoEngine("table.yaml").thermoPropertiesSubstance(400.0, 1e5, "T")
assert abs(t.gibbs_energy.val + 1100) < 1e-6
```

## 6. Write a reaction

| Field | What it holds |
|---|---|
| `equation` | For example `Ca+2 + CO3-2 = Calcite`. |
| `stoichiometry` | `{Calcite: 1, Ca+2: -1, CO3-2: -1}`: below 0 for a reactant. Optional if `equation` is there. If both are there, they must agree. |
| `reaction_properties` | `logK`, `dG`, `dH`, `dS`, `dCp`, `dV` at the reference conditions. |
| `methods` | One base method for log K, and optionally one volume method. |
| `reference_conditions`, `applicability`, `level`, `data_source`, `data_sources`, `covariances` | As for a substance. |

| You have | Base method |
|---|---|
| log K at 25 C only | `LogKConstant` |
| log K and dS = 0 | `LogKZeroEntropy` |
| log K and dH | `LogKVantHoff` |
| log K, dH and dCp | `LogKConstantCp` |
| Coefficients of log K(T) | `LogKPolynomial` |
| A table of log K | `LogKTable`, with `interpolation: lagrange` or `cubic_spline` |
| A density model | `DolejsManning10`, `MarshallFranck81`, `RyzhenkoBryzgalin` |
| Fixed reaction properties | `ConstantReactionProperties` |
| The properties of the substances | No method: the engine calculates the reaction from its substances. |

Add `ReactionConstantVolume` or `ReactionVolumePolynomial` for the effect of pressure. **A reaction without a volume method has no
pressure correction, also if it gives `dV`.**

<!-- file: reaction.yaml -->
```yaml
thermohub: 2.0.0-draft.1
reactions:
  fixed:
    equation: A = B
    reaction_properties: {logK: 2.0, dH: 10000, dCp: 50, dV: 1.0e-5}
    methods:
      - method: LogKConstantCp
      - method: ReactionConstantVolume
  polynomial:
    equation: A = B
    methods:
      - method: LogKPolynomial
        parameters: {a0: 1.0, a2: -300}
  from-a-table:
    stoichiometry: {A: -1, B: 1}
    methods:
      - method: LogKTable
        interpolation: lagrange
        order: 3
        table: {temperatures: [273.15, 298.15, 373.15, 473.15], logK: [1.0, 1.1, 1.3, 1.6]}
```

```python
engine = tf.ThermoEngine("reaction.yaml")
assert abs(engine.thermoPropertiesReaction(298.15, 1e5, "fixed").log_equilibrium_constant.val - 2.0) < 1e-9
assert abs(engine.thermoPropertiesReaction(300.0, 1e5, "polynomial").log_equilibrium_constant.val - 0.0) < 1e-9
assert abs(engine.thermoPropertiesReaction(298.15, 1e5, "from-a-table").log_equilibrium_constant.val - 1.1) < 1e-9
```

The name of a reaction can also be its equation in a calculation: `engine.thermoPropertiesReaction(T, P, "A = B")`. All its substances
must be in the database.

## 7. Elements, dataset and citations

<!-- file: dataset.yaml -->
```yaml
thermohub: 2.0.0-draft.1
thermodataset:
  symbol: my-data
  name: My dataset
  version: 1.0.0
  status: published            # a published dataset must have a version
  solvent: WATER               # the default is the record H2O@
  reference_conditions: {T: 298.15, P: 1.0e5}
  conventions: {water_triple_point_reference: HKF_HGK_1974}
elements:
  Ca: {number: 20, molar_mass: 40.078, standard_entropy: 41.59}      # g/mol, J/(mol K)
  Zz: {class: charge, standard_entropy: -65.34}                      # the charge
data_sources:
  1998HOL/POW: {title: An internally consistent dataset, year: 1998, journal: J. Metamorph. Geol.}
substances:
  WATER:
    formula: H2O@
    class: water_solvent
    aggregate_state: aqueous
    standard_state_properties: {G: -237181}
    methods:
      - {method: WaterHGK84, implementation: reaktoro}
      - {method: JohnsonNorton91, implementation: reaktoro}
    data_source: 1998HOL/POW
```

```python
db = tf.Database("dataset.yaml")
assert db.thermoDataSet().symbol == "my-data"
assert db.dataSources()["1998HOL/POW"].year == 1998
assert tf.ThermoEngine(db).solventSymbol() == "WATER"
assert abs(db.getElement("Ca").molarMass() - 40.078) < 1e-9
```

### 7.1 A dataset in many files

A file can load other files first. The key `include` holds paths from the folder of the file, and `*` and `?` can select many files. A file loads once. A
value in the file replaces a value in a file that it includes.

<!-- file: part.yaml -->
```yaml
thermohub: 2.0.0-draft.1
substances:
  First:
    formula: Ca
    standard_state_properties: {G: -1000}
```

<!-- file: main.yaml -->
```yaml
thermohub: 2.0.0-draft.1
include: [part.yaml]
substances:
  Second:
    formula: Mg
    standard_state_properties: {G: -2000}
```

```python
assert tf.Database("main.yaml").getSubstancesList() == ["First", "Second"]
```

A **dataset folder** holds one dataset. Give the name of the folder to `Database`:

```text
datasets/my-data/1.0.0/
  thermodataset.yaml        required: the file with the key thermodataset
  elements/*.yaml           each file has a key elements, or is one record
  substances/*.yaml         also in folders inside it
  reactions/*.yaml
```

A file that holds one record has the key `record_type`: `element`, `substance`, `reaction`, `method`, `parameter_set` or `data_source`. It does not have the key `thermohub`.
A file can be YAML or JSON.

ThermoFun also looks in the folders above the dataset for the first folder that has `methods/` or `bibliography/`. It loads `methods/definitions`, `methods/parameter_sets` and
`bibliography` from there, before the dataset.

## 8. Uncertainties and correlations

Give the uncertainty with each value (Section 4). Write the correlations between values as a list in the record: `paths` names the
values, and `matrix` is their covariance in SI units.

<!-- file: covariance.yaml -->
```yaml
thermohub: 2.0.0-draft.1
substances:
  X:
    formula: Ca
    class: component
    aggregate_state: crystal
    standard_state_properties: {G: {value: -1000, uncertainty: 2}, H: {value: -1000, uncertainty: 3}, S: 10, Cp: 20, V: 1.0e-5}
    covariances:
      - paths: [/standard_state_properties/G, /standard_state_properties/H]
        matrix: [[4, 1], [1, 9]]
    methods:
      - {method: ConstantCp}
      - {method: ConstantVolume}
```

```python
x = tf.Database("covariance.yaml").getSubstance("X").record()
assert x.covariance[0].matrix == [4.0, 1.0, 1.0, 9.0]
assert x.standard_state_properties[tf.PropertyName.G].uncertainty.standard == 2.0
```

The engine reads the block now. Its calculation with covariance comes with a later step.

## 9. All methods

<!-- methods begin -->
These tables come from the code. A parameter without a unit has no unit. A method can have aliases: old ids stay readable.

### Base methods of a substance

| Id | What it is | Parameters, with the SI unit | Implementations |
|---|---|---|---|
| `ConstantProperties` | Constant properties | none | one |
| `CpPolynomial` | Heat capacity polynomial | `a0` [J/(mol.K)]<br>`a1` [J/(mol.K2)] (optional)<br>`a2` [J.K/mol] (optional)<br>`a3` [J/(mol.K^0.5)] (optional)<br>`a4` [J/(mol.K3)] (optional)<br>`a5` [J/(mol.K4)] (optional)<br>`a6` [J/(mol.K5)] (optional)<br>`a7` [J.K2/mol] (optional)<br>`a8` [J/mol] (optional)<br>`a9` [J/(mol.K^1.5)] (optional) | one |
| `ConstantCp` | Constant heat capacity | none | one |
| `CpExtendedUNIQUAC` | Heat capacity of Extended UNIQUAC | `a` [J/(mol.K)]<br>`b` [J/(mol.K2)]<br>`c` [J/mol]<br>`alpha` [1/Pa]<br>`beta` [1/Pa2]<br>`Theta` [K] (optional) | one |
| `NASA9` | NASA polynomials | `a1`<br>`a2`<br>`a3`<br>`a4`<br>`a5`<br>`a6`<br>`a7`<br>`b1` [K]<br>`b2` | one |
| `TPTable` | Table of T and P | none | one |
| `HKF88` | Revised HKF equation of state (needs the solvent) | `a1` [J/(mol.Pa)]<br>`a2` [J/mol]<br>`a3` [J.K/(mol.Pa)]<br>`a4` [J.K/mol]<br>`c1` [J/(mol.K)]<br>`c2` [J.K/mol]<br>`omega` [J/mol]<br>`eta` (optional)<br>`theta` (optional)<br>`psi` (optional)<br>`Tr` (optional)<br>`Pr` (optional)<br>`Zr` (optional)<br>`Yr` (optional)<br>`zeta` (optional)<br>`gr` (optional)<br>`ag1` (optional)<br>`ag2` (optional)<br>`ag3` (optional)<br>`bg1` (optional)<br>`bg2` (optional)<br>`bg3` (optional)<br>`af1` (optional)<br>`af2` (optional)<br>`af3` (optional)<br>`Dmax` (optional) | `reaktoro`, `gems` |
| `SoluteHP98` | Aqueous solute of Holland and Powell (needs the solvent) | `b` [J/(mol.K2)] | `consistent`, `published` |
| `AndersonEtAl91` | Aqueous solute of Anderson et al. (needs the solvent) | none | one |

### Corrections of a substance

| Id | What it is | Parameters, with the SI unit | Implementations |
|---|---|---|---|
| `AkinfievDiamond03` | Model of Akinfiev and Diamond (needs the solvent) | `xi`<br>`a` [m3/kg]<br>`b` [m3.K^0.5/kg]<br>`MW` (optional) | `gems` |
| `LandauHP98` | Landau transition of Holland and Powell | `Tc0` [K]<br>`Smax` [J/(mol.K)]<br>`Vmax` [m3/mol]<br>`alpha0` [1/K] (optional)<br>`kappa0` [Pa] (optional) | one |
| `LandauHP11` | Landau transition of Holland and Powell (2011) | `Tc0` [K]<br>`Smax` [J/(mol.K)]<br>`Vmax` [m3/mol] | one |
| `ConstantVolume` | Constant molar volume | none | one |
| `VolumeBerman88` | Molar volume of Berman | `a1` [1/K]<br>`a2` [1/K2]<br>`a3` [1/K3]<br>`b1` [1/Pa]<br>`b2` [1/Pa2] | one |
| `VolumeDorogokupets88` | Molar volume of Dorogokupets et al. | `a1` [1/K]<br>`a2` [1/K2]<br>`a3` [1/K3]<br>`b1` [1/Pa]<br>`b2` [1/Pa2] | one |
| `BirchMurnaghan47` | Birch-Murnaghan equation of state | `a1` [1/K]<br>`a2` [1/K2] (optional)<br>`a3` [K] (optional)<br>`kt00` [Pa]<br>`dkdt` [Pa/K] (optional)<br>`kp`<br>`kpp` [1/Pa] (optional) | `gems` |
| `MurnaghanHP98` | Murnaghan equation of Holland and Powell | `alpha0` [1/K]<br>`kappa0` [Pa] | one |
| `TaitHP11` | Tait equation of Holland and Powell | `kappa0p`<br>`kappa0pp` [1/Pa]<br>`n_atoms`<br>`alpha0` [1/K]<br>`kappa0` [Pa] | one |
| `IdealGas` | Ideal gas | none | one |
| `PRSV86` | Peng-Robinson-Stryjek-Vera equation of state | `Tc` [K]<br>`Pc` [Pa]<br>`omega`<br>`k1` (optional)<br>`k2` (optional)<br>`k3` (optional)<br>`k0_0` (optional)<br>`k0_1` (optional)<br>`k0_2` (optional)<br>`k0_3` (optional)<br>`Oa` (optional)<br>`Ob` (optional) | one |
| `ChurakovGottschalk03` | Equation of state of Churakov and Gottschalk | `sigma0`<br>`eps0`<br>`mu0`<br>`pol0`<br>`sigma1`<br>`sigma2`<br>`eps1`<br>`eps2`<br>`mu1`<br>`mu2`<br>`pol1`<br>`pol2`<br>`PISIX` (optional)<br>`TWOPI` (optional)<br>`rm` (optional)<br>`P1` (optional)<br>`PP2` (optional)<br>`P3` (optional)<br>`P4` (optional)<br>`P5` (optional)<br>`P6` (optional)<br>`P7` (optional)<br>`P8` (optional)<br>`P9` (optional)<br>`P10` (optional)<br>`AA1` (optional)<br>`AA2` (optional)<br>`AA3` (optional)<br>`A4` (optional)<br>`A5` (optional)<br>`A6` (optional)<br>`BB1` (optional)<br>`BB2` (optional)<br>`BB3` (optional)<br>`B4` (optional)<br>`B5` (optional)<br>`B6` (optional)<br>`A00` (optional)<br>`A01` (optional)<br>`A10` (optional)<br>`A11` (optional)<br>`A12` (optional)<br>`A21` (optional)<br>`A22` (optional)<br>`A23` (optional)<br>`A31` (optional)<br>`A32` (optional)<br>`A33` (optional)<br>`A34` (optional)<br>`fi1_0` (optional)<br>`fi1_1` (optional)<br>`fi1_2` (optional)<br>`fi1_3` (optional)<br>`fi1_4` (optional)<br>`fi1_5` (optional)<br>`fi1_6` (optional)<br>`fi1_7` (optional)<br>`fi1_8` (optional)<br>`fi1_9` (optional)<br>`fi2_0` (optional)<br>`fi2_1` (optional)<br>`fi2_2` (optional)<br>`fi2_3` (optional)<br>`fi2_4` (optional)<br>`fi2_5` (optional)<br>`fi2_6` (optional)<br>`fi2_7` (optional)<br>`fi2_8` (optional)<br>`fi2_9` (optional)<br>`hs_a3` (optional)<br>`hs_a9` (optional)<br>`hs_a12` (optional)<br>`NA` (optional)<br>`dipole` (optional)<br>`kd0` (optional)<br>`kd1` (optional)<br>`kd2` (optional)<br>`kd3` (optional)<br>`kd4` (optional)<br>`kd5` (optional) | one |
| `SoaveRedlichKwong72` | Soave-Redlich-Kwong equation of state | `Tc` [K]<br>`Pc` [Pa]<br>`omega`<br>`N` (optional)<br>`m0` (optional)<br>`m1` (optional)<br>`m2` (optional)<br>`Oa` (optional)<br>`Ob` (optional) | one |
| `SternerPitzer94` | Equation of state of Sterner and Pitzer | `Tc` [K]<br>`Pc` [Pa]<br>`TMIN` (optional)<br>`TMAX` (optional)<br>`PMIN` (optional)<br>`PMAX` (optional)<br>`cfh_1` (optional)<br>`cfh_2` (optional)<br>`cfh_3` (optional)<br>`cfh_4` (optional)<br>`cfh_5` (optional)<br>`cfh_6` (optional)<br>`cfh_7` (optional)<br>`cfh_8` (optional)<br>`cfh_9` (optional)<br>`cfh_10` (optional)<br>`cfh_11` (optional)<br>`cfh_12` (optional)<br>`cfh_13` (optional)<br>`cfh_14` (optional)<br>`cfh_15` (optional)<br>`cfh_16` (optional)<br>`cfh_17` (optional)<br>`cfh_18` (optional)<br>`cfh_19` (optional)<br>`cfh_20` (optional)<br>`cfh_21` (optional)<br>`cfh_22` (optional)<br>`cfh_23` (optional)<br>`cfh_24` (optional)<br>`cfh_25` (optional)<br>`cfh_26` (optional)<br>`cfh_27` (optional)<br>`cfh_28` (optional)<br>`cfh_29` (optional)<br>`cfh_30` (optional)<br>`cfh_31` (optional)<br>`cfh_32` (optional)<br>`cfh_33` (optional)<br>`cfh_34` (optional)<br>`cfh_35` (optional)<br>`cfh_36` (optional)<br>`cfh_37` (optional)<br>`cfh_38` (optional)<br>`cfh_39` (optional)<br>`cfh_40` (optional)<br>`cfh_41` (optional)<br>`cfh_42` (optional)<br>`cfh_43` (optional)<br>`cfh_44` (optional)<br>`cfh_45` (optional)<br>`cfh_46` (optional)<br>`cfh_47` (optional)<br>`cfh_48` (optional)<br>`cfh_49` (optional)<br>`cfh_50` (optional)<br>`cfh_51` (optional)<br>`cfh_52` (optional)<br>`cfh_53` (optional)<br>`cfh_54` (optional)<br>`cfh_55` (optional)<br>`cfh_56` (optional)<br>`cfh_57` (optional)<br>`cfh_58` (optional)<br>`cfh_59` (optional)<br>`cfh_60` (optional)<br>`cfc_1` (optional)<br>`cfc_2` (optional)<br>`cfc_3` (optional)<br>`cfc_4` (optional)<br>`cfc_5` (optional)<br>`cfc_6` (optional)<br>`cfc_7` (optional)<br>`cfc_8` (optional)<br>`cfc_9` (optional)<br>`cfc_10` (optional)<br>`cfc_11` (optional)<br>`cfc_12` (optional)<br>`cfc_13` (optional)<br>`cfc_14` (optional)<br>`cfc_15` (optional)<br>`cfc_16` (optional)<br>`cfc_17` (optional)<br>`cfc_18` (optional)<br>`cfc_19` (optional)<br>`cfc_20` (optional)<br>`cfc_21` (optional)<br>`cfc_22` (optional)<br>`cfc_23` (optional)<br>`cfc_24` (optional)<br>`cfc_25` (optional)<br>`cfc_26` (optional)<br>`cfc_27` (optional)<br>`cfc_28` (optional)<br>`cfc_29` (optional)<br>`cfc_30` (optional)<br>`cfc_31` (optional)<br>`cfc_32` (optional)<br>`cfc_33` (optional)<br>`cfc_34` (optional)<br>`cfc_35` (optional)<br>`cfc_36` (optional)<br>`cfc_37` (optional)<br>`cfc_38` (optional)<br>`cfc_39` (optional)<br>`cfc_40` (optional)<br>`cfc_41` (optional)<br>`cfc_42` (optional)<br>`cfc_43` (optional)<br>`cfc_44` (optional)<br>`cfc_45` (optional)<br>`cfc_46` (optional)<br>`cfc_47` (optional)<br>`cfc_48` (optional)<br>`cfc_49` (optional)<br>`cfc_50` (optional)<br>`cfc_51` (optional)<br>`cfc_52` (optional)<br>`cfc_53` (optional)<br>`cfc_54` (optional)<br>`cfc_55` (optional)<br>`cfc_56` (optional)<br>`cfc_57` (optional)<br>`cfc_58` (optional)<br>`cfc_59` (optional)<br>`cfc_60` (optional)<br>`cs_a0` (optional)<br>`cs_a1` (optional)<br>`cs_b0` (optional)<br>`cs_c0` (optional)<br>`cs_c1` (optional)<br>`cs_d0` (optional)<br>`cs_d1` (optional)<br>`psat_co2_Rhoc` (optional)<br>`psat_co2_a1` (optional)<br>`psat_co2_a2` (optional)<br>`psat_co2_a3` (optional)<br>`psat_co2_a4` (optional)<br>`psat_co2_b1` (optional)<br>`psat_co2_b2` (optional)<br>`psat_co2_b3` (optional)<br>`psat_co2_b4` (optional)<br>`psat_co2_c1` (optional)<br>`psat_co2_c2` (optional)<br>`psat_co2_c3` (optional)<br>`psat_co2_c4` (optional)<br>`psat_co2_c5` (optional)<br>`psat_h2o_Rhoc` (optional)<br>`psat_h2o_a1` (optional)<br>`psat_h2o_a2` (optional)<br>`psat_h2o_a3` (optional)<br>`psat_h2o_a4` (optional)<br>`psat_h2o_a5` (optional)<br>`psat_h2o_a6` (optional)<br>`psat_h2o_b1` (optional)<br>`psat_h2o_b2` (optional)<br>`psat_h2o_b3` (optional)<br>`psat_h2o_b4` (optional)<br>`psat_h2o_b5` (optional)<br>`psat_h2o_b6` (optional)<br>`psat_h2o_c1` (optional)<br>`psat_h2o_c2` (optional)<br>`psat_h2o_c3` (optional)<br>`psat_h2o_c4` (optional)<br>`psat_h2o_c5` (optional)<br>`psat_h2o_c6` (optional)<br>`Mw_h2o` (optional)<br>`Mw_co2` (optional) | one |
| `PengRobinson78` | Peng-Robinson equation of state | `Tc` [K]<br>`Pc` [Pa]<br>`omega`<br>`N` (optional)<br>`k1_0` (optional)<br>`k1_1` (optional)<br>`k1_2` (optional)<br>`k2_0` (optional)<br>`k2_1` (optional)<br>`k2_2` (optional)<br>`k2_3` (optional)<br>`omega_switch` (optional)<br>`Oa` (optional)<br>`Ob` (optional) | one |
| `CorkHP91` | Compensated Redlich-Kwong equation of state | `Tc` [K]<br>`Pc` [Pa]<br>`omega`<br>`k1` (optional)<br>`k2` (optional)<br>`k3` (optional)<br>`cs_a0` (optional)<br>`cs_a1` (optional)<br>`cs_b0` (optional)<br>`cs_c0` (optional)<br>`cs_c1` (optional)<br>`cs_d0` (optional)<br>`cs_d1` (optional)<br>`co2_a0` (optional)<br>`co2_a1` (optional)<br>`co2_a2` (optional)<br>`co2_b` (optional)<br>`co2_c0` (optional)<br>`co2_c1` (optional)<br>`co2_d0` (optional)<br>`co2_d1` (optional)<br>`co2_e0` (optional)<br>`co2_p0` (optional)<br>`h2o_a0` (optional)<br>`h2o_a1` (optional)<br>`h2o_a2` (optional)<br>`h2o_a3` (optional)<br>`h2o_a4` (optional)<br>`h2o_a5` (optional)<br>`h2o_a6` (optional)<br>`h2o_a7` (optional)<br>`h2o_a8` (optional)<br>`h2o_a9` (optional)<br>`h2o_b` (optional)<br>`h2o_c0` (optional)<br>`h2o_c1` (optional)<br>`h2o_d0` (optional)<br>`h2o_d1` (optional)<br>`h2o_e0` (optional)<br>`h2o_p0` (optional)<br>`h2o_psat0` (optional)<br>`h2o_psat2` (optional)<br>`h2o_psat3` (optional)<br>`h2o_psat5` (optional)<br>`h2o_T0` (optional)<br>`h2o_Tsuper` (optional) | one |

### Equations of state of water

| Id | What it is | Parameters, with the SI unit | Implementations |
|---|---|---|---|
| `WaterHGK84` | Water, Haar-Gallagher-Kell | `A0_1` (optional)<br>`A0_2` (optional)<br>`A0_3` (optional)<br>`A0_4` (optional)<br>`A0_5` (optional)<br>`A0_6` (optional)<br>`A0_7` (optional)<br>`A0_8` (optional)<br>`A0_9` (optional)<br>`A0_10` (optional)<br>`A0_11` (optional)<br>`A0_12` (optional)<br>`A0_13` (optional)<br>`A0_14` (optional)<br>`A0_15` (optional)<br>`A0_16` (optional)<br>`A0_17` (optional)<br>`A0_18` (optional)<br>`A1_1` (optional)<br>`A1_2` (optional)<br>`A1_3` (optional)<br>`A1_4` (optional)<br>`A1_5` (optional)<br>`yc_1` (optional)<br>`yc_2` (optional)<br>`yc_3` (optional)<br>`yc_4` (optional)<br>`ki_1` (optional)<br>`ki_2` (optional)<br>`ki_3` (optional)<br>`ki_4` (optional)<br>`ki_5` (optional)<br>`ki_6` (optional)<br>`ki_7` (optional)<br>`ki_8` (optional)<br>`ki_9` (optional)<br>`ki_10` (optional)<br>`ki_11` (optional)<br>`ki_12` (optional)<br>`ki_13` (optional)<br>`ki_14` (optional)<br>`ki_15` (optional)<br>`ki_16` (optional)<br>`ki_17` (optional)<br>`ki_18` (optional)<br>`ki_19` (optional)<br>`ki_20` (optional)<br>`ki_21` (optional)<br>`ki_22` (optional)<br>`ki_23` (optional)<br>`ki_24` (optional)<br>`ki_25` (optional)<br>`ki_26` (optional)<br>`ki_27` (optional)<br>`ki_28` (optional)<br>`ki_29` (optional)<br>`ki_30` (optional)<br>`ki_31` (optional)<br>`ki_32` (optional)<br>`ki_33` (optional)<br>`ki_34` (optional)<br>`ki_35` (optional)<br>`ki_36` (optional)<br>`li_1` (optional)<br>`li_2` (optional)<br>`li_3` (optional)<br>`li_4` (optional)<br>`li_5` (optional)<br>`li_6` (optional)<br>`li_7` (optional)<br>`li_8` (optional)<br>`li_9` (optional)<br>`li_10` (optional)<br>`li_11` (optional)<br>`li_12` (optional)<br>`li_13` (optional)<br>`li_14` (optional)<br>`li_15` (optional)<br>`li_16` (optional)<br>`li_17` (optional)<br>`li_18` (optional)<br>`li_19` (optional)<br>`li_20` (optional)<br>`li_21` (optional)<br>`li_22` (optional)<br>`li_23` (optional)<br>`li_24` (optional)<br>`li_25` (optional)<br>`li_26` (optional)<br>`li_27` (optional)<br>`li_28` (optional)<br>`li_29` (optional)<br>`li_30` (optional)<br>`li_31` (optional)<br>`li_32` (optional)<br>`li_33` (optional)<br>`li_34` (optional)<br>`li_35` (optional)<br>`li_36` (optional)<br>`A3_1` (optional)<br>`A3_2` (optional)<br>`A3_3` (optional)<br>`A3_4` (optional)<br>`A3_5` (optional)<br>`A3_6` (optional)<br>`A3_7` (optional)<br>`A3_8` (optional)<br>`A3_9` (optional)<br>`A3_10` (optional)<br>`A3_11` (optional)<br>`A3_12` (optional)<br>`A3_13` (optional)<br>`A3_14` (optional)<br>`A3_15` (optional)<br>`A3_16` (optional)<br>`A3_17` (optional)<br>`A3_18` (optional)<br>`A3_19` (optional)<br>`A3_20` (optional)<br>`A3_21` (optional)<br>`A3_22` (optional)<br>`A3_23` (optional)<br>`A3_24` (optional)<br>`A3_25` (optional)<br>`A3_26` (optional)<br>`A3_27` (optional)<br>`A3_28` (optional)<br>`A3_29` (optional)<br>`A3_30` (optional)<br>`A3_31` (optional)<br>`A3_32` (optional)<br>`A3_33` (optional)<br>`A3_34` (optional)<br>`A3_35` (optional)<br>`A3_36` (optional)<br>`mi_1` (optional)<br>`mi_2` (optional)<br>`mi_3` (optional)<br>`mi_4` (optional)<br>`ni_1` (optional)<br>`ni_2` (optional)<br>`ni_3` (optional)<br>`ni_4` (optional)<br>`alpha_1` (optional)<br>`alpha_2` (optional)<br>`alpha_3` (optional)<br>`alpha_4` (optional)<br>`beta_1` (optional)<br>`beta_2` (optional)<br>`beta_3` (optional)<br>`beta_4` (optional)<br>`ri_1` (optional)<br>`ri_2` (optional)<br>`ri_3` (optional)<br>`ri_4` (optional)<br>`ti_1` (optional)<br>`ti_2` (optional)<br>`ti_3` (optional)<br>`ti_4` (optional)<br>`A4_1` (optional)<br>`A4_2` (optional)<br>`A4_3` (optional)<br>`A4_4` (optional)<br>`sat_a1` (optional)<br>`sat_a2` (optional)<br>`sat_a3` (optional)<br>`sat_a4` (optional)<br>`sat_a5` (optional)<br>`sat_a6` (optional)<br>`sat_b1` (optional)<br>`sat_b2` (optional)<br>`sat_b3` (optional)<br>`sat_b4` (optional)<br>`sat_b5` (optional)<br>`sat_b6` (optional)<br>`sat_c1` (optional)<br>`sat_c2` (optional)<br>`sat_c3` (optional)<br>`sat_c4` (optional)<br>`sat_c5` (optional)<br>`sat_c6` (optional)<br>`Tcr` (optional)<br>`Pcr` (optional)<br>`Dcr` (optional)<br>`M` (optional)<br>`Ttr` (optional)<br>`referenceTemperature` (optional)<br>`referenceDensity` (optional)<br>`referenceHelmholtz` (optional)<br>`A20` (optional)<br>`z0` (optional)<br>`visc_Pmax` (optional)<br>`visc_P2` (optional)<br>`visc_T2` (optional)<br>`visc_P3` (optional)<br>`visc_T3` (optional)<br>`visc_Tmax` (optional)<br>`cond_Pmax` (optional)<br>`cond_P2` (optional)<br>`cond_T2` (optional)<br>`cond_P3` (optional)<br>`cond_T3` (optional)<br>`cond_Tmax` (optional)<br>`surf_Ttripl` (optional)<br>`surf_Tcrit` (optional)<br>`surf_Tstar` (optional)<br>`surf_Tcstar` (optional)<br>`surf_v` (optional)<br>`surf_B` (optional)<br>`surf_stref` (optional)<br>`born_TMAX` (optional)<br>`born_PMAX` (optional)<br>`trans_Tstar` (optional)<br>`trans_Dstar` (optional)<br>`trans_Pstar` (optional)<br>`trans_ustar` (optional)<br>`cond_C` (optional)<br>`crit_T` (optional)<br>`crit_P` (optional)<br>`ac_wm` (optional)<br>`ac_gascon` (optional)<br>`ac_tz` (optional)<br>`ac_aa` (optional)<br>`ac_uref` (optional)<br>`ac_sref` (optional)<br>`ac_zb` (optional)<br>`ac_dzb` (optional)<br>`ac_yb` (optional)<br>`nc_nc` (optional)<br>`el_g1` (optional)<br>`el_g2` (optional)<br>`el_gf` (optional)<br>`el_b1` (optional)<br>`el_b2` (optional)<br>`el_b1t` (optional)<br>`el_b2t` (optional)<br>`el_b1tt` (optional)<br>`el_b2tt` (optional)<br>`hc_tcHGK` (optional)<br>`hc_dcHGK` (optional)<br>`hc_pcHGK` (optional)<br>`hb_Ttop` (optional)<br>`hb_Tbtm` (optional)<br>`hb_Ptop` (optional)<br>`hb_Pbtm` (optional)<br>`hb_Dtop` (optional)<br>`hb_Dbtm` (optional)<br>`li_sDli1` (optional)<br>`li_sPli1` (optional)<br>`li_sDli37` (optional)<br>`li_sPli37` (optional)<br>`li_sDIB30` (optional)<br>`li_Tli13` (optional)<br>`li_Pli13` (optional)<br>`li_Dli13` (optional)<br>`li_TnIB30` (optional)<br>`li_DnIB30` (optional)<br>`tt_Utri` (optional)<br>`tt_Stri` (optional)<br>`tt_Htri` (optional)<br>`tt_Atri` (optional)<br>`tt_Gtri` (optional)<br>`tt_Ttr` (optional)<br>`tt_Ptripl` (optional)<br>`tt_Dltrip` (optional)<br>`tt_Dvtrip` (optional)<br>`cr_Tc` (optional)<br>`cr_rhoC` (optional)<br>`cr_Pc` (optional)<br>`cr_Pcon` (optional)<br>`cr_Ucon` (optional)<br>`cr_Scon` (optional)<br>`cr_dPcon` (optional)<br>`nc_g_1` (optional)<br>`nc_g_2` (optional)<br>`nc_g_3` (optional)<br>`nc_g_4` (optional)<br>`nc_g_5` (optional)<br>`nc_g_6` (optional)<br>`nc_g_7` (optional)<br>`nc_g_8` (optional)<br>`nc_g_9` (optional)<br>`nc_g_10` (optional)<br>`nc_g_11` (optional)<br>`nc_g_12` (optional)<br>`nc_g_13` (optional)<br>`nc_g_14` (optional)<br>`nc_g_15` (optional)<br>`nc_g_16` (optional)<br>`nc_g_17` (optional)<br>`nc_g_18` (optional)<br>`nc_g_19` (optional)<br>`nc_g_20` (optional)<br>`nc_g_21` (optional)<br>`nc_g_22` (optional)<br>`nc_g_23` (optional)<br>`nc_g_24` (optional)<br>`nc_g_25` (optional)<br>`nc_g_26` (optional)<br>`nc_g_27` (optional)<br>`nc_g_28` (optional)<br>`nc_g_29` (optional)<br>`nc_g_30` (optional)<br>`nc_g_31` (optional)<br>`nc_g_32` (optional)<br>`nc_g_33` (optional)<br>`nc_g_34` (optional)<br>`nc_g_35` (optional)<br>`nc_g_36` (optional)<br>`nc_g_37` (optional)<br>`nc_g_38` (optional)<br>`nc_g_39` (optional)<br>`nc_g_40` (optional)<br>`nc_ii_1` (optional)<br>`nc_ii_2` (optional)<br>`nc_ii_3` (optional)<br>`nc_ii_4` (optional)<br>`nc_ii_5` (optional)<br>`nc_ii_6` (optional)<br>`nc_ii_7` (optional)<br>`nc_ii_8` (optional)<br>`nc_ii_9` (optional)<br>`nc_ii_10` (optional)<br>`nc_ii_11` (optional)<br>`nc_ii_12` (optional)<br>`nc_ii_13` (optional)<br>`nc_ii_14` (optional)<br>`nc_ii_15` (optional)<br>`nc_ii_16` (optional)<br>`nc_ii_17` (optional)<br>`nc_ii_18` (optional)<br>`nc_ii_19` (optional)<br>`nc_ii_20` (optional)<br>`nc_ii_21` (optional)<br>`nc_ii_22` (optional)<br>`nc_ii_23` (optional)<br>`nc_ii_24` (optional)<br>`nc_ii_25` (optional)<br>`nc_ii_26` (optional)<br>`nc_ii_27` (optional)<br>`nc_ii_28` (optional)<br>`nc_ii_29` (optional)<br>`nc_ii_30` (optional)<br>`nc_ii_31` (optional)<br>`nc_ii_32` (optional)<br>`nc_ii_33` (optional)<br>`nc_ii_34` (optional)<br>`nc_ii_35` (optional)<br>`nc_ii_36` (optional)<br>`nc_ii_37` (optional)<br>`nc_ii_38` (optional)<br>`nc_ii_39` (optional)<br>`nc_ii_40` (optional)<br>`nc_jj_1` (optional)<br>`nc_jj_2` (optional)<br>`nc_jj_3` (optional)<br>`nc_jj_4` (optional)<br>`nc_jj_5` (optional)<br>`nc_jj_6` (optional)<br>`nc_jj_7` (optional)<br>`nc_jj_8` (optional)<br>`nc_jj_9` (optional)<br>`nc_jj_10` (optional)<br>`nc_jj_11` (optional)<br>`nc_jj_12` (optional)<br>`nc_jj_13` (optional)<br>`nc_jj_14` (optional)<br>`nc_jj_15` (optional)<br>`nc_jj_16` (optional)<br>`nc_jj_17` (optional)<br>`nc_jj_18` (optional)<br>`nc_jj_19` (optional)<br>`nc_jj_20` (optional)<br>`nc_jj_21` (optional)<br>`nc_jj_22` (optional)<br>`nc_jj_23` (optional)<br>`nc_jj_24` (optional)<br>`nc_jj_25` (optional)<br>`nc_jj_26` (optional)<br>`nc_jj_27` (optional)<br>`nc_jj_28` (optional)<br>`nc_jj_29` (optional)<br>`nc_jj_30` (optional)<br>`nc_jj_31` (optional)<br>`nc_jj_32` (optional)<br>`nc_jj_33` (optional)<br>`nc_jj_34` (optional)<br>`nc_jj_35` (optional)<br>`nc_jj_36` (optional)<br>`nc_jj_37` (optional)<br>`nc_jj_38` (optional)<br>`nc_jj_39` (optional)<br>`nc_jj_40` (optional)<br>`bc_bp_1` (optional)<br>`bc_bp_2` (optional)<br>`bc_bp_3` (optional)<br>`bc_bp_4` (optional)<br>`bc_bp_5` (optional)<br>`bc_bp_6` (optional)<br>`bc_bp_7` (optional)<br>`bc_bp_8` (optional)<br>`bc_bp_9` (optional)<br>`bc_bp_10` (optional)<br>`bc_bq_1` (optional)<br>`bc_bq_2` (optional)<br>`bc_bq_3` (optional)<br>`bc_bq_4` (optional)<br>`bc_bq_5` (optional)<br>`bc_bq_6` (optional)<br>`bc_bq_7` (optional)<br>`bc_bq_8` (optional)<br>`bc_bq_9` (optional)<br>`bc_bq_10` (optional)<br>`ad_atz_1` (optional)<br>`ad_atz_2` (optional)<br>`ad_atz_3` (optional)<br>`ad_atz_4` (optional)<br>`ad_adz_1` (optional)<br>`ad_adz_2` (optional)<br>`ad_adz_3` (optional)<br>`ad_adz_4` (optional)<br>`ad_aat_1` (optional)<br>`ad_aat_2` (optional)<br>`ad_aat_3` (optional)<br>`ad_aat_4` (optional)<br>`ad_aad_1` (optional)<br>`ad_aad_2` (optional)<br>`ad_aad_3` (optional)<br>`ad_aad_4` (optional)<br>`co_a_1` (optional)<br>`co_a_2` (optional)<br>`co_a_3` (optional)<br>`co_a_4` (optional)<br>`co_a_5` (optional)<br>`co_a_6` (optional)<br>`co_a_7` (optional)<br>`co_a_8` (optional)<br>`co_a_9` (optional)<br>`co_a_10` (optional)<br>`co_a_11` (optional)<br>`co_a_12` (optional)<br>`co_a_13` (optional)<br>`co_a_14` (optional)<br>`co_a_15` (optional)<br>`co_a_16` (optional)<br>`co_a_17` (optional)<br>`co_a_18` (optional)<br>`co_a_19` (optional)<br>`co_a_20` (optional)<br>`co_q_1` (optional)<br>`co_q_2` (optional)<br>`co_q_3` (optional)<br>`co_q_4` (optional)<br>`co_q_5` (optional)<br>`co_q_6` (optional)<br>`co_q_7` (optional)<br>`co_q_8` (optional)<br>`co_q_9` (optional)<br>`co_q_10` (optional)<br>`co_q_11` (optional)<br>`co_q_12` (optional)<br>`co_q_13` (optional)<br>`co_q_14` (optional)<br>`co_q_15` (optional)<br>`co_q_16` (optional)<br>`co_q_17` (optional)<br>`co_q_18` (optional)<br>`co_q_19` (optional)<br>`co_q_20` (optional)<br>`co_x_1` (optional)<br>`co_x_2` (optional)<br>`co_x_3` (optional)<br>`co_x_4` (optional)<br>`co_x_5` (optional)<br>`co_x_6` (optional)<br>`co_x_7` (optional)<br>`co_x_8` (optional)<br>`co_x_9` (optional)<br>`co_x_10` (optional)<br>`co_x_11` (optional)<br>`psat_a_1` (optional)<br>`psat_a_2` (optional)<br>`psat_a_3` (optional)<br>`psat_a_4` (optional)<br>`psat_a_5` (optional)<br>`psat_a_6` (optional)<br>`psat_a_7` (optional)<br>`psat_a_8` (optional)<br>`ideal_c_1` (optional)<br>`ideal_c_2` (optional)<br>`ideal_c_3` (optional)<br>`ideal_c_4` (optional)<br>`ideal_c_5` (optional)<br>`ideal_c_6` (optional)<br>`ideal_c_7` (optional)<br>`ideal_c_8` (optional)<br>`ideal_c_9` (optional)<br>`ideal_c_10` (optional)<br>`ideal_c_11` (optional)<br>`ideal_c_12` (optional)<br>`ideal_c_13` (optional)<br>`ideal_c_14` (optional)<br>`ideal_c_15` (optional)<br>`ideal_c_16` (optional)<br>`ideal_c_17` (optional)<br>`ideal_c_18` (optional)<br>`visc_a_1` (optional)<br>`visc_a_2` (optional)<br>`visc_a_3` (optional)<br>`visc_a_4` (optional)<br>`visc_b_1` (optional)<br>`visc_b_2` (optional)<br>`visc_b_3` (optional)<br>`visc_b_4` (optional)<br>`visc_b_5` (optional)<br>`visc_b_6` (optional)<br>`visc_b_7` (optional)<br>`visc_b_8` (optional)<br>`visc_b_9` (optional)<br>`visc_b_10` (optional)<br>`visc_b_11` (optional)<br>`visc_b_12` (optional)<br>`visc_b_13` (optional)<br>`visc_b_14` (optional)<br>`visc_b_15` (optional)<br>`visc_b_16` (optional)<br>`visc_b_17` (optional)<br>`visc_b_18` (optional)<br>`visc_b_19` (optional)<br>`visc_b_20` (optional)<br>`visc_b_21` (optional)<br>`visc_b_22` (optional)<br>`visc_b_23` (optional)<br>`visc_b_24` (optional)<br>`visc_b_25` (optional)<br>`visc_b_26` (optional)<br>`visc_b_27` (optional)<br>`visc_b_28` (optional)<br>`visc_b_29` (optional)<br>`visc_b_30` (optional)<br>`visc_b_31` (optional)<br>`visc_b_32` (optional)<br>`visc_b_33` (optional)<br>`visc_b_34` (optional)<br>`visc_b_35` (optional)<br>`visc_b_36` (optional)<br>`visc_b_37` (optional)<br>`visc_b_38` (optional)<br>`visc_b_39` (optional)<br>`visc_b_40` (optional)<br>`visc_b_41` (optional)<br>`visc_b_42` (optional)<br>`cond_aL_1` (optional)<br>`cond_aL_2` (optional)<br>`cond_aL_3` (optional)<br>`cond_aL_4` (optional)<br>`cond_au_1` (optional)<br>`cond_au_2` (optional)<br>`cond_au_3` (optional)<br>`cond_au_4` (optional)<br>`cond_bL_1` (optional)<br>`cond_bL_2` (optional)<br>`cond_bL_3` (optional)<br>`cond_bL_4` (optional)<br>`cond_bL_5` (optional)<br>`cond_bL_6` (optional)<br>`cond_bL_7` (optional)<br>`cond_bL_8` (optional)<br>`cond_bL_9` (optional)<br>`cond_bL_10` (optional)<br>`cond_bL_11` (optional)<br>`cond_bL_12` (optional)<br>`cond_bL_13` (optional)<br>`cond_bL_14` (optional)<br>`cond_bL_15` (optional)<br>`cond_bL_16` (optional)<br>`cond_bL_17` (optional)<br>`cond_bL_18` (optional)<br>`cond_bL_19` (optional)<br>`cond_bL_20` (optional)<br>`cond_bL_21` (optional)<br>`cond_bL_22` (optional)<br>`cond_bL_23` (optional)<br>`cond_bL_24` (optional)<br>`cond_bL_25` (optional)<br>`cond_bL_26` (optional)<br>`cond_bL_27` (optional)<br>`cond_bL_28` (optional)<br>`cond_bL_29` (optional)<br>`cond_bL_30` (optional)<br>`cond_bu_1` (optional)<br>`cond_bu_2` (optional)<br>`cond_bu_3` (optional)<br>`cond_bu_4` (optional)<br>`cond_bu_5` (optional)<br>`cond_bu_6` (optional)<br>`cond_bu_7` (optional)<br>`cond_bu_8` (optional)<br>`cond_bu_9` (optional)<br>`cond_bu_10` (optional)<br>`cond_bu_11` (optional)<br>`cond_bu_12` (optional)<br>`cond_bu_13` (optional)<br>`cond_bu_14` (optional)<br>`cond_bu_15` (optional)<br>`cond_bu_16` (optional)<br>`cond_bu_17` (optional)<br>`cond_bu_18` (optional)<br>`cond_bu_19` (optional)<br>`cond_bu_20` (optional)<br>`cond_bu_21` (optional)<br>`cond_bu_22` (optional)<br>`cond_bu_23` (optional)<br>`cond_bu_24` (optional)<br>`cond_bu_25` (optional)<br>`cond_bu_26` (optional)<br>`cond_bu_27` (optional)<br>`cond_bu_28` (optional)<br>`cond_bu_29` (optional)<br>`cond_bu_30` (optional)<br>`state` (optional) | `reaktoro`, `gems` |
| `WaterIAPWS95` | Water, IAPWS-95 | `sat_a1` (optional)<br>`sat_a2` (optional)<br>`sat_a3` (optional)<br>`sat_a4` (optional)<br>`sat_a5` (optional)<br>`sat_a6` (optional)<br>`sat_b1` (optional)<br>`sat_b2` (optional)<br>`sat_b3` (optional)<br>`sat_b4` (optional)<br>`sat_b5` (optional)<br>`sat_b6` (optional)<br>`sat_c1` (optional)<br>`sat_c2` (optional)<br>`sat_c3` (optional)<br>`sat_c4` (optional)<br>`sat_c5` (optional)<br>`sat_c6` (optional)<br>`Tcr` (optional)<br>`Pcr` (optional)<br>`Dcr` (optional)<br>`M` (optional)<br>`Ttr` (optional)<br>`no_1` (optional)<br>`no_2` (optional)<br>`no_3` (optional)<br>`no_4` (optional)<br>`no_5` (optional)<br>`no_6` (optional)<br>`no_7` (optional)<br>`no_8` (optional)<br>`no_9` (optional)<br>`gammao_1` (optional)<br>`gammao_2` (optional)<br>`gammao_3` (optional)<br>`gammao_4` (optional)<br>`gammao_5` (optional)<br>`c_1` (optional)<br>`c_2` (optional)<br>`c_3` (optional)<br>`c_4` (optional)<br>`c_5` (optional)<br>`c_6` (optional)<br>`c_7` (optional)<br>`c_8` (optional)<br>`c_9` (optional)<br>`c_10` (optional)<br>`c_11` (optional)<br>`c_12` (optional)<br>`c_13` (optional)<br>`c_14` (optional)<br>`c_15` (optional)<br>`c_16` (optional)<br>`c_17` (optional)<br>`c_18` (optional)<br>`c_19` (optional)<br>`c_20` (optional)<br>`c_21` (optional)<br>`c_22` (optional)<br>`c_23` (optional)<br>`c_24` (optional)<br>`c_25` (optional)<br>`c_26` (optional)<br>`c_27` (optional)<br>`c_28` (optional)<br>`c_29` (optional)<br>`c_30` (optional)<br>`c_31` (optional)<br>`c_32` (optional)<br>`c_33` (optional)<br>`c_34` (optional)<br>`c_35` (optional)<br>`c_36` (optional)<br>`c_37` (optional)<br>`c_38` (optional)<br>`c_39` (optional)<br>`c_40` (optional)<br>`c_41` (optional)<br>`c_42` (optional)<br>`c_43` (optional)<br>`c_44` (optional)<br>`c_45` (optional)<br>`c_46` (optional)<br>`c_47` (optional)<br>`c_48` (optional)<br>`c_49` (optional)<br>`c_50` (optional)<br>`c_51` (optional)<br>`c_52` (optional)<br>`c_53` (optional)<br>`c_54` (optional)<br>`c_55` (optional)<br>`d_1` (optional)<br>`d_2` (optional)<br>`d_3` (optional)<br>`d_4` (optional)<br>`d_5` (optional)<br>`d_6` (optional)<br>`d_7` (optional)<br>`d_8` (optional)<br>`d_9` (optional)<br>`d_10` (optional)<br>`d_11` (optional)<br>`d_12` (optional)<br>`d_13` (optional)<br>`d_14` (optional)<br>`d_15` (optional)<br>`d_16` (optional)<br>`d_17` (optional)<br>`d_18` (optional)<br>`d_19` (optional)<br>`d_20` (optional)<br>`d_21` (optional)<br>`d_22` (optional)<br>`d_23` (optional)<br>`d_24` (optional)<br>`d_25` (optional)<br>`d_26` (optional)<br>`d_27` (optional)<br>`d_28` (optional)<br>`d_29` (optional)<br>`d_30` (optional)<br>`d_31` (optional)<br>`d_32` (optional)<br>`d_33` (optional)<br>`d_34` (optional)<br>`d_35` (optional)<br>`d_36` (optional)<br>`d_37` (optional)<br>`d_38` (optional)<br>`d_39` (optional)<br>`d_40` (optional)<br>`d_41` (optional)<br>`d_42` (optional)<br>`d_43` (optional)<br>`d_44` (optional)<br>`d_45` (optional)<br>`d_46` (optional)<br>`d_47` (optional)<br>`d_48` (optional)<br>`d_49` (optional)<br>`d_50` (optional)<br>`d_51` (optional)<br>`d_52` (optional)<br>`d_53` (optional)<br>`d_54` (optional)<br>`d_55` (optional)<br>`t_1` (optional)<br>`t_2` (optional)<br>`t_3` (optional)<br>`t_4` (optional)<br>`t_5` (optional)<br>`t_6` (optional)<br>`t_7` (optional)<br>`t_8` (optional)<br>`t_9` (optional)<br>`t_10` (optional)<br>`t_11` (optional)<br>`t_12` (optional)<br>`t_13` (optional)<br>`t_14` (optional)<br>`t_15` (optional)<br>`t_16` (optional)<br>`t_17` (optional)<br>`t_18` (optional)<br>`t_19` (optional)<br>`t_20` (optional)<br>`t_21` (optional)<br>`t_22` (optional)<br>`t_23` (optional)<br>`t_24` (optional)<br>`t_25` (optional)<br>`t_26` (optional)<br>`t_27` (optional)<br>`t_28` (optional)<br>`t_29` (optional)<br>`t_30` (optional)<br>`t_31` (optional)<br>`t_32` (optional)<br>`t_33` (optional)<br>`t_34` (optional)<br>`t_35` (optional)<br>`t_36` (optional)<br>`t_37` (optional)<br>`t_38` (optional)<br>`t_39` (optional)<br>`t_40` (optional)<br>`t_41` (optional)<br>`t_42` (optional)<br>`t_43` (optional)<br>`t_44` (optional)<br>`t_45` (optional)<br>`t_46` (optional)<br>`t_47` (optional)<br>`t_48` (optional)<br>`t_49` (optional)<br>`t_50` (optional)<br>`t_51` (optional)<br>`t_52` (optional)<br>`t_53` (optional)<br>`t_54` (optional)<br>`t_55` (optional)<br>`n_1` (optional)<br>`n_2` (optional)<br>`n_3` (optional)<br>`n_4` (optional)<br>`n_5` (optional)<br>`n_6` (optional)<br>`n_7` (optional)<br>`n_8` (optional)<br>`n_9` (optional)<br>`n_10` (optional)<br>`n_11` (optional)<br>`n_12` (optional)<br>`n_13` (optional)<br>`n_14` (optional)<br>`n_15` (optional)<br>`n_16` (optional)<br>`n_17` (optional)<br>`n_18` (optional)<br>`n_19` (optional)<br>`n_20` (optional)<br>`n_21` (optional)<br>`n_22` (optional)<br>`n_23` (optional)<br>`n_24` (optional)<br>`n_25` (optional)<br>`n_26` (optional)<br>`n_27` (optional)<br>`n_28` (optional)<br>`n_29` (optional)<br>`n_30` (optional)<br>`n_31` (optional)<br>`n_32` (optional)<br>`n_33` (optional)<br>`n_34` (optional)<br>`n_35` (optional)<br>`n_36` (optional)<br>`n_37` (optional)<br>`n_38` (optional)<br>`n_39` (optional)<br>`n_40` (optional)<br>`n_41` (optional)<br>`n_42` (optional)<br>`n_43` (optional)<br>`n_44` (optional)<br>`n_45` (optional)<br>`n_46` (optional)<br>`n_47` (optional)<br>`n_48` (optional)<br>`n_49` (optional)<br>`n_50` (optional)<br>`n_51` (optional)<br>`n_52` (optional)<br>`n_53` (optional)<br>`n_54` (optional)<br>`n_55` (optional)<br>`n_56` (optional)<br>`n_57` (optional)<br>`alpha_1` (optional)<br>`alpha_2` (optional)<br>`alpha_3` (optional)<br>`beta_1` (optional)<br>`beta_2` (optional)<br>`beta_3` (optional)<br>`gamma_1` (optional)<br>`gamma_2` (optional)<br>`gamma_3` (optional)<br>`epsilon_1` (optional)<br>`epsilon_2` (optional)<br>`epsilon_3` (optional)<br>`a_1` (optional)<br>`a_2` (optional)<br>`b_1` (optional)<br>`b_2` (optional)<br>`A_1` (optional)<br>`A_2` (optional)<br>`B_1` (optional)<br>`B_2` (optional)<br>`C_1` (optional)<br>`C_2` (optional)<br>`F_1` (optional)<br>`F_2` (optional)<br>`E_1` (optional)<br>`E_2` (optional)<br>`state` (optional) | `reaktoro` |
| `WaterZhangDuan05` | Water, Zhang and Duan | `dew_gibbs` (optional)<br>`a1` (optional)<br>`a2` (optional)<br>`a3` (optional)<br>`a4` (optional)<br>`a5` (optional)<br>`a6` (optional)<br>`a7` (optional)<br>`a8` (optional)<br>`a9` (optional)<br>`a10` (optional)<br>`a11` (optional)<br>`a12` (optional)<br>`a13` (optional)<br>`a14` (optional)<br>`gamma` (optional)<br>`Tc` (optional)<br>`Vc` (optional)<br>`R` (optional)<br>`g1kb_0` (optional)<br>`g1kb_1` (optional)<br>`g1kb_2` (optional)<br>`g1kb_3` (optional)<br>`g1kb_4` (optional)<br>`Tmax` (optional)<br>`Pmax` (optional)<br>`Pmin` (optional)<br>`state` (optional) | one |

### Dielectric models of water

| Id | What it is | Parameters, with the SI unit | Implementations |
|---|---|---|---|
| `JohnsonNorton91` | Dielectric constant of Johnson and Norton | `a1` (optional)<br>`a2` (optional)<br>`a3` (optional)<br>`a4` (optional)<br>`a5` (optional)<br>`a6` (optional)<br>`a7` (optional)<br>`a8` (optional)<br>`a9` (optional)<br>`a10` (optional)<br>`Tr` (optional)<br>`Dr` (optional) | `reaktoro`, `gems` |
| `BradleyPitzer79` | Dielectric constant of Bradley and Pitzer | `U1` (optional)<br>`U2` (optional)<br>`U3` (optional)<br>`U4` (optional)<br>`U5` (optional)<br>`U6` (optional)<br>`U7` (optional)<br>`U8` (optional)<br>`U9` (optional)<br>`Tmin` (optional)<br>`Tmax` (optional)<br>`Tlow` (optional)<br>`Pmax_low` (optional)<br>`Pmax` (optional) | one |
| `HelgesonKirkham74` | Dielectric constant of Helgeson and Kirkham | `e_0_0` (optional)<br>`e_0_1` (optional)<br>`e_0_2` (optional)<br>`e_0_3` (optional)<br>`e_0_4` (optional)<br>`e_1_0` (optional)<br>`e_1_1` (optional)<br>`e_1_2` (optional)<br>`e_1_3` (optional)<br>`e_2_0` (optional)<br>`e_2_1` (optional)<br>`e_2_2` (optional)<br>`e_3_0` (optional)<br>`e_3_1` (optional)<br>`e_4_0` (optional) | one |
| `SverjenskyEtAl14` | Dielectric constant of Sverjensky et al. | `a1` (optional)<br>`a2` (optional)<br>`a3` (optional)<br>`b1` (optional)<br>`b2` (optional)<br>`b3` (optional) | one |
| `FernandezEtAl97` | Dielectric constant of Fernandez et al. | `n1` (optional)<br>`n2` (optional)<br>`n3` (optional)<br>`n4` (optional)<br>`n5` (optional)<br>`n6` (optional)<br>`n7` (optional)<br>`n8` (optional)<br>`n9` (optional)<br>`n10` (optional)<br>`n11` (optional)<br>`n12` (optional)<br>`I1` (optional)<br>`I2` (optional)<br>`I3` (optional)<br>`I4` (optional)<br>`I5` (optional)<br>`I6` (optional)<br>`I7` (optional)<br>`I8` (optional)<br>`I9` (optional)<br>`I10` (optional)<br>`I11` (optional)<br>`J1` (optional)<br>`J2` (optional)<br>`J3` (optional)<br>`J4` (optional)<br>`J5` (optional)<br>`J6` (optional)<br>`J7` (optional)<br>`J8` (optional)<br>`J9` (optional)<br>`J10` (optional)<br>`J11` (optional)<br>`Tc` (optional)<br>`rhoc` (optional)<br>`k` (optional)<br>`Na` (optional)<br>`alpha` (optional)<br>`epsilon0` (optional)<br>`mu` (optional)<br>`M` (optional) | one |

### Base methods of a reaction

| Id | What it is | Parameters, with the SI unit | Implementations |
|---|---|---|---|
| `ConstantReactionProperties` | Constant reaction properties | none | one |
| `LogKPolynomial` | log K polynomial | `a0`<br>`a1` [1/K] (optional)<br>`a2` [K] (optional)<br>`a3` (optional)<br>`a4` [K2] (optional)<br>`a5` [1/K2] (optional)<br>`a6` [K^0.5] (optional)<br>`a` [J/(mol.K)] (optional)<br>`b` [J/(mol.K2)] (optional)<br>`c` [J.K/mol] (optional)<br>`d` [J/(mol.K^0.5)] (optional)<br>`e` [J/(mol.K3)] (optional) | one |
| `LogKNordstromMunoz88` | log K of Nordstrom and Munoz | `a0`<br>`a1` [1/K] (optional)<br>`a2` [K] (optional)<br>`a3` (optional)<br>`a4` [K2] (optional)<br>`a5` [1/K2] (optional)<br>`a6` [K^0.5] (optional)<br>`a` [J/(mol.K)] (optional)<br>`b` [J/(mol.K2)] (optional)<br>`c` [J.K/mol] (optional)<br>`d` [J/(mol.K^0.5)] (optional)<br>`e` [J/(mol.K3)] (optional) | one |
| `LogKConstant` | Constant log K | `a0` (optional)<br>`a1` [1/K] (optional)<br>`a2` [K] (optional)<br>`a3` (optional)<br>`a4` [K2] (optional)<br>`a5` [1/K2] (optional)<br>`a6` [K^0.5] (optional)<br>`a` [J/(mol.K)] (optional)<br>`b` [J/(mol.K2)] (optional)<br>`c` [J.K/mol] (optional)<br>`d` [J/(mol.K^0.5)] (optional)<br>`e` [J/(mol.K3)] (optional) | one |
| `LogKZeroEntropy` | log K with zero entropy of reaction | `a0` (optional)<br>`a1` [1/K] (optional)<br>`a2` [K] (optional)<br>`a3` (optional)<br>`a4` [K2] (optional)<br>`a5` [1/K2] (optional)<br>`a6` [K^0.5] (optional)<br>`a` [J/(mol.K)] (optional)<br>`b` [J/(mol.K2)] (optional)<br>`c` [J.K/mol] (optional)<br>`d` [J/(mol.K^0.5)] (optional)<br>`e` [J/(mol.K3)] (optional) | one |
| `LogKVantHoff` | log K of van 't Hoff | `a0` (optional)<br>`a1` [1/K] (optional)<br>`a2` [K] (optional)<br>`a3` (optional)<br>`a4` [K2] (optional)<br>`a5` [1/K2] (optional)<br>`a6` [K^0.5] (optional)<br>`a` [J/(mol.K)] (optional)<br>`b` [J/(mol.K2)] (optional)<br>`c` [J.K/mol] (optional)<br>`d` [J/(mol.K^0.5)] (optional)<br>`e` [J/(mol.K3)] (optional) | one |
| `LogKConstantCp` | log K with constant heat capacity of reaction | `a0` (optional)<br>`a1` [1/K] (optional)<br>`a2` [K] (optional)<br>`a3` (optional)<br>`a4` [K2] (optional)<br>`a5` [1/K2] (optional)<br>`a6` [K^0.5] (optional)<br>`a` [J/(mol.K)] (optional)<br>`b` [J/(mol.K2)] (optional)<br>`c` [J.K/mol] (optional)<br>`d` [J/(mol.K^0.5)] (optional)<br>`e` [J/(mol.K3)] (optional) | one |
| `LogKTable` | Table of log K | none | one |
| `MarshallFranck81` | Ion product of water of Marshall and Franck (needs the solvent) | `a`<br>`b` [K]<br>`c` [K2]<br>`d` [K3]<br>`e`<br>`f` [K]<br>`g` [K2] | one |
| `RyzhenkoBryzgalin` | Model of Ryzhenko and Bryzgalin (needs the solvent) | `pK298`<br>`A`<br>`B`<br>`a` (optional)<br>`b` (optional)<br>`c` (optional)<br>`d` (optional)<br>`e` (optional)<br>`f` (optional)<br>`g` (optional)<br>`RHO298` (optional)<br>`zzH2O` (optional) | one |
| `DolejsManning10` | Density model of Dolejs and Manning (needs the solvent) | `a` [J/mol]<br>`b` [J/(mol.K)]<br>`c` [J/(mol.K)] (optional)<br>`d` [J/(mol.K2)] (optional)<br>`e` [J/(mol.K)] | one |

### Other methods of a reaction

| Id | What it is | Parameters, with the SI unit | Implementations |
|---|---|---|---|
| `ReactionCpPolynomial` | Heat capacity of reaction | `a` [J/(mol.K)]<br>`b` [J/(mol.K2)] (optional)<br>`c` [J.K/mol] (optional)<br>`d` [J/(mol.K^0.5)] (optional)<br>`e` [J/(mol.K3)] (optional) | one |
| `ReactionVolumePolynomial` | Volume of reaction, T and P | `a1` [1/K]<br>`a2` [1/K2]<br>`a3` [1/K3]<br>`b1` [1/Pa]<br>`b2` [1/Pa2] | one |
| `ReactionConstantVolume` | Constant volume of reaction | none | one |

<!-- methods end -->

### Built-in constants

Some methods have constants that do not belong to one substance, for example the coefficients of a water model. They are in the files of `ThermoFun/Methods/ParameterSets`, written like a `parameter_sets` section, each with a description and its source. The page of the method lists them.

A record does not give them. To use another value, give it inline in the `parameters` of the method, or in a `parameter_set` of the same method. The value then holds only for that record. Where this is not possible, the page of the method says so.

A model can have variants of these files. A record chooses one by its name, and a value given inline still wins:

```
methods:
  - method: HKF88
    parameter_set: HKF88_my_variant_v1
    parameters: {a1: 1.0}
```

A variant has `default: false`. It can be in the library, in the `parameter_sets` of the file, in a `methods/parameter_sets` folder above the dataset, or in a folder named by the environment variable `THERMOFUN_PARAMETER_PATH`. Files in that variable's folders change nothing until a record names one of their sets.

## 10. Choose where properties come from

A substance can have methods and also a defining reaction. A reaction can have methods and also reactants. The record says which one to use. You can change that in the engine.

A substance and its defining reaction often have the same symbol. Each one keeps its own choice:

| Word | Substance with this symbol | Reaction with this symbol |
|---|---|---|
| `reaction` | from its defining reaction | no change |
| `reactants` | no change | from its reactants |
| `direct` | from its methods | from its methods |
| `record` | what the record says | what the record says |

To get a substance from its methods and the reaction from its reactants, give `direct` and then `reactants`. To get a substance from its reaction and the reaction from its reactants, the reaction must not use that substance: ThermoFun raises an error that names the cycle.

Derivatives and propagated errors use the same path as the values. To see the path of a result, use `thermoPropertiesSubstanceDetailed(T, P, symbol).source`.


```python
engine = tf.ThermoEngine("defined.yaml")
engine.setPropertySource("makes-B", "reactants")          # one record
assert engine.propertySource("makes-B") == tf.PropertySource.Reactants
engine.setPropertySource("makes-B", "record")             # back to what the record says
engine.preferences.property_source = "direct"             # all records
```

The words are `record`, `direct`, `reaction` and `reactants`. The engine ignores a source that a record cannot use. A wrong word gives an
error with a hint.

To use one implementation for all records, set `engine.preferences.method_implementation = {"HKF88": "gems"}`.

To see the path that the engine takes for a record, without calculating:

```python
engine = tf.ThermoEngine("small.yaml")
path = engine.calculationPath("Corundum")
assert path["source"] == "methods"
assert path["methods"] == ["ConstantCp", "ConstantVolume"]
```

A reaction and a substance can define each other. If a reaction with `reactants` needs a substance that comes from this reaction, the calculation stops with an error that names the cycle. With `reactants` set for all records in the preferences of the engine, this reaction keeps its own methods, with a warning.

## 11. Use the old setters

Old programs can still build records with setters. These calls are deprecated in version 1.0. New data must use a file.

```python
s = tf.Substance()
s.setSymbol("Y")
s.setFormula("Ca")
s.setMethodGenEoS(tf.MethodGenEoS_Thrift.CTPM_CPT)
parameters = tf.ThermoParametersSubstance()
parameters.Cp_coeff = [[100.0]]
parameters.temperature_intervals = [[273.15, 2000.0]]
s.setThermoParameters(parameters)
assert s.record().methods[0].method == "CpPolynomial"
```

## 12. Find and correct problems

ThermoFun reports each problem with a code, the path of the value, and the line and the column in the file. Errors stop the load.
Warnings go to the log and to `loadDiagnostics()`.

<!-- file: problem.yaml -->
```yaml
thermohub: 2.0.0-draft.1
substances:
  A:
    formula: Ca
    aggregate_state: crystall
    standard_state_properties: {G: -1000}
```

```python
problem = tf.Database("problem.yaml").loadDiagnostics().all()[0]
assert problem.code == "V2-VOC-001"
assert problem.path == "/substances/A/aggregate_state"
assert problem.line == 5
assert 'Did you mean "crystal"' in problem.message
```

A load cannot see some problems, because a record can come from another file. `database.validate()` reports them: a missing reaction, an equation that is not balanced, a cycle of
defining reactions, a missing parameter, a covariance matrix that is wrong.

```python
checked = tf.Database("thermohub: 2.0.0-draft.1\nsubstances:\n  A: {defining_reaction: nothing, standard_state_properties: {G: 1}}\n").validate()
assert [d.code for d in checked.all()] == ["V2-REF-001"]
```

| Code | What is wrong | What to do |
|---|---|---|
| `V2-VER-003` | The file has no key `thermohub`. | Add it at the top of the file. |
| `V2-MTH-012` | The record has two base methods, or no method and no values. | Keep one base method, or give the values. |
| `V2-MTH-005` | The method is for another class of substance. | Use another method, or correct `class`. |
| `V2-MTH-001` | The method id is not known. | Use an id of Section 9. |
| `V2-MTH-002` | The method has no parameter with this name. | Use a name of Section 9. |
| `V2-MTH-003` | The method has two of `parameters`, `segments` and `table`. | Keep one. |
| `V2-MTH-004` | A required parameter is missing. | Add it. |
| `V2-MTH-009` | The method has no such implementation. | Use one of Section 9, or remove the key. |
| `V2-MTH-018` | A table has the wrong size, or a grid is not ascending. | Give nT x nP values. |
| `V2-UNIT-001`, `V2-UNIT-002` | The unit is not known, or has the wrong dimension. | Correct the unit. |
| `V2-RXN-001` | The equation and the stoichiometry do not agree. | Correct one, or remove one. |
| `V2-YAML-001` | The text is not YAML. | Correct the place that the message gives. |

**Check:** run `python -m pytest pytests/test_add_data_guide.py`. All examples of this page run and their results are correct.
