<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Choose water models

**You need:** ThermoFun, and a water record in your database.

A water record has two methods: an equation of state and a dielectric model.

| Equation of state | Use it for |
|---|---|
| `WaterIAPWS95` | The current standard for water. |
| `WaterHGK84` | Data that were fitted with Haar-Gallagher-Kell, such as SUPCRT data. |
| `WaterZhangDuan05` | High pressure: 1 kbar to about 30 GPa, up to 2000 K. |

| Dielectric model | Use it for |
|---|---|
| `JohnsonNorton91` | The usual model up to 5 kbar. |
| `HelgesonKirkham74` | The HKF coefficients of Tanger and Helgeson (1988) and Sverjensky et al. (1997), which were fitted with it. |
| `BradleyPitzer79` | 0 to 350 degC. Tanger and Helgeson used it at low T above 1 kbar. It does not read the density. |
| `FernandezEtAl97` | The IAPWS formulation. |
| `SverjenskyEtAl14` | High pressure, in the Deep Earth Water model. |

1. Write the two methods in the record of water.
2. Set `implementation` only to reproduce the results of another code. `WaterHGK84` and `JohnsonNorton91` have `reaktoro` and `gems`.

The dielectric model changes the volume and the heat capacity of a solute. Use the model that its coefficients were fitted with:

```python
import thermofun as tf

water = tf.methods.solventMethod("WaterHGK84", implementation="reaktoro").properties(298.15, 1e5, 0)
for model, Q in (("HelgesonKirkham74", 5.90e-12), ("JohnsonNorton91", 6.63e-12)):
    born = tf.methods.dielectricMethod(model, implementation="reaktoro" if model == "JohnsonNorton91" else "").properties(298.15, 1e5, water, 0)
    assert abs(born.bornQ.val - Q) < 0.01e-12                   # 1/Pa
```

**Check:** the solvent of your engine is the record that you changed.

```python
engine = tf.ThermoEngine("examples/v2/minimal.yaml")
assert engine.solventSymbol() == "H2O@"
assert engine.calculationPath("Na+")["steps"][0]["methods"] == ["WaterIAPWS95", "JohnsonNorton91"]
```

Use `engine.setSolventSymbol("...")` if your database has another name for water.
