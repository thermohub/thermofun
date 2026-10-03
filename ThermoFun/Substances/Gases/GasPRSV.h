#ifndef GASPRSV
#define GASPRSV
#include "ThermoProperties.h"


namespace ThermoFun {

class Substance;

auto thermoPropertiesGasPRSV(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD;

}


#endif // GASPRSV
