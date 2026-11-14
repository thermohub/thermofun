# NASA9

NASA polynomials.

Calculates the properties from NASA 9-coefficient polynomials, by T segment.

Source: McBride, Zehe and Gordon (2002).

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Classes | any |
| Needs | nothing from outside its record |
| Temperature segments | yes |
| Has code | yes |
| Convention of its results | assigned_enthalpy |
| Legacy names | `cp_nasa_polynomials` |
| Other names | `StandardThermoModelNasa` |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| a1 | 1 | 1 | yes |  | coefficient of Cp/R |
| a2 | 1 | 1 | yes |  | coefficient of Cp/R |
| a3 | 1 | 1 | yes |  | coefficient of Cp/R |
| a4 | 1 | 1 | yes |  | coefficient of Cp/R |
| a5 | 1 | 1 | yes |  | coefficient of Cp/R |
| a6 | 1 | 1 | yes |  | coefficient of Cp/R |
| a7 | 1 | 1 | yes |  | coefficient of Cp/R |
| b1 | K | K | yes |  | integration constant of H |
| b2 | 1 | 1 | yes |  | integration constant of S |

## Example

From McBride, Zehe and Gordon (2002), NASA/TP-2002-211556, CO2 from 200 to 1000 K. No bundled database has this method yet.

NASA polynomials give H on the scale of the assigned enthalpy (the enthalpy of formation at 298.15 K); ThermoFun converts the results to the convention of the engine.

```yaml
substances:
  CO2(g):
    formula: CO2
    class: gas_fluid
    aggregate_state: gas
    methods:
      - method: NASA9
        segments:
          - t_min: 200.0
            t_max: 1000.0
            parameters: {a1: 4.943650540e+4, a2: -6.264116010e+2, a3: 5.301725240, a4: 2.503813816e-3, a5: -2.127308728e-7,
                         a6: -7.689988780e-10, a7: 2.849677801e-13, b1: -4.528198460e+4, b2: -7.048279440}
```

Published values, checked by `pytests/test_method_examples.py`:

| Record | T, K | P | Property | Published | Tolerance |
|---|---|---|---|---|---|
| CO2(g) | 298.15 | 1 bar | Cp | 37.135 | 0.01 |

Calculation, with the record as above:

```python
import thermofun as tf

engine = tf.ThermoEngine("record.yaml")  # the record above, in a file
for T, P in [(298.15, 100000)]:  # K, Pa
    p = engine.thermoPropertiesSubstance(T, P, "CO2(g)")
    print(T, P, p.gibbs_energy.val, p.enthalpy.val, p.entropy.val, p.heat_capacity_cp.val, p.volume.val)
```

The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does.

Output, as `T  P  G  H  S  Cp  V` (J/mol, J/(mol K), J/(mol K), J/(mol K), m3/mol):

```text
298.15  100000  -3.9438e+05  -3.9351e+05  213.79  37.135  0
```
