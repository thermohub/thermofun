// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_BUILTINPARAMETERSETS_H
#define THERMOFUN_MODEL_BUILTINPARAMETERSETS_H

#include <map>
#include <string>
#include <vector>

#include <ThermoFun/Model/MethodParameterSet.h>

namespace ThermoFun::model {

// The constants of the methods: the files of Methods/ParameterSets, kept in the library when it is built. A file is named
// after its method, Method.yaml or Method-variant.yaml, and is read the first time that method needs it.

/// The default set of a method and implementation, else the default set of the method for all its implementations. nullptr if it has none.
auto builtinConstants(const std::string& method, const std::string& implementation = "") -> const MethodParameterSet*;

/// All the sets in the files of a method.
auto builtinSetsOf(const std::string& method) -> const std::vector<MethodParameterSet>&;

/// A variant by its symbol: a set of the library, or of a file in the folders of the environment variable THERMOFUN_PARAMETER_PATH.
/// A set from those folders is used only when a record names it. nullptr if there is none.
auto parameterSetVariant(const std::string& symbol) -> const MethodParameterSet*;

/// All the sets of the library, by symbol.
auto builtinParameterSets() -> const std::map<std::string, MethodParameterSet>&;

} // namespace ThermoFun::model

#endif
