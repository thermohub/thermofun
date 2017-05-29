#ifndef SOLIDMURNAGHANHP98_H
#define SOLIDMURNAGHANHP98_H

#include "Substances/Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace ThermoFun {

struct ThermoPropertiesSubstance;

/// Returns the  correcected themrodynamic properties of a substance (mineral) uisng the Murnaghan EoS (Holland and Powell, 1998)
/// @ref Holland T.J.B., Powell R. (1998) An internally consistent thermodynamic data set for phases of
/// petrological interest. Journal of Metamorphic Geology, 16, 309-343.
/// @param t temparature (K)
/// @param p pressure (bar)
/// @param subst substance instance
/// @param tps structure holding the thermodynamicp porperties of the substance (previously) corrected with other models
auto thermoPropertiesMinMurnaghanEOSHP98(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // SOLIDMURNAGHANHP98_H
