#ifndef SOLIDHPLANDAU
#define SOLIDHPLANDAU

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

struct Substance;
struct ThermoPropertiesSubstance;
/// Returns the  correcected themrodynamic properties of a substance (mineral) uisng the Holland-Powell phases with Landau transition
/// @ref Holland T.J.B., Powell R. (1998) An internally consistent thermodynamic data set for phases of
/// petrological interest. Journal of Metamorphic Geology, 16, 309-343.
/// @param t temparature (K)
/// @param p pressure (bar)
/// @param subst substance instance
/// @param tps structure holding the thermodynamicp porperties of the substance (previously) corrected with other models
auto thermoPropertiesHPLandau(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // SOLIDHPLANDAU

