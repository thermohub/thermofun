// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

// FROZEN COPY of ThermoFun/Common/ParseJsonToData.h. Do not edit the parser.
// The library objects hold only records now, so the frozen parser fills the plain objects below: the fields and the
// setters that Substance, Reaction and Element had at 2e00e56. Only this keeps the reference independent of the library.
#pragma once

#include <map>
#include <regex>
#include <string>
#include <vector>

#include "GlobalVariables.h"
#include "ThermoParameters.h"
#include "ThermoProperties.h"

namespace ThermoFun {
struct DatabaseConventions;
}

namespace frozen_v073 {

#define FROZEN_FIELD(TYPE, GET, SET, FIELD, INIT)        \
private:                                                 \
    TYPE FIELD INIT;                                     \
public:                                                  \
    auto GET() const -> TYPE { return FIELD; }           \
    auto SET(TYPE value) -> void { FIELD = value; }

class Substance
{
    FROZEN_FIELD(std::string, name, setName, name_, )
    FROZEN_FIELD(std::string, symbol, setSymbol, symbol_, )
    FROZEN_FIELD(std::string, formula, setFormula, formula_, )
    FROZEN_FIELD(std::string, reactionSymbol, setReactionSymbol, reaction_, )
    FROZEN_FIELD(double, molarMass, setMolarMass, molar_mass_, = 0.0)
    FROZEN_FIELD(double, referenceT, setReferenceT, reference_T_, = 298.15)
    FROZEN_FIELD(double, referenceP, setReferenceP, reference_P_, = 1e5)
    FROZEN_FIELD(double, lowerT, setLowerT, lower_T_, = 0.0)
    FROZEN_FIELD(double, lowerP, setLowerP, lower_P_, = 0.0)
    FROZEN_FIELD(double, upperT, setUpperT, upper_T_, = 0.0)
    FROZEN_FIELD(double, upperP, setUpperP, upper_P_, = 0.0)
    FROZEN_FIELD(ThermoFun::MethodGenEoS_Thrift::type, methodGenEOS, setMethodGenEoS, general_, = static_cast<ThermoFun::MethodGenEoS_Thrift::type>(0))
    FROZEN_FIELD(ThermoFun::MethodCorrT_Thrift::type, method_T, setMethod_T, temperature_, = static_cast<ThermoFun::MethodCorrT_Thrift::type>(0))
    FROZEN_FIELD(ThermoFun::MethodCorrP_Thrift::type, method_P, setMethod_P, pressure_, = static_cast<ThermoFun::MethodCorrP_Thrift::type>(0))
    FROZEN_FIELD(ThermoFun::SubstanceClass::type, substanceClass, setSubstanceClass, class_, = static_cast<ThermoFun::SubstanceClass::type>(0))
    FROZEN_FIELD(ThermoFun::AggregateState::type, aggregateState, setAggregateState, state_, = static_cast<ThermoFun::AggregateState::type>(0))
    FROZEN_FIELD(ThermoFun::SubstanceThermoCalculationType::type, thermoCalculationType, setThermoCalculationType, type_, = static_cast<ThermoFun::SubstanceThermoCalculationType::type>(0))
    FROZEN_FIELD(int, charge, setCharge, charge_, = 0)
    FROZEN_FIELD(ThermoFun::ThermoPropertiesSubstance, thermoProperties, setThermoProperties, properties_, )
    FROZEN_FIELD(ThermoFun::ThermoPropertiesSubstance, thermoReferenceProperties, setThermoReferenceProperties, reference_, )
    FROZEN_FIELD(ThermoFun::ThermoParametersSubstance, thermoParameters, setThermoParameters, parameters_, )
    FROZEN_FIELD(std::string, jsonString, setJsonString, json_, )
};

using ReactantsMap_ = std::map<std::string, double>;

class Reaction
{
    FROZEN_FIELD(std::string, name, setName, name_, )
    FROZEN_FIELD(std::string, symbol, setSymbol, symbol_, )
    FROZEN_FIELD(std::string, equation, setEquation, equation_, )
    FROZEN_FIELD(ReactantsMap_, reactants, setReactants, reactants_, )
    FROZEN_FIELD(double, referenceT, setReferenceT, reference_T_, = 298.15)
    FROZEN_FIELD(double, referenceP, setReferenceP, reference_P_, = 1e5)
    FROZEN_FIELD(double, lowerT, setLowerT, lower_T_, = 0.0)
    FROZEN_FIELD(double, lowerP, setLowerP, lower_P_, = 0.0)
    FROZEN_FIELD(double, upperT, setUpperT, upper_T_, = 0.0)
    FROZEN_FIELD(double, upperP, setUpperP, upper_P_, = 0.0)
    FROZEN_FIELD(ThermoFun::MethodGenEoS_Thrift::type, methodGenEOS, setMethodGenEoS, general_, = static_cast<ThermoFun::MethodGenEoS_Thrift::type>(0))
    FROZEN_FIELD(ThermoFun::MethodCorrT_Thrift::type, method_T, setMethod_T, temperature_, = static_cast<ThermoFun::MethodCorrT_Thrift::type>(0))
    FROZEN_FIELD(ThermoFun::MethodCorrP_Thrift::type, method_P, setMethod_P, pressure_, = static_cast<ThermoFun::MethodCorrP_Thrift::type>(0))
    FROZEN_FIELD(ThermoFun::ThermoPropertiesReaction, thermoReferenceProperties, setThermoReferenceProperties, reference_, )
    FROZEN_FIELD(ThermoFun::ThermoParametersReaction, thermoParameters, setThermoParameters, parameters_, )
    FROZEN_FIELD(std::string, jsonString, setJsonString, json_, )
public:
    auto thermo_ref_prop() const -> ThermoFun::ThermoPropertiesReaction { return reference_; }
    auto fromEquation(const std::string& reactionEquation) -> void;
};

class Element
{
    FROZEN_FIELD(std::string, name, setName, name_, )
    FROZEN_FIELD(std::string, symbol, setSymbol, symbol_, )
    FROZEN_FIELD(double, molarMass, setMolarMass, molar_mass_, = 0.0)
    FROZEN_FIELD(double, entropy, setEntropy, entropy_, = 0.0)
    FROZEN_FIELD(double, heatCapacity, setHeatCapacity, heat_capacity_, = 0.0)
    FROZEN_FIELD(double, volume, setVolume, volume_, = 0.0)
    FROZEN_FIELD(int, valence, setValence, valence_, = 777)
    FROZEN_FIELD(int, class_, setClass, class__, = 0)
    FROZEN_FIELD(int, isotopeMass, setIsotopeMass, isotope_, = 0)
    FROZEN_FIELD(int, number, setNumber, number_, = 0)
    FROZEN_FIELD(std::string, jsonString, setJsonString, json_, )
};

#undef FROZEN_FIELD

auto parseElement(const std::string& data) -> Element;
auto parseSubstance(const std::string& data) -> Substance;
auto parseReaction(const std::string& data) -> Reaction;

}
