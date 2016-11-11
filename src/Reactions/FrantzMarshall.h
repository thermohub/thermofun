#ifndef FRANTZMARSHALL_H
#define FRANTZMARSHALL_H

#include "Common/ThermoScalar.hpp"
#include "Reaction.h"
#include "ThermoProperties.h"

namespace TCorrPT {

auto thermoPropertiesFrantzMarshall(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Reaction reaction, PropertiesSolvent wp) -> ThermoPropertiesReaction;

}

#endif // FRANTZMARSHALL_H

