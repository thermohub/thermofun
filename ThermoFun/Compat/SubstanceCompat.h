// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_COMPAT_SUBSTANCECOMPAT_H
#define THERMOFUN_COMPAT_SUBSTANCECOMPAT_H

#include <map>
#include <string>

#include <ThermoFun/GlobalVariables.h>
#include <ThermoFun/Model/ReactionRecord.h>
#include <ThermoFun/Model/SubstanceRecord.h>
#include <ThermoFun/ThermoParameters.h>
#include <ThermoFun/ThermoProperties.h>

// The old getters of Substance, answered from its record: method codes, positional arrays, old units.

namespace ThermoFun::compat {

/// The old codes of the methods of a record, one for each slot. 0: no method in the slot.
struct MethodCodes
{
    MethodGenEoS_Thrift::type general = static_cast<MethodGenEoS_Thrift::type>(0);
    MethodCorrT_Thrift::type temperature = static_cast<MethodCorrT_Thrift::type>(0);
    MethodCorrP_Thrift::type pressure = static_cast<MethodCorrP_Thrift::type>(0);
};
auto methodCodes(const v2::SubstanceRecord& record) -> MethodCodes;

/// The parameters of the methods in the arrays of the old API, in the old units.
auto thermoParameters(const v2::SubstanceRecord& record) -> ThermoParametersSubstance;

/// The reference properties, with V in J/bar.
auto thermoReferenceProperties(const v2::SubstanceRecord& record) -> ThermoPropertiesSubstance;

/// The molar mass in g/mol.
auto molarMass(const v2::SubstanceRecord& record) -> double;

/// How the properties come: from methods, or from a reaction.
auto thermoCalculationType(const v2::SubstanceRecord& record) -> SubstanceThermoCalculationType::type;

/// The same for a reaction. The merged log K methods give back the general code of the polynomial.
auto methodCodes(const v2::ReactionRecord& record) -> MethodCodes;
auto thermoParameters(const v2::ReactionRecord& record) -> ThermoParametersReaction;
auto thermoReferenceProperties(const v2::ReactionRecord& record) -> ThermoPropertiesReaction;
auto reactants(const v2::ReactionRecord& record) -> std::map<std::string, double>;

} // namespace ThermoFun::compat

#endif
