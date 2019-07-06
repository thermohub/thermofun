#ifndef GASPR78
#define GASPR78

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

class Substance;
struct ThermoPropertiesSubstance;

auto thermoPropertiesGasPR78(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // GASPR78

