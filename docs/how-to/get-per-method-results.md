<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# See what each method gives

**You need:** ThermoFun, and the file `examples/v2/mineral-methods.yaml`.

1. Call `explain` to see the state after each method, in SI units.

```python
import thermofun as tf

engine = tf.ThermoEngine("examples/v2/mineral-methods.yaml")
for step in engine.explain(473.15, 1e7, "Calcite"):
    print(step["method"], step["G"], step["S"], step["V"])
```

2. Call `thermoPropertiesSubstanceDetailed` to see also where the result comes from.

```python
detailed = engine.thermoPropertiesSubstanceDetailed(473.15, 1e7, "Calcite")
print(detailed["source"], detailed["conversions"])
```

3. Call `calculationPath` to see the path of a record before you calculate. It also shows the other records that it needs.

```python
path = engine.calculationPath("Calcite")
```

**Check:**

```python
assert [s["method"] for s in detailed["steps"]] == ["CpPolynomial", "LandauHP98", "MurnaghanHP98"]
assert path["source"] == "methods"
```
