// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef THERMOFUN_THERMOPROPERTIESSTATUS_H
#define THERMOFUN_THERMOPROPERTIESSTATUS_H

#include <string>
#include "ThermoProperties.h"

namespace ThermoFun {

/// Add "<property> not defined; " to message_ if the status is notdefined, else replace message_ by calcMessage.
auto setMessage(Reaktoro_::Status sta, std::string propertyOfsymbol, std::string calcMessage, std::string &message_ ) -> void;

/// Add the message to the status text of every property of tps that has the status sta.
auto setMessage(Reaktoro_::Status sta, std::string message , ThermoPropertiesSubstance &tps) -> void;

/// Add the message to the status text of every property of tpr that has the status sta.
auto setMessage(Reaktoro_::Status sta, std::string message, ThermoPropertiesReaction &tpr ) -> void;

} // namespace ThermoFun

#endif // THERMOFUN_THERMOPROPERTIESSTATUS_H
