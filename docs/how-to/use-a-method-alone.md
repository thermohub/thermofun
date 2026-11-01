<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Use a method alone

**You need:** ThermoFun. You need no database and no engine.

1. Make the method from its id and its parameters in SI.
2. Give the reference values.
3. Evaluate at T in K and P in Pa.

```python
import thermofun as tf

method = tf.methods.substanceMethod("ConstantVolume")
reference = tf.methods.Reference(T=298.15, P=1e5, G=-1e6, H=-1e6, S=10.0, Cp=10.0, V=2e-5)
result = method.evaluate(400.0, 1e7, reference)
print(result.V.val)                                             # m3/mol
```

Water and dielectric models work in the same way:

```python
water = tf.methods.solventMethod("WaterIAPWS95").properties(298.15, 1e5, 0)
dielectric = tf.methods.dielectricMethod("BradleyPitzer79").properties(298.15, 1e5, water, 0)
```

`tf.methods.catalog()` lists all methods and their parameters. The pages are in [Methods](../reference/methods/index.md).

**Check:**

```python
assert result.V.val == 2e-5
assert abs(water.density.val - 997.05) < 0.01                   # kg/m3
assert abs(dielectric.epsilon.val - 78.38) < 0.01
```
