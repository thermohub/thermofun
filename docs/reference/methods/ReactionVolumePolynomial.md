# ReactionVolumePolynomial

Volume of reaction, T and P.

Calculates dV(T, P) from expansion and compressibility terms, and adds the terms to log K, dG and dH.

| | |
|---|---|
| Works on | reaction |
| Kind | correction |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `dr_volume_fpt` |
| Other names | `DrVolumeFT` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a1 | 1/K | 1/K | yes |  | thermal expansion, first coefficient |
| a2 | 1/K2 | 1/K2 | yes |  | thermal expansion, second coefficient |
| a3 | 1/K3 | 1/K3 | yes |  | thermal expansion, third coefficient |
| b1 | 1/bar | 1/Pa | yes |  | compressibility, first coefficient |
| b2 | 1/bar2 | 1/Pa2 | yes |  | compressibility, second coefficient |

## Example

From the form of the method (dV = dVr (1 + a1 dT + a2 dT2 + a3 dT3 + b1 dP + b2 dP2)); the coefficients are illustrative values of the order of a solid. No bundled database has this method yet.

The reference volume dV is of the reaction. The volume adds a pressure term to log K, so log K at 1 bar is the value of the record, and it changes with pressure.

```yaml
reactions:
  Calcite-dissolution:
    equation: Calcite = Ca+2 + CO3-2
    reaction_properties: {logK: -8.48, dH: -9610.6, dV: -3.0e-5}
    methods:
      - method: LogKVantHoff
      - method: ReactionVolumePolynomial
        parameters: {a1: 2.0e-5, a2: 0.0, a3: 0.0, b1: -2.0e-11, b2: 0.0}
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| Calcite-dissolution | 298.15 | 1 bar | logK | -8.48 | 1e-09 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(298.15, 100000), (298.15, 1e+08)]:  # K, Pa
    p = engine.thermoPropertiesReaction(T, P, "Calcite-dissolution")
    print(T, P, p.log_equilibrium_constant.val, p.reaction_gibbs_energy.val, p.reaction_enthalpy.val, p.reaction_entropy.val, p.reaction_heat_capacity_cp.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  logK  dG  dH  dS  dCp` (no unit, J/mol, J/mol, J/(mol K), J/(mol K)):

```text
298.15  100000  -8.48  48404  -9610.6  -194.58  0
298.15  1e+08  -7.9555  45410  -12587  -194.52  0
```
