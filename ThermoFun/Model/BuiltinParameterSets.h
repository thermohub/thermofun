// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_BUILTINPARAMETERSETS_H
#define THERMOFUN_MODEL_BUILTINPARAMETERSETS_H

#include <map>
#include <string>

#include <ThermoFun/Model/MethodParameterSet.h>

namespace ThermoFun::v2 {

/// The constants of the methods, by method: the files of Methods/ParameterSets, kept in the library when it is built. They are read once.
auto builtinParameterSets() -> const std::map<std::string, MethodParameterSet>&;

} // namespace ThermoFun::v2

#endif
