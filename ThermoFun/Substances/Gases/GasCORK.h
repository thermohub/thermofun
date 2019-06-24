#ifndef GASCORK
#define GASCORK

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

struct ThermoPropertiesSubstance;
struct Substance;

auto thermoPropertiesGasCORK(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // GASCORK

