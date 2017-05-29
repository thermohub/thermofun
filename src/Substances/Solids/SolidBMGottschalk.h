#ifndef SOLIDBMGOTTSCHALK
#define SOLIDBMGOTTSCHALK

#include "Substances/Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace ThermoFun {

struct ThermoPropertiesSubstance;

/// Returns the correcected (dVdP) themrodynamic properties of a substance (mineral) uisng the using the Birch-Murnaghan EOS
/// @ref Gottschalk M. (1997) Internally consistent thermodynamic data for rock-forming minerals in the system
/// SiO2-TiO2-Al2O3-Fe2O3-CaO-MgO-FeO-K2O-Na2O-H2O-CO2. Eur. J. Mineral., 9, 175-223.
/// @param t temparature (K)
/// @param p pressure (bar)
/// @param subst substance instance
/// @param tps structure holding the thermodynamicp porperties of the substance (previously) corrected with other models
auto thermoPropertiesMinBMGottschalk (Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // SOLIDBMGOTTSCHALK

