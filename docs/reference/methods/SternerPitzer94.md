# SternerPitzer94

Equation of state of Sterner and Pitzer.

Adds the pressure terms of the Sterner-Pitzer equation of state to G, H, S and Cp, and sets V.

Source: Sterner and Pitzer (1994).

| | |
|---|---|
| Works on | substance |
| Kind | correction |
| Classes | gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |
| Legacy names | `fluid_sterner_pitzer` |
| Other names | `SternerPitzer` |

## Implementations

`consistent`, `gems`. The default is `consistent`. Use another only to reproduce the results of that code.

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| Tc | K | K | yes |  | critical temperature |
| Pc | bar | Pa | yes |  | critical pressure |

## Built-in constants

A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: T: K, P: MPa, rho: kg/m3, M: g/mol. They are not converted.

Equation of state of Sterner and Pitzer, in the units of the formulas. cfh_N, cfc_N: its 10 x 6 tables of coefficients for H2O and for CO2, by rows, cfh_1 to cfh_6 being the first row. TMIN, TMAX in K and PMIN, PMAX in MPa: its range. cs: any other fluid, the corresponding states form of Holland and Powell (1991), from Tc and Pc. psat_co2, psat_h2o: saturation pressure and densities used for the starting density; Rhoc in kg/m3. Mw_h2o, Mw_co2 in g/mol.

Source: Sterner S.M., Pitzer K.S. (1994) Contrib. Mineral. Petrol. 117, 362-374.

Names: `TMIN`, `TMAX`, `PMIN`, `PMAX`, `cfh_1` to `cfh_60`, `cfc_1` to `cfc_60`, `cs_a0`, `cs_a1`, `cs_b0`, `cs_c0`, `cs_c1`, `cs_d0`, `cs_d1`, `psat_co2_Rhoc`, `psat_co2_a1` to `psat_co2_a4`, `psat_co2_b1` to `psat_co2_b4`, `psat_co2_c1` to `psat_co2_c5`, `psat_h2o_Rhoc`, `psat_h2o_a1` to `psat_h2o_a6`, `psat_h2o_b1` to `psat_h2o_b6`, `psat_h2o_c1` to `psat_h2o_c6`, `Mw_h2o`, `Mw_co2`.

## Example

From Sterner and Pitzer (1994); the critical constants of CO2 in THEREDA. No bundled database has this method yet.

The model has built-in coefficients for H2O and CO2 only; the record gives which one by its critical constants.

```yaml
substances:
  CO2(g):
    formula: CO2
    class: gas_fluid
    aggregate_state: gas
    standard_state_properties: {G: -394372.57, H: -393519.66, S: 213.7527, Cp: 37.135}
    methods:
      - method: ConstantCp
      - method: SternerPitzer94
        parameters: {Tc: 304.12, Pc: {value: 73.74, unit: bar}}
```

`pytests/test_method_examples.py` checks that it calculates, at 773.15 K and 1000 bar. There is no published value here to compare with.
