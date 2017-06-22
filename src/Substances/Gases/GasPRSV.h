#ifndef GASPRSV
#define GASPRSV

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

struct Substance;
struct ThermoPropertiesSubstance;

auto thermoPropertiesGasPRSV(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}


#endif // GASPRSV
