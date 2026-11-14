# LandauHP98

Landau transition of Holland and Powell.

Adds the Landau term of a phase transition to G, H, S and Cp, and sets V. Implementation consistent (the default): the volume term is integrated from the reference pressure, and has its Cp. Implementation gems: from P = 0, without that Cp.

Source: Holland and Powell (1998).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | component |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `landau_holland_powell98` |
| Other names | `HP98Landau` |

## Implementations

`consistent`, `gems`. The default is `consistent`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc0 | Cel | K | yes |  | critical temperature at 1 bar |
| Smax | J/(mol.K) | J/(mol.K) | yes |  | maximum entropy of the transition |
| Vmax | J/(mol.bar) | m3/mol | yes |  | maximum volume of the transition |
| alpha0 | 1/K | 1/K | no |  | thermal expansion at the reference state |
| kappa0 | kbar | Pa | no |  | bulk modulus at 298 K |

## Example

From Albite in `aq17-thermofun.json`.

```yaml
- method: LandauHP98
  units: {pressure: kbar}
  parameters:
    Tc0: 676.85
    Smax: 16
    Vmax: 0.124
    alpha0: 4.56e-05
    kappa0: 593
```

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("aq17-thermofun.json")
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "Albite")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -3.712e+06  -3.9346e+06  210.1  207.73  0.00010029
373.15  1e+07  -3.7287e+06  -3.9168e+06  260.09  237.2  0.00010046
```
