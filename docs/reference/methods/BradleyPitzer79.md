# BradleyPitzer79

Dielectric constant of Bradley and Pitzer.

Calculates the dielectric constant of water and the Born functions from T and P, with no density of water.

Source: Bradley and Pitzer (1979).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | water_solvent |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: T: K, P: bar. They are not converted.

Dielectric constant of water as a function of T and P, with no density: D = D1000 + C ln((B + P)/(B + 1000)), D1000 = U1 exp(U2 T + U3 T2), C = U4 + U5/(U6 + T), B = U7 + U8/T + U9 T, with T in K and P in bar. Tmin, Tmax in K, Pmax in bar: its range, up to Pmax_low below Tlow. Outside of it the method warns. Tanger and Helgeson (1988) used it below about 70 degC and above 1 kbar.

Source: Bradley D.J., Pitzer K.S. (1979) J. Phys. Chem. 83, 1599-1603, Table I.

Names: `U1` to `U9`, `Tmin`, `Tmax`, `Tlow`, `Pmax_low`, `Pmax`.

## Example

From Fernandez et al. (1997), Table 7, column B&P. No bundled database has this method yet.

```yaml
substances:
  H2O@:
    formula: H2O@
    class: water_solvent
    aggregate_state: aqueous
    standard_state_properties: {G: -237183.0, H: -285881.0, S: 69.923, Cp: 75.36053, V: {value: 1.8068397, unit: J/bar}}
    methods:
      - method: WaterHGK84
      - method: BradleyPitzer79
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| H2O@ | 298.15 | saturation | epsilon | 78.38 | 0.02 |
| H2O@ | 473.15 | 1000 bar | epsilon | 38.19 | 0.02 |
| H2O@ | 298.15 | 5000 bar | epsilon | 92.24 | 0.02 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(473.15, 1e+08), (298.15, 5e+08)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "H2O@")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
473.15  1e+08  -2.5071e+05  -2.7144e+05  103.07  74.593  1.9503e-05
298.15  5e+08  -2.2884e+05  -2.784e+05  66.9  62.35  1.5695e-05
```
