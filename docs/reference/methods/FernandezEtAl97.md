# FernandezEtAl97

Dielectric constant of Fernandez et al..

Calculates the dielectric constant of water from the IAPWS formulation.

Source: Fernandez D.P., Goodwin A.R.H., Lemmon E.W., Levelt Sengers J.M.H., Williams R.C. (1997) J. Phys. Chem. Ref. Data 26, 1125-1166.

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | water_solvent |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `water_diel_fern97` |
| Other names | `Fernandez97Diel` |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: T: K, rho: kg/m3. They are not converted.

Dielectric constant of water, IAPWS formulation: g = 1 + n12 r w^-1.2 + sum n_i r^I_i (Tc/T)^J_i, r = rho/rhoc, w = T/228 - 1. n: coefficients; I, J: exponents; Tc in K, rhoc in kg/m3. k, Na, alpha, epsilon, mu, M: the physical constants of the molecule and of vacuum, in SI, as in the paper.

Source: Fernandez D.P., Goodwin A.R.H., Lemmon E.W., Levelt Sengers J.M.H., Williams R.C. (1997) J. Phys. Chem. Ref. Data 26, 1125-1166.

Names: `n1` to `n12`, `I1` to `I11`, `J1` to `J11`, `Tc`, `rhoc`, `k`, `Na`, `alpha`, `epsilon0`, `mu`, `M`.

## Example

From Fernandez et al. (1997), J. Phys. Chem. Ref. Data 26, 1125-1166, Table 7 (column "This work"). No bundled database has this method yet.

The water of the IAPWS release on the dielectric constant. The standard-state values are those of aq17.

```yaml
substances:
  H2O@:
    formula: H2O@
    class: water_solvent
    aggregate_state: aqueous
    standard_state_properties: {G: -237183.0, H: -285881.0, S: 69.923, Cp: 75.36053, V: {value: 1.8068397, unit: J/bar}}
    methods:
      - method: WaterIAPWS95
      - method: FernandezEtAl97
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| H2O@ | 298.15 | saturation | epsilon | 78.41 | 0.02 |
| H2O@ | 373.15 | 1000 bar | epsilon | 58.67 | 0.02 |
| H2O@ | 673.15 | 1000 bar | epsilon | 15.82 | 0.02 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(373.15, 1e+08), (673.15, 1e+08)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "H2O@")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
373.15  1e+08  -2.4125e+05  -2.788e+05  85.606  72.755  1.802e-05
673.15  1e+08  -2.7422e+05  -2.5545e+05  131.12  88.17  2.5999e-05
```
