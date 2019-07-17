#ifndef SOLUTEANDERSON91_H
#define SOLUTEANDERSON91_H

#include "Common/ScalarTypes.hpp"

namespace ThermoFun {

struct ThermoPropertiesSubstance;
struct PropertiesSolvent;
class Substance;

auto thermoPropertiesAqSoluteAN91(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Substance subst, const PropertiesSolvent& wpr,  const PropertiesSolvent& wp) -> ThermoPropertiesSubstance;

}

#endif // SOLUTEANDERSON91_H
