#ifndef FRANTZMARSHALL_H
#define FRANTZMARSHALL_H

#include "Common/Real.hpp"
#include "Reaction.h"
#include "ThermoProperties.h"

namespace ThermoFun {

auto thermoPropertiesFrantzMarshall(real TK, real Pbar, Reaction reaction, const PropertiesSolventAD& wp) -> ThermoPropertiesReactionAD;

}

#endif // FRANTZMARSHALL_H

