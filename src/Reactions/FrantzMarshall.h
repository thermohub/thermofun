#ifndef FRANTZMARSHALL_H
#define FRANTZMARSHALL_H

#include "Common/ThermoScalar.hpp"
#include "Reaction.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace TCorrPT {

auto thermoPropertiesFrantzMarshall(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Reaction reaction, PropertiesSolvent wp) -> ThermoPropertiesReaction;

}

#endif // FRANTZMARSHALL_H

