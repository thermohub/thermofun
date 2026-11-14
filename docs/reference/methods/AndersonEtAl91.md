# AndersonEtAl91

Aqueous solute of Anderson et al..

Calculates the properties of an aqueous species from the density model of Anderson et al.

Source: Anderson et al. (1991).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | aqueous_solute |
| Needs | the solvent |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `solute_anderson91` |
| Other names | `Anderson91` |

## Example

From Anderson, Castet, Schott and Mesmer (1991); the reference values are SiO2@ of aq17 (SUPCRT92). No bundled database has this method yet.

The density model needs only the reference values of the species and the water of the database.

```yaml
substances:
  SiO2@:
    formula: SiO2@
    class: aqueous_solute
    aggregate_state: aqueous
    standard_state_properties: {G: -834041.0, H: -878357.0, S: 75.312, Cp: -318.484, V: {value: 1.60633, unit: J/bar}}
    methods:
      - method: AndersonEtAl91
```

`pytests/test_method_examples.py` checks that it calculates, at 473.15 K and saturation. There is no published value here to compare with.

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(298.15, 100000), (373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "SiO2@")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -8.3404e+05  -8.7836e+05  75.312  -318.48  1.6063e-05
373.15  1e+07  -8.3797e+05  -8.9638e+05  22.395  -210.07  1.6063e-05
```
