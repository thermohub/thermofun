#ifndef GASSRK
#define GASSRK
#include "ThermoProperties.h"


namespace ThermoFun {

class Substance;

auto thermoPropertiesGasSRK(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD;

}

#endif // GASSRK

