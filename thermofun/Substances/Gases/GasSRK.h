#ifndef GASSRK
#define GASSRK

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

struct Substance;
struct ThermoPropertiesSubstance;

auto thermoPropertiesGasSRK(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // GASSRK

