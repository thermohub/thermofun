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
runs them at the T and P that you ask for. [Parameters of all methods](../reference/method-parameters.md) lists them all.

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
legacy.save("converted.yaml")                    # the values are written with short decimals
exact = tf.Database(legacy.toJson(readable=False))
print(exact.getSubstance("A").record().standard_state_properties[tf.PropertyName.G].value)
```

Output:

```text
-56.484001159668
```

<!-- check
```python
assert "-56.484\n" in open("converted.yaml").read() or "G: -56.484" in open("converted.yaml").read()
assert exact.getSubstance("A").record().standard_state_properties[tf.PropertyName.G].value == -56.484001159668
```
-->

`database.save("old.json", format=tf.DataFormat.Legacy)` writes the legacy format. `database.legacyWriteProblems()` lists what that format cannot hold.

By default, a value from a legacy file is written as its short decimal: `-56.484`, not `-56.484001159668`. These values were single-precision
numbers. A database read back from such a file can differ by a small amount from the legacy file: in the bundled databases, at most 0.46 J/mol in G and
3.6e-5 in log K. Use `readable=False` to read back the same numbers.

No information is lost. A short decimal gives the same single-precision number as the long one. A value in another unit, for example cal, is written as the shortest
number that reads back as the same value in SI units. A number in a file always has a point as the decimal mark, in all locales.

## 3. The smallest file

The words of `class` and `aggregate_state` (and of the other fields with a fixed list) are in [Allowed words](../reference/vocabulary.md). The fields of a substance are in Section 5, the methods in Section 9.

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
print(result.heat_capacity_cp.val)
```

Output:

```text
79.0
```

<!-- check
```python
assert result.heat_capacity_cp.val == 79.0                   # J/(mol K)
assert abs(result.gibbs_energy.val + 1586508.58) < 0.01      # J/mol
```
-->

The results of the engine have volumes in J/bar: `result.volume.val` is 2.558 for this record.

A record with no method and no reaction has its reference properties at all T and P.

Add more data to this database:

```python
db = tf.Database("small.yaml")
one = '{"symbol": "Corundum", "formula": "X", "standard_state_properties": {"G": 1}}'
db.addSubstance(one)       # the symbol is there: nothing changes
print(db.getSubstance("Corundum").formula())   # Al2O3

db.setSubstance(one)       # the record is replaced
print(db.getSubstance("Corundum").formula())   # X
```

Output:

```text
Al2O3
X
```

<!-- check
```python
assert db.getSubstance("Corundum").formula() == "X"
```
-->

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
| `{value: -237183, uncertainty: {half_width: 300, distribution: uniform}}` | A rectangular distribution of half-width 300: u = 300/√3. `triangular`: u = a/√6. |
| `{value: -237183, derivation: "from the log K of R1"}` | A note on how the value was made. It is kept and written back; no calculation reads it. |
| `{value: -237183, quality: core, origin: calculated, source: 2018LOT/MAT}` | Data about the value. |

**Default units.** Standard-state properties are in J/mol, J/(mol K) and m3/mol. Reaction properties use the same units. Molar
mass is in g/mol. Temperature is in K. Pressure is in Pa. A method parameter has the SI unit of [Parameters of all methods](../reference/method-parameters.md).

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
```

<!-- check
```python
assert abs(lime.standard_state_properties[tf.PropertyName.G].value + 604169.6) < 1e-6    # J/mol
assert abs(lime.standard_state_properties[tf.PropertyName.V].value - 16.76e-6) < 1e-15   # m3/mol
```
-->

A molar volume can also be in J/bar. That is the unit of the legacy data: 1 J/bar is 1e-5 m3.

**Missing is not zero.** Leave out a value that you do not know. A value of 0 means that the value is known to be 0.

## 5. Write a substance

| Field | What it holds |
|---|---|
| `formula` | The chemical formula, for example `Al2O3` or `Ca+2`. Required by the methods that read the formula. |
| `name`, `sourcetdb` | Free text. |
| `charge` | The formula charge, a whole number. |
| `class` | `component`, `gas_fluid`, `aqueous_solute`, `water_solvent`, `surface_species`, `site_moiety` or `other` (the default). All words: [Allowed words](../reference/vocabulary.md). |
| `aggregate_state` | `gas`, `liquid`, `crystal`, `aqueous`, `solid` and more; `other` is the default. All words: [Allowed words](../reference/vocabulary.md). An unknown word gives a warning with the nearest word. |
| `molar_mass` | The molar mass. |
| `reference_conditions` | `{T: 298.15}`: the reference temperature. The default comes from the dataset, else it is 298.15 K. The values are at this T and at the standard pressure p° of `standard_state` (1 bar unless given). |
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

Each substance has **one base method** and any number of increments. The base method makes all properties. An increment changes
some of them. You can list the methods in any order, because the engine finds the order.

| Substance | Base method | Increments |
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
```

<!-- check
```python
assert abs(quartz.heat_capacity_cp.val - (44.603 + 0.03778 * 500 - 1.0018e6 / 500**2)) < 1e-6
```
-->

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
print(engine.explain(373.15, 1e6, "Ca+2")[0]["method"])
```

Output:

```text
HKF88
```

<!-- check
```python
assert engine.explain(373.15, 1e6, "Ca+2")[0]["method"] == "HKF88"
```
-->

**Conventions.** A value is defined by its standard state, its reference temperature and its energy reference. Give the blocks
`standard_state` and `energy_reference` in a record, or in the dataset, if the values are not in the defaults (`1bar`, `molality`,
`benson_helgeson`, `helgeson_kirkham_1974`). For example: `standard_state: {pressure: 1atm}` and `energy_reference: {elements: berman_brown}`.

- `standard_state.pressure`: `1bar`, `1atm`. It changes gases only.
- `standard_state.concentration_scale`: `molality`, `molarity`, `mole_fraction` (IUPAC: `amount_concentration`, `amount_fraction`). It changes aqueous solutes only. `molarity` needs the water solvent in the database.
- `energy_reference.elements`: `benson_helgeson`, `berman_brown`, `ser`, `assigned_enthalpy`, `gibbs_helmholtz_formation`, `dft_formation_0K`.
- `energy_reference.water`: the anchor of water, `helgeson_kirkham_1974` (G of liquid water at 25 °C -237181 J/mol, HKF data) or
  `codata_1989` (-237140 J/mol, NEA data), declared on the dataset or the water record: it is data of water, used inside every
  reaction with H2O, and never converted between the two. `steam_tables` says that the values of a water record are on the
  steam scale (S = 0, U = 0 at the triple point).

The engine converts the result of the base method. It does not convert increments. Species with different conventions can be in one reaction. A change of the energy reference needs the
entropies of the elements. Put them in `elements`. If they are not there, the engine uses the table of ChemicalFun and gives a warning (`CNV-006`). If no source has them, the engine gives an error (`CNV-001`).
`NASA9` is always in `assigned_enthalpy` and `CpExtendedUNIQUAC` in `gibbs_helmholtz_formation`, whatever the record says. Results come in the defaults. To ask for others, set `EngineConventions`.

**The hydrogen ion.** The geochemical convention (`benson_helgeson`, the default) defines all properties of the hydrogen ion as 0, at all T and P. A record with the class
`aqueous_solute` and the formula `H+` needs no methods and no values in this convention. In another convention, the properties of H+ can be different: give `energy_reference` in
the record, or in the dataset, with the values and the methods. ThermoFun then uses them. It also uses values other than 0 in the geochemical convention, and `validate` gives a warning (`CNV-007`).

```python
hydrogen = tf.Database("thermohub: 2.0.0-draft.1\nsubstances:\n  H+: {formula: H+, class: aqueous_solute, aggregate_state: aqueous}\n")
print(tf.ThermoEngine(hydrogen).thermoPropertiesSubstance(400.0, 1e6, "H+").gibbs_energy.val)
other = tf.Database("""thermohub: 2.0.0-draft.1
substances:
  H+:
    formula: H+
    class: aqueous_solute
    aggregate_state: aqueous
    energy_reference: {elements: ser}
    standard_state_properties: {G: -1000, H: -1000, S: 10, Cp: 20, V: 0}
    methods:
      - {method: ConstantCp}
""")
print(tf.ThermoEngine(other).thermoPropertiesSubstance(298.15, 1e5, "H+").gibbs_energy.val)
```

Output:

```text
0.0
-1000.0
```

<!-- check
```python
assert tf.ThermoEngine(hydrogen).thermoPropertiesSubstance(400.0, 1e6, "H+").gibbs_energy.val == 0.0
assert tf.ThermoEngine(other).thermoPropertiesSubstance(298.15, 1e5, "H+").gibbs_energy.val == -1000.0
```
-->

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
low, high = (engine.thermoPropertiesSubstance(400.0, p, "CO2").volume.val for p in (1e5, 1e7))   # the volume at 1 bar and at 100 bar
```

<!-- check
```python
assert high < low
assert engine.thermoPropertiesSubstance(400.0, 1e7, "CO2").gibbs_energy.val > engine.thermoPropertiesSubstance(400.0, 1e5, "CO2").gibbs_energy.val
```
-->

Two options of the engine remove the terms of the pressure from G:

```python
engine.preferences.apply_pressure_correction_to_gas_props = False
engine.preferences.cancel_fugacity_coefficient_in_gas_props = True
```

<!-- check
```python
assert engine.thermoPropertiesSubstance(400.0, 1e7, "CO2").gibbs_energy.val == engine.thermoPropertiesSubstance(400.0, 1e5, "CO2").gibbs_energy.val
```
-->

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
print(engine.thermoPropertiesSubstanceDetailed(298.15, 1e5, "B")["source"])
```

Output:

```text
reaction
```

<!-- check
```python
assert abs(b.gibbs_energy.val - (-1000 - 8.31451 * 298.15 * 2.302585093)) < 1
assert engine.thermoPropertiesSubstanceDetailed(298.15, 1e5, "B")["source"] == "reaction"
```
-->

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
```

<!-- check
```python
assert abs(t.gibbs_energy.val + 1100) < 1e-6
```
-->

## 6. Write a reaction

| Field | What it holds |
|---|---|
| `equation` | For example `Ca+2 + CO3-2 = Calcite`. |
| `stoichiometry` | `{Calcite: 1, Ca+2: -1, CO3-2: -1}`: below 0 for a reactant. Optional if `equation` is there. If both are there, they must agree. |
| `reaction_properties` | `logK`, `dG`, `dH`, `dS`, `dCp`, `dV` at the reference conditions. |
| `methods` | One base method for log K, and optionally one volume method. |
| `reference_conditions`, `applicability`, `level`, `data_sources`, `covariances` | As for a substance. |

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
        interpolation_order: 3
        table: {temperatures: [273.15, 298.15, 373.15, 473.15], logK: [1.0, 1.1, 1.3, 1.6]}
```

```python
engine = tf.ThermoEngine("reaction.yaml")
```

<!-- check
```python
assert abs(engine.thermoPropertiesReaction(298.15, 1e5, "fixed").log_equilibrium_constant.val - 2.0) < 1e-9
assert abs(engine.thermoPropertiesReaction(300.0, 1e5, "polynomial").log_equilibrium_constant.val - 0.0) < 1e-9
assert abs(engine.thermoPropertiesReaction(298.15, 1e5, "from-a-table").log_equilibrium_constant.val - 1.1) < 1e-9
```
-->

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
  reference_conditions: {T: 298.15}
  energy_reference: {water: helgeson_kirkham_1974}   # the default: the anchor of water of HKF data
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
print(db.thermoDataSet().symbol)
print(db.dataSources()["1998HOL/POW"].year)
print(tf.ThermoEngine(db).solventSymbol())
```

Output:

```text
my-data
1998
WATER
```

<!-- check
```python
assert db.thermoDataSet().symbol == "my-data"
assert db.dataSources()["1998HOL/POW"].year == 1998
assert tf.ThermoEngine(db).solventSymbol() == "WATER"
assert abs(db.getElement("Ca").molarMass() - 40.078) < 1e-9
```
-->

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
print(tf.Database("main.yaml").getSubstancesList())
```

Output:

```text
['First', 'Second']
```

<!-- check
```python
assert tf.Database("main.yaml").getSubstancesList() == ["First", "Second"]
```
-->

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
print(x.covariance[0].matrix)
print(x.standard_state_properties[tf.PropertyName.G].uncertainty.standard)
```

Output:

```text
[4.0, 1.0, 1.0, 9.0]
2.0
```

<!-- check
```python
assert x.covariance[0].matrix == [4.0, 1.0, 1.0, 9.0]
assert x.standard_state_properties[tf.PropertyName.G].uncertainty.standard == 2.0
```
-->

The engine reads the block now. Its calculation with covariance comes with a later step.

## 9. Methods and their parameters

A record names its methods in the list `methods`. Each entry has the id of the method and, if the method has them, its parameters. The parameters that belong to one substance (for example `a1` and `omega` of an aqueous species) are in the record:

```
methods:
  - method: HKF88
    units: {energy: cal, pressure: bar}      # the units of the published parameters
    parameters: {a1: 0.1839, a2: -228.5, omega: 33060.0}
```

The tables of all methods, with their parameters and units, are on their own page: [Parameters of all methods](../reference/method-parameters.md). Each method also has its page in the [list of methods](../reference/methods/index.md).

### Built-in coefficients

Some methods have coefficients that do not belong to one substance, for example the coefficients of a water model. They are not in the record but in default YAML files, one for each set, in the folder `ThermoFun/Methods/ParameterSets/` of the source (a copy is in `include/ThermoFun/Methods/ParameterSets/` of an installation). Each file has a description and its source, written like a `parameter_sets` section. The page of the method lists them.

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
```

<!-- check
```python
assert engine.propertySource("makes-B") == tf.PropertySource.Reactants
```
-->

```python
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
print(path["source"])
print(path["methods"])
```

Output:

```text
methods
['ConstantCp', 'ConstantVolume']
```

<!-- check
```python
assert path["source"] == "methods"
assert path["methods"] == ["ConstantCp", "ConstantVolume"]
```
-->

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
print(s.record().methods[0].method)
```

Output:

```text
CpPolynomial
```

<!-- check
```python
assert s.record().methods[0].method == "CpPolynomial"
```
-->

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
print(problem.code)
print(problem.path)
print(problem.line)
```

Output:

```text
V2-VOC-001
/substances/A/aggregate_state
5
```

<!-- check
```python
assert problem.code == "V2-VOC-001"
assert problem.path == "/substances/A/aggregate_state"
assert problem.line == 5
assert 'Did you mean "crystal"' in problem.message
```
-->

A load cannot see some problems, because a record can come from another file. `database.validate()` reports them: a missing reaction, an equation that is not balanced, a cycle of
defining reactions, a missing parameter, a covariance matrix that is wrong.

```python
checked = tf.Database("thermohub: 2.0.0-draft.1\nsubstances:\n  A: {defining_reaction: nothing, standard_state_properties: {G: 1}}\n").validate()
```

<!-- check
```python
assert [d.code for d in checked.all()] == ["V2-REF-001"]
```
-->

| Code | What is wrong | What to do |
|---|---|---|
| `V2-VER-003` | The file has no key `thermohub`. | Add it at the top of the file. |
| `V2-MTH-012` | The record has two base methods, or no method and no values. | Keep one base method, or give the values. |
| `V2-MTH-005` | The method is for another class of substance. | Use another method, or correct `class`. |
| `V2-MTH-001` | The method id is not known. | Use an id of [the list of methods](../reference/methods/index.md). |
| `V2-MTH-002` | The method has no parameter with this name. | Use a name of [Parameters of all methods](../reference/method-parameters.md). |
| `V2-MTH-003` | The method has two of `parameters`, `segments` and `table`. | Keep one. |
| `V2-MTH-004` | A required parameter is missing. | Add it. |
| `V2-MTH-009` | The method has no such implementation. | Use one of the implementations on the page of the method, or remove the key. |
| `V2-MTH-018` | A table has the wrong size, or a grid is not ascending. | Give nT x nP values. |
| `V2-UNIT-001`, `V2-UNIT-002` | The unit is not known, or has the wrong dimension. | Correct the unit. |
| `V2-RXN-001` | The equation and the stoichiometry do not agree. | Correct one, or remove one. |
| `V2-YAML-001` | The text is not YAML. | Correct the place that the message gives. |

