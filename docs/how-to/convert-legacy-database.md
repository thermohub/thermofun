<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Convert a legacy database

**You need:** ThermoFun, and a database in the legacy format. Here it is `aq17-thermofun.json`.

1. Run the conversion. A report lists what changed in each record.

   ```
   python -m thermofun.convert aq17-thermofun.json aq17.yaml --format yaml --report report.md
   ```

2. Read the report. Records that the conversion cannot keep exactly are listed there.
3. Use `--split` to write a folder with one file for each record.
4. Each number is written so that it reads back exactly: a result from the converted file is the same as from the original. With `--readable` the numbers are short decimals (for example -56.484 for the -56.484001159668 of a GEMS export); the values then change a little, and the report gives how many and the largest change.
5. Use `--to legacy` to go back to the legacy format.

The same in Python:

```python
import thermofun as tf

database = tf.Database("aq17-thermofun.json")
with open("aq17.yaml", "w") as f:
    f.write(database.toYaml(True))
```

**Check:** the two files give the same results.

```python
old = tf.ThermoEngine("aq17-thermofun.json").thermoPropertiesSubstance(473.15, 1e7, "Calcite")
new = tf.ThermoEngine("aq17.yaml").thermoPropertiesSubstance(473.15, 1e7, "Calcite")
assert abs(old.gibbs_energy.val - new.gibbs_energy.val) < 0.1   # J/mol
```

The formats are in [Two formats](../explanation/legacy-and-v2.md).
