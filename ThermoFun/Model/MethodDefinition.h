// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_METHODDEFINITION_H
#define THERMOFUN_MODEL_METHODDEFINITION_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <ThermoFun/Model/Applicability.h>

namespace ThermoFun {

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

    auto operator==(const ReferenceTestCase&) const -> bool = default;
};

/// A Method record of a project. ThermoFun reads it and checks it against its built-in specification.
struct MethodDefinition
{
    std::string symbol;                          ///< id of the method, for example "HKFReaktoro"
    std::string kind = "reference";              ///< "symbolic", "reference" or "composite"
    int tier = 2;
    std::string operates_on = "substance";       ///< "substance" or "reaction"
    std::vector<std::string> applies_to_classes;
    std::vector<std::string> contributes_to;     ///< "G", "H", "S", "Cp", "V"
    std::vector<ParameterDefinition> parameter_definitions;
    std::string implementation_id;               ///< for reference methods
    // Symbolic methods of tier 1
    std::string source_form;                     ///< "cp", "cp_increment", "volume", "gibbs", "logk", "drcp", "drv", ...
    std::string source_expression;               ///< the model as the author wrote it (one piece)
    std::vector<Segment> source_segments;        ///< pieces of a method that is piecewise
    std::map<std::string, std::string> expression;                         ///< generated expressions
    std::vector<std::map<std::string, std::string>> expression_segments;   ///< generated, one for each piece
    std::string generator;                       ///< tool and version that made `expression`
    std::string source_hash;                     ///< sha256 of the source block when `expression` was made
    std::vector<ReferenceTestCase> tests;
    std::string effects_json;                    ///< the block `effects` of the file as JSON text
    std::string citation;                        ///< symbol of a data source
    std::string extra_json;                      ///< unknown fields, as JSON text

    auto operator==(const MethodDefinition&) const -> bool = default;
};

}

#endif
