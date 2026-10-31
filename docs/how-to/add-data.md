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
| A record that you change in a program | Use the setters of `Substance`, `Reaction` or `Element`. They edit a copy of the record. |

`addReaction`, `setReaction`, `addElement` and `setElement` do the same for the other records.

These are not available yet: dataset folders, `include`, the legacy writer and the Python record builders.

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

By default, a value from a legacy file is written as its short decimal: `-56.484`, not `-56.484001159668`. These values were single-precision
numbers. A database read back from such a file can differ by a small amount from the legacy file: in the bundled databases, at most 0.46 J/mol in G and
3.6e-5 in log K. Use `readable=False` to read back the same numbers.

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

A record with the class `aqueous_solute` and the formula `H+` has all properties 0 at all T and P. It needs no methods.

```python
hydrogen = tf.Database("thermohub: 2.0.0-draft.1\nsubstances:\n  H+: {formula: H+, class: aqueous_solute, aggregate_state: aqueous}\n")
assert tf.ThermoEngine(hydrogen).thermoPropertiesSubstance(400.0, 1e6, "H+").gibbs_energy.val == 0.0
```

### 5.4 A gas

An equation of state changes H, S and V with pressure. Two preferences of the engine decide what happens to G:

- By default, G stays G(T, 1 bar).
- `apply_pressure_correction_to_gas_props = True` adds the pressure term to G and H.
- `cancel_fugacity_coefficient_in_gas_props = False` keeps RT ln(phi) in G.

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
assert engine.thermoPropertiesSubstance(400.0, 1e7, "CO2").gibbs_energy.val == engine.thermoPropertiesSubstance(400.0, 1e5, "CO2").gibbs_energy.val
engine.preferences.apply_pressure_correction_to_gas_props = True
assert engine.thermoPropertiesSubstance(400.0, 1e7, "CO2").gibbs_energy.val > engine.thermoPropertiesSubstance(400.0, 1e5, "CO2").gibbs_energy.val
```

### 5.5 A substance from a reaction

Give `defining_reaction`, and leave out `methods`. The engine calculates the reaction, and takes the properties of the substance from
it and from the other substances of the reaction. The reaction must name the substance.

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
| `HKF88` | Revised HKF equation of state (needs the solvent) | `a1` [J/(mol.Pa)]<br>`a2` [J/mol]<br>`a3` [J.K/(mol.Pa)]<br>`a4` [J.K/mol]<br>`c1` [J/(mol.K)]<br>`c2` [J.K/mol]<br>`omega` [J/mol] | `reaktoro`, `gems` |
| `SoluteHP98` | Aqueous solute of Holland and Powell (needs the solvent) | `b` [J/(mol.K2)] | one |
| `AndersonEtAl91` | Aqueous solute of Anderson et al. (needs the solvent) | none | one |

### Corrections of a substance

| Id | What it is | Parameters, with the SI unit | Implementations |
|---|---|---|---|
| `AkinfievDiamond03` | Model of Akinfiev and Diamond (needs the solvent) | `xi`<br>`a` [m3/kg]<br>`b` [m3.K^0.5/kg] | `gems` |
| `LandauHP98` | Landau transition of Holland and Powell | `Tc0` [K]<br>`Smax` [J/(mol.K)]<br>`Vmax` [m3/mol]<br>`alpha0` [1/K] (optional)<br>`kappa0` [Pa] (optional) | one |
| `LandauHP11` | Landau transition of Holland and Powell (2011) | `Tc0` [K]<br>`Smax` [J/(mol.K)]<br>`Vmax` [m3/mol] | one |
| `ConstantVolume` | Constant molar volume | none | one |
| `VolumeBerman88` | Molar volume of Berman | `a1` [1/K]<br>`a2` [1/K2]<br>`a3` [1/K3]<br>`b1` [1/Pa]<br>`b2` [1/Pa2] | one |
| `VolumeDorogokupets88` | Molar volume of Dorogokupets et al. | `a1` [1/K]<br>`a2` [1/K2]<br>`a3` [1/K3]<br>`b1` [1/Pa]<br>`b2` [1/Pa2] | one |
| `BirchMurnaghan47` | Birch-Murnaghan equation of state | `a1` [1/K]<br>`a2` [1/K2] (optional)<br>`a3` [K] (optional)<br>`kt00` [Pa]<br>`dkdt` [Pa/K] (optional)<br>`kp`<br>`kpp` [1/Pa] (optional) | `gems` |
| `MurnaghanHP98` | Murnaghan equation of Holland and Powell | `alpha0` [1/K]<br>`kappa0` [Pa] | one |
| `TaitHP11` | Tait equation of Holland and Powell | `kappa0p`<br>`kappa0pp` [1/Pa]<br>`n_atoms`<br>`alpha0` [1/K]<br>`kappa0` [Pa] | one |
| `IdealGas` | Ideal gas | none | one |
| `PRSV86` | Peng-Robinson-Stryjek-Vera equation of state | `Tc` [K]<br>`Pc` [Pa]<br>`omega`<br>`k1` (optional)<br>`k2` (optional)<br>`k3` (optional) | one |
| `ChurakovGottschalk03` | Equation of state of Churakov and Gottschalk | `sigma0`<br>`eps0`<br>`mu0`<br>`pol0`<br>`sigma1`<br>`sigma2`<br>`eps1`<br>`eps2`<br>`mu1`<br>`mu2`<br>`pol1`<br>`pol2` | one |
| `SoaveRedlichKwong72` | Soave-Redlich-Kwong equation of state | `Tc` [K]<br>`Pc` [Pa]<br>`omega`<br>`N` (optional) | one |
| `SternerPitzer94` | Equation of state of Sterner and Pitzer | `Tc` [K]<br>`Pc` [Pa] | one |
| `PengRobinson78` | Peng-Robinson equation of state | `Tc` [K]<br>`Pc` [Pa]<br>`omega`<br>`N` (optional) | one |
| `CorkHP91` | Compensated Redlich-Kwong equation of state | `Tc` [K]<br>`Pc` [Pa]<br>`omega`<br>`k1` (optional)<br>`k2` (optional)<br>`k3` (optional) | one |

### Equations of state of water

| Id | What it is | Parameters, with the SI unit | Implementations |
|---|---|---|---|
| `WaterHGK84` | Water, Haar-Gallagher-Kell | `state` (optional) | `reaktoro`, `gems` |
| `WaterIAPWS95` | Water, IAPWS-95 | `state` (optional) | `reaktoro` |
| `WaterZhangDuan05` | Water, Zhang and Duan | `dew_gibbs` (optional)<br>`g1kb_0` (optional)<br>`g1kb_1` (optional)<br>`g1kb_2` (optional)<br>`g1kb_3` (optional)<br>`g1kb_4` (optional)<br>`state` (optional) | one |

### Dielectric models of water

| Id | What it is | Parameters, with the SI unit | Implementations |
|---|---|---|---|
| `JohnsonNorton91` | Dielectric constant of Johnson and Norton | none | `reaktoro`, `gems` |
| `SverjenskyEtAl14` | Dielectric constant of Sverjensky et al. | none | one |
| `FernandezEtAl97` | Dielectric constant of Fernandez et al. | none | one |

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
| `RyzhenkoBryzgalin` | Model of Ryzhenko and Bryzgalin (needs the solvent) | `pK298`<br>`A`<br>`B` | one |
| `DolejsManning10` | Density model of Dolejs and Manning (needs the solvent) | `a` [J/mol]<br>`b` [J/(mol.K)]<br>`c` [J/(mol.K)] (optional)<br>`d` [J/(mol.K2)] (optional)<br>`e` [J/(mol.K)] | one |

### Other methods of a reaction

| Id | What it is | Parameters, with the SI unit | Implementations |
|---|---|---|---|
| `ReactionCpPolynomial` | Heat capacity of reaction | `a` [J/(mol.K)]<br>`b` [J/(mol.K2)] (optional)<br>`c` [J.K/mol] (optional)<br>`d` [J/(mol.K^0.5)] (optional)<br>`e` [J/(mol.K3)] (optional) | one |
| `ReactionVolumePolynomial` | Volume of reaction, T and P | `a1` [1/K]<br>`a2` [1/K2]<br>`a3` [1/K3]<br>`b1` [1/Pa]<br>`b2` [1/Pa2] | one |
| `ReactionConstantVolume` | Constant volume of reaction | none | one |

<!-- methods end -->

## 10. Choose where properties come from

A substance can have methods and also a reaction. The record says which one to use. You can change that in the engine:

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
```

```python
problem = tf.Database("problem.yaml").loadDiagnostics().all()[0]
assert problem.code == "V2-VOC-001"
assert problem.path == "/substances/A/aggregate_state"
assert problem.line == 5
assert 'Did you mean "crystal"' in problem.message
```

| Code | What is wrong | What to do |
|---|---|---|
| `V2-MTH-001` | The method id is not known. | Use an id of Section 9. |
| `V2-MTH-002` | The method has no parameter with this name. | Use a name of Section 9. |
| `V2-MTH-003` | The method has two of `parameters`, `segments` and `table`. | Keep one. |
| `V2-MTH-004` | A required parameter is missing. | Add it. |
| `V2-MTH-009` | The method has no such implementation. | Use one of Section 9, or remove the key. |
| `V2-MTH-018` | A table has the wrong size, or a grid is not ascending. | Give nT x nP values. |
| `V2-UNIT-001`, `V2-UNIT-002` | The unit is not known, or has the wrong dimension. | Correct the unit. |
| `V2-RXN-001` | The equation and the stoichiometry do not agree. | Correct one, or remove one. |
| `V2-YAML-001` | The text is not YAML. | Correct the place that the message gives. |

## 13. What ThermoFun does not use yet

ThermoFun reads these, but does not use them in a calculation:

- `parameter_set` of a method and the record `parameter_sets`.
- Method records with `kind: symbolic`.
- Covariance blocks.
- `convention` of a record or a dataset. Conventions other than `benson_helgeson` and `berman_brown` raise an error in the engine.

**Check:** run `python -m pytest pytests/test_add_data_guide.py`. All examples of this page run and their results are correct.
