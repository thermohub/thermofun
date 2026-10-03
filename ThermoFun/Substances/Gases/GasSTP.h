#ifndef GASSTP
#define GASSTP
#include "ThermoProperties.h"


namespace ThermoFun {

class Substance;

auto thermoPropertiesGasSTP(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD;

}


#endif // GASSTP

