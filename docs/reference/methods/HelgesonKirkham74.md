# HelgesonKirkham74

Dielectric constant of Helgeson and Kirkham.

Calculates the dielectric constant of water and the Born functions from a polynomial in T and the density. HKF coefficients of Tanger and Helgeson (1988) and Sverjensky et al. (1997) were fitted with it.

Source: Helgeson and Kirkham (1974); Tanger and Helgeson (1988).

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

Units of the values: T: K, rho: g/cm3. They are not converted.

Dielectric constant of water, from which the HKF coefficients of Tanger and Helgeson (1988) were fitted: eps = sum over i, j of e_i_j T^i rho^j, with T in K and rho in g/cm3. The paper prints the first row of Table E-1 as e(0,1) to e(0,5): these are e_0_0 to e_0_4 of eq. E-1, which gives eps = 78.46 at 298.15 K. Tanger and Helgeson used the equations of Bradley and Pitzer (1979), the method BradleyPitzer79, at low T above 1 kbar; this method does not switch to it.

Source: Helgeson H.C., Kirkham D.H. (1974) Am. J. Sci. 274, 1089-1198; Tanger J.C., Helgeson H.C. (1988) Am. J. Sci. 288, 19-98, Appendix E.

Names: `e_0_0` to `e_0_4`, `e_1_0` to `e_1_3`, `e_2_0` to `e_2_2`, `e_3_0`, `e_3_1`, `e_4_0`.

## Example

From Fernandez et al. (1997), Table 7, column H&K. No bundled database has this method yet.

```yaml
substances:
  H2O@:
    formula: H2O@
    class: water_solvent
    aggregate_state: aqueous
    standard_state_properties: {G: -237183.0, H: -285881.0, S: 69.923, Cp: 75.36053, V: {value: 1.8068397, unit: J/bar}}
    methods:
      - method: WaterHGK84
      - method: HelgesonKirkham74
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| H2O@ | 298.15 | saturation | epsilon | 78.47 | 0.02 |
| H2O@ | 373.15 | 1000 bar | epsilon | 58.55 | 0.02 |
| H2O@ | 473.15 | 1000 bar | epsilon | 38.27 | 0.02 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(373.15, 1e+08), (473.15, 1e+08)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "H2O@")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
373.15  1e+08  -2.4125e+05  -2.788e+05  85.6  72.767  1.8021e-05
473.15  1e+08  -2.5071e+05  -2.7144e+05  103.07  74.593  1.9503e-05
```
