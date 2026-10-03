#ifndef GASCGF
#define GASCGF
#include "ThermoProperties.h"


namespace ThermoFun {
class Substance;

auto thermoPropertiesGasCGF(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD;

}

#endif // GASCGF

