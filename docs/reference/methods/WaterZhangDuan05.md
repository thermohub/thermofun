# WaterZhangDuan05

Water, Zhang and Duan.

Calculates the properties of water at high pressure from the equation of Zhang and Duan.

Source: Zhang and Duan (2005).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | water_solvent |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `water_pvt_zhang_duan05` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| dew_gibbs | 1 | 1 | no | 0 | 1: the Gibbs energy of the model DEW (P >= 1 kbar) |
| state | 1 | 1 | no | 0 | 0: liquid, 1: gas |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: T: K, P: Pa, V: cm3/mol, g1kb: T in degC and G in cal/mol. They are not converted.

Water at high pressure and temperature. a1 to a14, gamma: coefficients of the equation of state; Tc in K, Vc in cm3/mol, R in cm3 bar/(K mol), as in the formulas. g1kb_0 to g1kb_4: G(T, 1 kbar) of the DEW model, a polynomial in T (degC) in cal/mol, highest power first. Tmax in K, Pmin and Pmax in Pa: the range of the model; outside of it the calculation stops with an error.

Source: Zhang Z., Duan Z. (2005) Phys. Earth Planet. Inter. 149, 335-354; Sverjensky et al. (2014).

Names: `a1` to `a14`, `gamma`, `Tc`, `Vc`, `R`, `g1kb_0` to `g1kb_4`, `Tmax`, `Pmax`, `Pmin`.

## Example

From the Deep Earth Water model, Zhang and Duan (2005) and Sverjensky et al. (2014). No bundled database has this method yet.

Valid from 1 kbar. The standard-state values are those of aq17.

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

`pytests/test_method_examples.py` checks that it calculates, at 773.15 K and 5000 bar. There is no published value here to compare with.

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(773.15, 5e+08)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "H2O@")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
773.15  5e+08  0  0  0  0  2.0788e-05
```
