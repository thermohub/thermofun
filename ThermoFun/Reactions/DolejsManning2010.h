#ifndef DOLEJSMANNING2010_H
#define DOLEJSMANNING2010_H

#include "Common/Real.hpp"
#include "Reaction.h"
#include "ThermoProperties.h"

namespace ThermoFun {

auto thermoPropertiesDolejsManning2010(real TK, real Pbar, Reaction reaction, const PropertiesSolventAD& wp) -> ThermoPropertiesReactionAD;

}

#endif // DOLEJSMANNING2010_H

