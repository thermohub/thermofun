// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_METHODDEFINITION_H
#define THERMOFUN_MODEL_METHODDEFINITION_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <ThermoFun/Model/Applicability.h>
#include <ThermoFun/Model/Vocabulary.h>

namespace ThermoFun::model {

/// Declares one parameter of a method.
struct ParameterDefinition
{
    std::string name;                     ///< for example "a1"
    std::string unit;                     ///< SI unit in UCUM, for example "J/(mol.Pa)"
    std::string description;
    bool required = true;
    std::optional<double> default_value;  ///< SI

    auto operator==(const ParameterDefinition&) const -> bool = default;
};

/// A reference test of a method: inputs and the expected output.
struct ReferenceTestCase
{
    double T = 298.15, P = 1e5;                  ///< K, Pa
    std::map<std::string, double> parameters;    ///< SI
    std::map<std::string, double> expected;      ///< name of the property and its SI value
    double relative_tolerance = 1e-9;
    std::map<std::string, double> reference;     ///< Gr, Hr, Sr, Cpr, Vr, Tr, Pr
    int piece = 0;                               ///< of a method with pieces in T

    auto operator==(const ReferenceTestCase&) const -> bool = default;
};

/// A Method record of a project. ThermoFun reads it and checks it against its built-in specification.
struct MethodDefinition
{
    std::string symbol;                          ///< id of the method, for example "HKF88"
    MethodKind kind = MethodKind::Reference;
    OperatesOn operates_on = OperatesOn::Substance;
    std::vector<SubstanceClass> applies_to_classes;
    /// What the method contributes to (decision 0031, row 11), in the order of the file: a word of PropertyName or
    /// ReactionPropertyName -> sets (the whole value), adds (an increment) or uses (an input only). A property not listed is not
    /// touched. Written as a short list when contributes_to_list is true: each property then adds.
    std::vector<std::pair<std::string, std::string>> contributes_to;
    bool contributes_to_list = false;
    std::vector<ParameterDefinition> parameter_definitions;
    std::string implementation_id;               ///< for reference methods
    // Symbolic methods: the formula as the author wrote it, and the expressions generated from it
    std::string formula_form;                     ///< "cp", "cp_increment", "volume", "gibbs", "logk", "drcp", "drv", ...
    std::string formula_expression;               ///< the formula as the author wrote it (one piece)
    std::vector<Segment> formula_segments;        ///< pieces of a method that is piecewise
    std::map<std::string, std::string> expression;                         ///< generated expressions
    std::vector<std::map<std::string, std::string>> expression_segments;   ///< generated, one for each piece
    std::string generator;                       ///< tool and version that made `expression`
    std::string formula_json;                     ///< the formula block as compact JSON text with sorted keys, for the hash
    std::string formula_hash;                     ///< sha256 of the formula block when `expression` was made
    std::vector<ReferenceTestCase> tests;
    std::vector<std::string> data_sources;       ///< NEA reference codes (TDB-5 1.11), or free text
    std::string extra_json;                      ///< unknown fields, as JSON text

    auto operator==(const MethodDefinition&) const -> bool = default;
};

} // namespace ThermoFun::model

#endif
