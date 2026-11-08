// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_REACTIONBINDING_H
#define THERMOFUN_ENGINE_REACTIONBINDING_H

#include <map>
#include <ThermoFun/Model/MethodDefinition.h>
#include <memory>
#include <string>

#include <ThermoFun/Methods/Reactions/ReactionMethods.h>
#include <ThermoFun/Model/ReactionRecord.h>

namespace ThermoFun::model {

/// The methods of a reaction, ready to run. Immutable.
struct BoundReaction
{
    std::shared_ptr<const ReactionRecord> record;
    std::shared_ptr<const ReactionMethod> base;    ///< log K, or the constant properties
    std::shared_ptr<const ReactionMethod> volume;  ///< null: no correction for the pressure
    bool needs_solvent = false;
};

/// True if the properties of the record come from its methods, every method has code, and one of them is a base method.
auto canBindReaction(const ReactionRecord& record, const std::map<std::string, MethodDefinition>* definitions = nullptr) -> bool;

/// Make the methods of a reaction.
auto bindReaction(std::shared_ptr<const ReactionRecord> record, const std::map<std::string, MethodDefinition>* definitions = nullptr) -> std::shared_ptr<const BoundReaction>;

/// The properties of a bound reaction at T (K) and P (Pa). solvent: if the methods need it.
auto executeReaction(const BoundReaction& bound, double T, double P, const PropertiesSolvent* solvent) -> ThermoPropertiesReaction;

/// The derivatives of the properties of a bound reaction by one parameter: each val is a derivative, V in J/bar per unit. method: the method of the parameter,
/// empty for a reference value (logK, dG, dH, dS, dCp, dV), which every method reads.
auto reactionParameterDerivative(const BoundReaction& bound, const std::string& method, const std::string& parameter, double T, double P, const PropertiesSolvent* solvent) -> ThermoPropertiesReaction;

} // namespace ThermoFun::model

#endif
