#ifndef SOLUTEHOLLANDPOWELL98_H
#define SOLUTEHOLLANDPOWELL98_H

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

struct ThermoPropertiesSubstance;
struct PropertiesSolvent;
class Substance;

auto thermoPropertiesAqSoluteHP98(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Substance subst, const PropertiesSolvent& wpr,  const PropertiesSolvent& wp) -> ThermoPropertiesSubstance;

}

#endif // SOLUTEHOLLANDPOWELL98_H
