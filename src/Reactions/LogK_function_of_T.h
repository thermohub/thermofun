#ifndef LOGK_FUNCTION_OF_T
#define LOGK_FUNCTION_OF_T

#include "Common/ThermoScalar.hpp"
#include "Reaction.h"
#include "ThermoProperties.h"

namespace TCorrPT {

auto thermoPropertiesReaction_LogK_fT(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Reaction reaction, MethodCorrT_Thrift::type CE) -> ThermoPropertiesReaction;

}

#endif // LOGK_FUNCTION_OF_T

