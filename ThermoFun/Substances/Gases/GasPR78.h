#ifndef GASPR78
#define GASPR78
#include "ThermoProperties.h"


namespace ThermoFun {

class Substance;

auto thermoPropertiesGasPR78(real t, real p, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD;

}

#endif // GASPR78

