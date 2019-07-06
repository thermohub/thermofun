#ifndef GASCGF
#define GASCGF

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

struct ThermoPropertiesSubstance;
class Substance;

auto thermoPropertiesGasCGF(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance;

}

#endif // GASCGF

