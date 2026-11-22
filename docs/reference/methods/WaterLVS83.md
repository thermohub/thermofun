# WaterLVS83

Water near the critical point, Levelt Sengers.

Calculates the properties of water near its critical point from the scaling equation of Levelt Sengers et al., in its region only (SUPCRT92: about 643 to 695 K, 200 to 420 kg/m3, and its saturation from 643 K to the critical point). Elsewhere the next equation of state of the record is used; it has no metastable states.

Source: Levelt Sengers J.M.H., Kamgar-Parsi B., Balfour F.W., Sengers J.V. (1983) J. Phys. Chem. Ref. Data 12, 1-28.

| | |
|---|---|
| Works on | substance |
| Kind | base method |
| Contributes to | G sets, H sets, S sets, Cp sets, V sets |
| Classes | water_solvent, gas_fluid |
| Needs | nothing from outside its record |
| Temperature segments | no |
| Has code | yes |

## Parameters

| Name | Unit of the source | SI unit | Required | Default | Description |
|---|---|---|---|---|---|
| state | 1 | 1 | no | 0 | 0: liquid, 1: gas |

## Model parameters

Parameters of the model, the same for every record. A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.

Units of the values: T: K, P: MPa, rho: kg/m3. They are not converted.

The equation of state of water near the critical point of Levelt Sengers et al. (1983), as in SUPCRT92 and the water code of GEMS. co_a and co_q: its coefficients (arrays a and q of the code); cr_: the critical point (Tc K, rhoC kg/m3, Pc MPa) and the reducing constants. Region where it is used (SUPCRT92): Tmin1 <= T <= Tmax and Pmin(T) <= P <= Pmax(T), with Pmin = Pbase1 + PTmins (T - Tmin1) and Pmax = Pbase2 + PTmaxs (T - Tmin2) (Dmin and Dmax, kg/m3, are the isochores that these pressure bounds follow in SUPCRT92: they only describe the bounds, and changing them does not change the coverage; change the pressure coefficients); below Tmin2 also the vapour side up to the saturation pressure; its saturation from Tmin1 to Tc. Ttr: the temperature of the triple point, K, for the reference of the energies. The equation has no metastable states: off the saturation curve it gives only the stable phase.

Source: Levelt Sengers J.M.H., Kamgar-Parsi B., Balfour F.W., Sengers J.V. (1983) J. Phys. Chem. Ref. Data 12, 1-28; Johnson J.W., Oelkers E.H., Helgeson H.C. (1992) Comput. Geosci. 18, 899-947 (SUPCRT92).

Names: `co_a_1` to `co_a_20`, `co_q_1` to `co_q_20`, `cr_Tc`, `cr_rhoC`, `cr_Pc`, `cr_Pcon`, `cr_Ucon`, `cr_Scon`, `cr_dPcon`, `Ttr`, `Tmin1`, `Tmin2`, `Tmax`, `Dmin`, `Dmax`, `Pbase1`, `Pbase2`, `PTmins`, `PTmaxs`.

## Example

No bundled record uses this method.
