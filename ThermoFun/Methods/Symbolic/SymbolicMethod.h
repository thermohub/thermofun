// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_SYMBOLIC_SYMBOLICMETHOD_H
#define THERMOFUN_METHODS_SYMBOLIC_SYMBOLICMETHOD_H

#include <memory>
#include <string>
#include <vector>

#include <ThermoFun/Common/Diagnostics.h>
#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Methods/Reactions/ReactionMethods.h>
#include <ThermoFun/Model/ReactionRecord.h>
#include <ThermoFun/Model/MethodApplication.h>
#include <ThermoFun/Model/MethodDefinition.h>

namespace ThermoFun::v2 {

/// The names that an expression of a substance method can use: T, P, the reference values, R and the parameters of the method.
auto symbolicVariables(const MethodDefinition& definition) -> std::vector<std::string>;

/// True if the definition has an expression that a substance can run.
auto isSymbolicSubstanceDefinition(const MethodDefinition& definition) -> bool;

/// True if the expression gives G, which makes it a base method; else it adds to what comes before.
auto isSymbolicBase(const MethodDefinition& definition) -> bool;

/// The method object of a Method record of the database. An error (V2-MTH-008) if the library has no SymEngine.
auto symbolicMethod(const MethodDefinition& definition, const MethodApplication& application) -> std::shared_ptr<const SubstanceMethod>;

/// True if the definition has an expression of dG that a reaction can run.
auto isSymbolicReactionDefinition(const MethodDefinition& definition) -> bool;

/// The method object of a Method record for a reaction, with its reference values.
auto symbolicReactionMethod(const MethodDefinition& definition, const MethodApplication& application, const ReactionRecord& record) -> std::shared_ptr<const ReactionMethod>;

/// The checks of a symbolic Method record: its expressions (V2-SYM-001, 002, 006), its source (003), its test cases (004) and its units (005).
auto checkSymbolicDefinition(const MethodDefinition& definition, Diagnostics& diagnostics) -> void;

} // namespace ThermoFun::v2

#endif
