# SverjenskyEtAl14

Dielectric constant of Sverjensky et al..

Calculates the dielectric constant of water at high pressure (DEW model). The published coefficients are for t in degC and the density in g/cm3.

Source: Sverjensky D.A., Harrison B., Azzolini D. (2014) Geochim. Cosmochim. Acta 129, 125-145.

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | water_solvent |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `water_diel_sverj14` |
| Other names | `Sverjensky14Diel` |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: T: degC, rho: g/cm3. They are not converted.

Dielectric constant of water at high pressure (Deep Earth Water model): eps = exp(u) rho^w, u = b1 t + b2 sqrt(t) + b3, w = a1 t + a2 sqrt(t) + a3, with t in degC and rho in g/cm3. The coefficients are those of the paper.

Source: Sverjensky D.A., Harrison B., Azzolini D. (2014) Geochim. Cosmochim. Acta 129, 125-145.

Names: `a1` to `a3`, `b1` to `b3`.

## Example

From the Deep Earth Water model, Sverjensky et al. (2014), Geochim. Cosmochim. Acta 129, 125-145. No bundled database has this method yet.

The water of the model DEW, from 1 kbar. The standard-state values are those of aq17.

```yaml
substances:
  H2O@:
    formula: H2O@
    class: water_solvent
    aggregate_state: aqueous
    standard_state_properties: {G: -237183.0, H: -285881.0, S: 69.923, Cp: 75.36053, V: {value: 1.8068397, unit: J/bar}}
    methods:
      - method: WaterZhangDuan05
      - method: SverjenskyEtAl14
```

`pytests/test_method_examples.py` checks that it calculates, at 1073.15 K and 10000 bar. There is no published value here to compare with.

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(1073.15, 1e+09)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "H2O@")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
1073.15  1e+09  0  0  0  0  2.0606e-05
```
