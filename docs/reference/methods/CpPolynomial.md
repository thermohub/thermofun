# CpPolynomial

Heat capacity polynomial.

Integrates Cp = a0 + a1 T + a2/T2 + a3/sqrt(T) + a4 T2 + a5 T3 + a6 T4 + a7/T3 + a8/T + a9 sqrt(T) over T segments, with phase transitions.

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | yes |
| Has code | yes |
| Legacy names | `cp_ft_equation` |
| Other names | `CpIntegration` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a0 | J/(mol.K) | J/(mol.K) | yes |  | constant term |
| a1 | J/(mol.K2) | J/(mol.K2) | no |  | term a1 T |
| a2 | J.K/mol | J.K/mol | no |  | term a2 / T2 |
| a3 | J/(mol.K^0.5) | J/(mol.K^0.5) | no |  | term a3 / sqrt(T) |
| a4 | J/(mol.K3) | J/(mol.K3) | no |  | term a4 T2 |
| a5 | J/(mol.K4) | J/(mol.K4) | no |  | term a5 T3 |
| a6 | J/(mol.K5) | J/(mol.K5) | no |  | term a6 T4 |
| a7 | J.K2/mol | J.K2/mol | no |  | term a7 / T3 |
| a8 | J/mol | J/mol | no |  | term a8 / T |
| a9 | J/(mol.K^1.5) | J/(mol.K^1.5) | no |  | term a9 sqrt(T) |

## Example

From Akermanite in `aq17-thermofun.json`.

```yaml
- method: CpPolynomial
  segments:
    - t_min: 273.15
      t_max: 2273.15
      parameters:
        a0: 385.4
        a1: 0.003209
        a2: -247500
        a3: -2889.9
```

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "Akermanite")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -3.6699e+06  -3.8664e+06  212.5  216.21  9.254e-05
373.15  1e+07  -3.6869e+06  -3.8485e+06  263.17  235.2  9.2693e-05
```
