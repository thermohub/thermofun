#ifndef DOLEJSMANNING2010_H
#define DOLEJSMANNING2010_H

#include "Common/ThermoScalar.hpp"
#include "Reaction.h"
#include "ThermoProperties.h"

namespace ThermoFun {

auto thermoPropertiesDolejsManning2010(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Reaction reaction, PropertiesSolvent wp) -> ThermoPropertiesReaction;

}

#endif // DOLEJSMANNING2010_H

