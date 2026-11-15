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
import pandas as pd

pd.DataFrame(engine.explain(473.15, 1e7, "Calcite"))[["method", "G", "S", "V"]]
```

2. Call `thermoPropertiesSubstanceDetailed` to see also where the result comes from.

```python
detailed = engine.thermoPropertiesSubstanceDetailed(473.15, 1e7, "Calcite")
print(detailed["source"], detailed["conversions"])
```

Output:

```text
methods []
```

3. Call `calculationPath` to see the path of a record before you calculate. It also shows the other records that it needs.

```python
path = engine.calculationPath("Calcite")
```

4. Call `contributions` to get the part of each method at many points, as a table. Each row is one (T, P). The column
   `Cp:LandauHP98` is what that method changed Cp by. The columns of a property add up to its value: `Cp:other` is what
   changed after the methods (the conventions of the engine, the options of gases).

```python
import pandas as pd

rows = engine.contributions("Calcite", [400.0, 800.0, 1400.0], 1e7)   # the Landau transition of calcite is at 1240 K
table = pd.DataFrame(rows)[["T", "Cp", "Cp:CpPolynomial", "Cp:LandauHP98", "Cp:MurnaghanHP98", "Cp:other"]]
print(table)
```

Output:

```text
        T          Cp  ...  Cp:MurnaghanHP98  Cp:other
0   400.0   96.004551  ...         -0.004017       0.0
1   800.0  118.497846  ...         -0.002839       0.0
2  1400.0  124.511680  ...         -0.002144       0.0

[3 rows x 6 columns]
```

   A method that sets a property replaces what came before it, so its column also removes the earlier value. The catalog
   says which methods set and which add: `{m["id"]: m["effects"] for m in tf.methods.catalog()}`. In C++ the same is
   `ThermoEngine::contributions(symbol, T, P)`, a `MethodContributions` for each point.

```python
print(path["source"])
```

Output:

```text
methods
```

<!-- check
```python
assert [s["method"] for s in detailed["steps"]] == ["CpPolynomial", "LandauHP98", "MurnaghanHP98"]
assert path["source"] == "methods"
assert abs(table["Cp"][1] - table.iloc[1, 2:].sum()) < 1e-9
assert abs(table["Cp:LandauHP98"][2]) < 1e-3   # above the transition: only the small term of the pressure
```
-->
