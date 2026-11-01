<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Propagate uncertainties

**You need:** ThermoFun with autodiff.

1. Give the uncertainties in the file. Correlations go in `covariances`: see [Add data](add-data.md).

<!-- file: uncertain.yaml -->
```yaml
thermohub: 2.0.0-draft.1
substances:
  X:
    formula: SiO2
    class: component
    aggregate_state: crystal
    standard_state_properties:
      G: {value: -856000.0, uncertainty: 1000.0}
      H: -910000.0
      S: {value: 41.5, uncertainty: 0.5}
      Cp: 44.9
      V: 2.27e-5
    methods:
      - method: CpPolynomial
        segments:
          - t_min: 273.15
            t_max: 2273.15
            parameters: {a0: {value: 100.0, uncertainty: 2.0}}
```

2. Read `err` of a result. The reference values always count.

```python
import math
import thermofun as tf

T = 500.0
engine = tf.ThermoEngine("uncertain.yaml")
G = engine.thermoPropertiesSubstance(T, 1e5, "X").gibbs_energy
assert abs(G.err - math.hypot(1000.0, (T - 298.15) * 0.5)) < 1e-6   # J/mol
```

3. Set `propagate_parameter_errors` to count the parameters of the methods too. It takes more time.

```python
preferences = engine.preferences
preferences.propagate_parameter_errors = True
engine.setPreferences(preferences)
with_parameters = engine.thermoPropertiesSubstance(T, 1e5, "X").gibbs_energy
assert with_parameters.err > G.err
```

**Check:** see the text in [Uncertainties and rounding](../explanation/uncertainty.md) for the formula and its limits.
