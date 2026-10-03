#ifndef LOGK_FUNCTION_OF_T
#define LOGK_FUNCTION_OF_T

#include "Common/Real.hpp"
#include "Reaction.h"
#include "ThermoProperties.h"

namespace ThermoFun {

/// The reference properties of a reaction (with the errors and statuses) and the method used to calculate its
/// properties as functions of temperature
struct LogKInputs
{
    Reaktoro_::ThermoProperty dVr, dHr, dSr, dCpr, dGr, lgK;
    MethodCorrT_Thrift::type method;
};

/// Prepare the calculation of the properties of a reaction from the reference properties: the enthalpy or entropy
/// are calculated if missing and the method is determined if it is the 3-term one
auto prepareLogK_fT(Reaction reaction, double T, MethodCorrT_Thrift::type CE) -> LogKInputs;

/// Calculate the properties of the reaction (autodiff numbers) with the method given by prepareLogK_fT
auto thermoPropertiesReaction_LogK_fT(real TK, real Pbar, Reaction reaction, MethodCorrT_Thrift::type CE) -> ThermoPropertiesReactionAD;

/// Set the errors and the statuses of the properties calculated with thermoPropertiesReaction_LogK_fT
auto setStatusLogK_fT(ThermoPropertiesReaction& tpr, const LogKInputs& inputs) -> void;

}

#endif // LOGK_FUNCTION_OF_T

