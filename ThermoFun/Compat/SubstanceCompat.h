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

// The old getters and setters of Substance and Reaction, translated to and from their records: method codes, positional
// arrays, old units.

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

/// Replaces the methods of a record with those of the old codes, with the parameters taken from the arrays. Goes through
/// the legacy reader, so the rules of a file apply. Gives an error if a code has no method; a code that asks for nothing
/// (for example CPM_NUL) leaves the slot empty. Methods kept from the record keep what the arrays cannot hold.
void setMethods(v2::SubstanceRecord& record, const MethodCodes& codes, const ThermoParametersSubstance& parameters);
void setMethods(v2::ReactionRecord& record, const MethodCodes& codes, const ThermoParametersReaction& parameters);

/// The reference properties of the old API (V in J/bar) in a record.
void setThermoReferenceProperties(v2::SubstanceRecord& record, const ThermoPropertiesSubstance& properties);
void setThermoReferenceProperties(v2::ReactionRecord& record, const ThermoPropertiesReaction& properties);

/// The arrays of `waiting` that `parameters` does not have: the old API can set them before the method that reads them.
void addWaiting(ThermoParametersSubstance& parameters, const ThermoParametersSubstance& waiting);
void addWaiting(ThermoParametersReaction& parameters, const ThermoParametersReaction& waiting);

} // namespace ThermoFun::compat

#endif
