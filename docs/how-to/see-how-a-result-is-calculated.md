<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# See how a result is calculated

**Related:** [Results](../reference/results.md) · [How methods combine](../reference/how-methods-combine.md) · [Methods](../reference/methods/index.md) · [Options of the engine](../reference/engine-preferences.md)

**You need:** ThermoFun, and the file `examples/v2/mineral-methods.yaml`.

Every result has a **route**: where its values came from (`by`), the methods and what each one contributed, the other
records it needed, and the conversions after the methods. For each of G, H, S, Cp and V the result is the sum of the
contributions and the changes of the conversions.

1. Call `route` to see how a record is calculated, before you calculate. It also shows the other records that it needs.

```python
import thermofun as tf

engine = tf.ThermoEngine("examples/v2/mineral-methods.yaml")
print(engine.route("Calcite"))
```

Output:

```text
symbol: Calcite
kind: substance
by: methods
methods:
  - method: CpPolynomial
  - method: LandauHP98
  - method: MurnaghanHP98
```

2. Read the route of a result: `info.route` of the result or of any of its values. It has the point and the
   contribution of each method, in J/mol, J/(mol K) and m3/mol.

```python
calcite = engine.thermoPropertiesSubstance(473.15, 1e7, "Calcite")
print(calcite.info.route)
```

Output:

```text
symbol: Calcite
kind: substance
T: 473.15
P: 10000000
by: methods
methods:
  - method: CpPolynomial
    contribution: {G: -1149243.70634, H: -1191352.29535, S: 134.725321747, Cp: 99.5698106434, V: 0}
  - method: LandauHP98
    contribution: {G: -72.2168686371, H: 330.141120562, S: 0.85038146296, Cp: 2.42501876785, V: 1.00143143652e-07}
  - method: MurnaghanHP98
    contribution: {G: 366.556703475, H: 362.451016904, S: -0.00867734665829, Cp: -0.00369284702661, V: 3.70234540285e-05}
```

3. Call `routeTable` to get the contributions at many points, as a table. Each row is one (T, P). The column
   `LandauHP98:Cp` is what that method contributed to Cp. The columns of a property add up to its value. A conversion
   after the methods (a convention of the results, an option of gases) has a column of its own, `<what>(<detail>):Cp`.

```python
import pandas as pd

rows = engine.routeTable([400.0, 800.0, 1400.0], 1e7, "Calcite")   # the Landau transition of calcite is at 1240 K
table = pd.DataFrame(rows)[["T", "Cp", "CpPolynomial:Cp", "LandauHP98:Cp", "MurnaghanHP98:Cp"]]
print(table)
```

Output:

```text
        T          Cp  CpPolynomial:Cp  LandauHP98:Cp  MurnaghanHP98:Cp
0   400.0   96.004551        94.049725       1.958842         -0.004017
1   800.0  118.497846       113.088708       5.411977         -0.002839
2  1400.0  124.511680       124.513844      -0.000020         -0.002144
```

   A method that sets a property replaces what came before it, so its column also removes the earlier value. The catalog
   says which methods set and which add: `tf.methods.catalog("LandauHP98")["contributes_to"]`.

4. For the parts of one model, ask for its terms or variables: `engine.routeTable(T, P, symbol, "terms")`. Only some
   methods have them, for example [HKF88](../reference/methods/HKF88.md). The route of every result has them too.

<!-- check
```python
route = calcite.info.route
assert [m.method for m in route.methods] == ["CpPolynomial", "LandauHP98", "MurnaghanHP98"]
assert abs(route.total()["G"] - calcite.gibbs_energy.val) < 1e-6
assert abs(table["Cp"][1] - table.iloc[1, 2:].sum()) < 1e-9
assert abs(table["LandauHP98:Cp"][2]) < 1e-3   # above the transition
```
-->

In C++ the route is `x.info.route` (a `std::shared_ptr<const ThermoFun::Route>`, `Common/Route.h`), `ThermoEngine::route(symbol)`
and `ThermoEngine::routeTable(T, P, symbol, part)`; `ThermoFun::toYaml(route)` prints it.
