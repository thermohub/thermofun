#ifndef SOLUTEANDERSON91_H
#define SOLUTEANDERSON91_H
#include "ThermoProperties.h"


namespace ThermoFun {
class Substance;

auto thermoPropertiesAqSoluteAN91(real TK, real Pbar, Substance subst, const PropertiesSolventAD& wpr,  const PropertiesSolventAD& wp) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLUTEANDERSON91_H
