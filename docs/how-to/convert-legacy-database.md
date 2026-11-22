<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Convert a legacy database

**You need:** ThermoFun, and a database in the legacy format. Here it is `aq17-thermofun.json`, from `Resources/databases/`
of the repository.

The tool `thermofun.convert` writes the records in the v2 format, and a report of what it changed.

## 1. Convert

```console
$ python -m thermofun.convert aq17-thermofun.json aq17.yaml --format yaml --report report.md
```

It prints nothing, and writes `aq17.yaml` and `report.md`. The options:

| Option | What it does |
|---|---|
| `--format yaml` or `json` | The format of the file to write. |
| `--report report.md` | Writes the report (Markdown). |
| `--split` | Writes a folder with one file for each record (step 4). |
| `--readable` | Short decimals: -56.484 for the -56.484001159668 of a GEMS export. The values then change a little, and the report says how many and the largest change. Without it, each number reads back exactly. |
| `--to legacy` | Back to the legacy format. |

## 2. Read the report

```console
$ cat report.md
# Conversion report

## Summary

- input: `aq17-thermofun.json`
- output format: v2
- elements: 11, substances: 161, reactions: 0
- methods: 0, parameter sets: 0, data sources: 0
- numbers written exactly: a result from the output is the same as from the input

## Quirks applied

| Code | Count | First records |
|---|---|---|
| LEG-QRK-003 | 43 | Al(OH)2+, Al(OH)3@, Al(OH)4-, Al+3, AlH3SiO4+2 |
| LEG-QRK-004 | 109 | Corundum, Kaolinite, Pyrophyllite, Topaz-OH, Sillimanite |
| LEG-QRK-005 | 109 | Corundum, Kaolinite, Pyrophyllite, Topaz-OH, Sillimanite |
| LEG-QRK-010 | 169 | Al(OH)2+, Al(OH)3@, Al(OH)4-, Al+3, AlH3SiO4+2 |
| LEG-QRK-011 | 116 | CH4@, CO2@, H2@, O2@, Gibbsite |

## Unread legacy keys

None.

## Unresolved references

None.

## Other problems of the data

| Code | Count | First records |
|---|---|---|
| V2-MTH-028 | 2 | Boehmite, Gibbsite |

## Methods used

| Method | Records |
|---|---|
| AkinfievDiamond03 | 4 |
| ConstantVolume | 1 |
| CorkHP91 | 6 |
| CpPolynomial | 116 |
| HKF88 | 43 |
| JohnsonNorton91 | 1 |
| LandauHP98 | 14 |
| MurnaghanHP98 | 105 |
| WaterIAPWS95 | 1 |

## Uncertainty convention notice

A legacy `error` is read as a standard uncertainty (coverage factor 1). The legacy format does not say if an error is a standard uncertainty, an expanded one or a confidence interval. Check the source of the data before you use the uncertainties.
```

- **Summary**: what was read. "Numbers written exactly" means that the converted file gives the same results.
- **Quirks applied**: the habits of the legacy format that the conversion corrected, with a code (`LEG-QRK-...`). Each
  code is explained in [Diagnostics](../reference/diagnostics.md).
- **Unread legacy keys** and **unresolved references**: data that the conversion could not keep. Here none.
- **Other problems of the data**: what [Validate data](validate-data.md) would report. Here two minerals miss
  parameters of their volume method.
- **Methods used**: the v2 methods of the records, with the [list of methods](../reference/methods/index.md).

## 3. Look at the new file

```console
$ head -n 22 aq17.yaml
thermohub: 2.0.0-draft.1
generator: ThermoFun
thermodataset: {symbol: aq17-gem, name: aq17-gem, conventions: {water_triple_point_reference: HKF_HGK_1974}}
elements:
  Al: {molar_mass: 26.9815006256104, standard_entropy: 28.2999992370605, data_sources: ["[1998SHO/EA]"]}
  C: {molar_mass: 12.0108003616333, standard_entropy: 5.73999977111816, data_sources: ["[1998SHO/EA]"]}
  Ca: {molar_mass: 40.0779991149902, standard_entropy: 41.5900001525879, data_sources: ["[1998SHO/EA]"]}
  Cl: {molar_mass: 35.4529991149902, standard_entropy: 111.540000915527, data_sources: ["[1998SHO/EA]"]}
  H: {molar_mass: 1.00794994831085, standard_entropy: 65.3399963378906, data_sources: ["[1998SHO/EA]"]}
  K: {molar_mass: 39.0983009338379, standard_entropy: 64.6800003051758, data_sources: ["[1998SHO/EA]"]}
  Mg: {molar_mass: 24.3050003051758, standard_entropy: 32.6699981689453, data_sources: ["[1998SHO/EA]"]}
  Na: {molar_mass: 22.9897994995117, standard_entropy: 51.2999992370605, data_sources: ["[1998SHO/EA]"]}
  O: {molar_mass: 15.999400138855, standard_entropy: 102.569000244141, data_sources: ["[1998SHO/EA]"]}
  Si: {molar_mass: 28.0855007171631, standard_entropy: 18.8099994659424, data_sources: ["[1998SHO/EA]"]}
  Zz: {class: charge, molar_mass: 0.0, standard_entropy: -65.3399963378906, data_sources: ["[1998SHO/EA]"]}
substances:
  Akermanite:
    name: Akermanite (ak)
    formula: Ca2MgSi2O7
    charge: 0
    class: component
    aggregate_state: crystal
```

The format is in [The v2 format](../reference/format-v2.md).

## 4. One file for each record

```console
$ python -m thermofun.convert aq17-thermofun.json aq17-folder --format yaml --split
$ ls aq17-folder
elements
substances
thermodataset.yaml
```

`substances/` has one file for each substance, for example `substances/Calcite.yaml`. `tf.Database("aq17-folder")` reads
the folder back.

## 5. The same in Python

```python
import thermofun as tf

tf.convertFile("aq17-thermofun.json", "aq17.yaml", yaml=True, report="report.md")
tf.Database("aq17-thermofun.json").saveDataset("aq17-folder", yaml=True)   # as --split
```

In C++ the same calls are `ThermoFun::convertFile` (with `ConvertOptions`) and `Database::saveDataset`, in
`ThermoFun/Convert.h` and `ThermoFun/Database.h`.

## 6. Check that the two files give the same results

```python
old = tf.ThermoEngine("aq17-thermofun.json").thermoPropertiesSubstance(473.15, 1e7, "Calcite")
new = tf.ThermoEngine("aq17.yaml").thermoPropertiesSubstance(473.15, 1e7, "Calcite")
print(old.gibbs_energy.val == new.gibbs_energy.val)
```

Output:

```text
True
```

The two formats are compared in [Two formats](../reference/legacy-and-v2.md).
