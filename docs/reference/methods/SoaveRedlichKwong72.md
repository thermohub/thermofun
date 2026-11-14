# SoaveRedlichKwong72

Soave-Redlich-Kwong equation of state.

Adds the pressure terms of the SRK equation of state to G, H, S and Cp, and sets V.

Source: Soave (1972); Soave (1993).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `fluid_soave_redlich_kwong` |
| Other names | `SRK` |

## Implementations

`consistent`, `gems`. The default is `consistent`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc | K | K | yes |  | critical temperature |
| Pc | bar | Pa | yes |  | critical pressure |
| omega | 1 | 1 | yes |  | acentric factor |
| N | 1 | 1 | no |  | empirical parameter of the attraction term |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: none: numbers. They are not converted.

Soave-Redlich-Kwong equation of state. a = Oa R2 Tc2/Pc alpha, b = Ob R Tc/Pc, alpha = (1 + m (1 - sqrt(T/Tc)))2, m = m0 + m1 w - m2 w2 with the omega w. All are numbers without units.

Source: Soave G. (1972) Chem. Eng. Sci. 27, 1197-1203.

Names: `m0` to `m2`, `Oa`, `Ob`.

## Example

From the critical constants of CO2 in THEREDA (pytests/data/v2/thereda-co2.json). No bundled database has this method yet.

```yaml
substances:
  CO2(g):
    formula: CO2
    class: gas_fluid
    aggregate_state: gas
    standard_state_properties: {G: -394372.57, H: -393519.66, S: 213.7527, Cp: 37.135}
    methods:
      - method: ConstantCp
      - method: SoaveRedlichKwong72
        parameters: {Tc: 304.12, Pc: {value: 73.74, unit: bar}, omega: 0.225}
```

`pytests/test_method_examples.py` checks that it calculates, at 373.15 K and 100 bar. There is no published value here to compare with.

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(373.15, 1e+07)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "CO2(g)")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
373.15  1e+07  -3.9715e+05  -3.9386e+05  177.31  63.284  0.00024
```
