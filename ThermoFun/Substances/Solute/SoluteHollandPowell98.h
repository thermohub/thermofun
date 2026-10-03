#ifndef SOLUTEHOLLANDPOWELL98_H
#define SOLUTEHOLLANDPOWELL98_H
#include "ThermoProperties.h"


namespace ThermoFun {
class Substance;

auto thermoPropertiesAqSoluteHP98(real TK, real Pbar, Substance subst, const PropertiesSolventAD& wpr,  const PropertiesSolventAD& wp) -> ThermoPropertiesSubstanceAD;

}

#endif // SOLUTEHOLLANDPOWELL98_H
