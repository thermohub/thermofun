// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef LOGK_FUNCTION_OF_T
#define LOGK_FUNCTION_OF_T

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/Methods/Reactions/ReactionInputs.h>

namespace ThermoFun {

/// The reference properties of a reaction (with the errors and statuses) and the method used to calculate its
/// properties as functions of temperature
struct LogKInputs
{
    Reaktoro_::ThermoProperty dVr, dHr, dSr, dCpr, dGr, lgK;
    LogKForm method;
    double R = R_CONSTANT;   ///< the gas constant of the calculation
};

/// Which given reference values the forms of log K read; the others are derived from them at Tr, a given one too.
/// The default reads log K (else dG) and dH, and derives dS: the values that data compilations select (NEA TDB); dS is read
/// only without dH, dH is derived from dS then. gems: the rule of GEMS, given dH and dS are read and log K or dG only
/// completes them; a given dG wins over log K.
struct LogKReading
{
    bool logK = false, dG = false, dH = false, dS = false;
};
auto logKReading(const ThermoPropertiesReaction& reference, bool gems) -> LogKReading;
/// The same from which values the record gives
auto logKReading(LogKReading given, bool gems) -> LogKReading;

/// Prepare the calculation of the properties of a reaction from the reference properties: the enthalpy or entropy
/// are calculated if missing and the method is determined if it is the 3-term one
auto prepareLogK_fT(const ReactionInputs& reaction, double T, LogKForm CE) -> LogKInputs;

/// Calculate the properties of the reaction (autodiff numbers) with the method given by prepareLogK_fT
auto thermoPropertiesReaction_LogK_fT(real TK, real Pbar, const ReactionInputs& reaction, LogKForm CE) -> ThermoPropertiesReactionAD;

/// Set the errors and the statuses of the properties calculated with thermoPropertiesReaction_LogK_fT
/// @param T temperature (K), P pressure (Pa)
auto setStatusLogK_fT(ThermoPropertiesReaction& tpr, const LogKInputs& inputs, double T, double P) -> void;

}

#endif // LOGK_FUNCTION_OF_T

