#ifndef VOLUME_FUNCTION_OF_T
#define VOLUME_FUNCTION_OF_T

#include "Common/ThermoScalar.hpp"
#include "Reaction.h"
#include "ThermoProperties.h"

namespace TCorrPT {

auto thermoPropertiesReaction_Vol_fT(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Reaction reaction, ThermoPropertiesReaction tpr) -> ThermoPropertiesReaction;

}

#endif // VOLUME_FUNCTION_OF_T

