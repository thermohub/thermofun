#ifndef GASCORK
#define GASCORK
#include "ThermoProperties.h"


namespace ThermoFun {
class Substance;

auto thermoPropertiesGasCORK(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD;

}

#endif // GASCORK

