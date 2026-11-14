# PRSV86

Peng-Robinson-Stryjek-Vera equation of state.

Adds the pressure terms of the PRSV equation of state to G, H, S and Cp, and sets V.

Source: Stryjek and Vera (1986); Proust and Vera (1989).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `fluid_prsv` |
| Other names | `PRSV` |

## Implementations

`consistent`, `gems`. The default is `consistent`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc | K | K | yes |  | critical temperature |
| Pc | bar | Pa | yes |  | critical pressure |
| omega | 1 | 1 | yes |  | acentric factor |
| k1 | 1 | 1 | no |  | interaction parameter k1 |
| k2 | 1 | 1 | no |  | interaction parameter k2 |
| k3 | 1 | 1 | no |  | interaction parameter k3 |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: none: numbers. They are not converted.

Peng-Robinson-Stryjek-Vera equation of state. a = Oa R2 Tc2/Pc alpha, b = Ob R Tc/Pc, alpha = (1 + k (1 - sqrt(T/Tc)))2, k0 = k0_0 + k0_1 w - k0_2 w2 + k0_3 w3 with the omega w. k1, k2, k3 belong to each record. All are numbers without units.

Source: Stryjek R., Vera J.H. (1986) Can. J. Chem. Eng. 64, 323-333.

Names: `k0_0` to `k0_3`, `Oa`, `Ob`.

## Example

From CH4 in `mines16-thermofun.json`.

```yaml
- method: PRSV86
  units: {pressure: bar}
  parameters:
    Tc: 190.555
    Pc: 45.95
    omega: 0.01045
    k1: -0.00159
```

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("mines16-thermofun.json")
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "CH4")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -50514  -74588  186.36  35.811  0.024735
373.15  1e+07  -50775  -72910  154.1  45.113  0.00028991
```
