// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_REACTIONRECORD_H
#define THERMOFUN_MODEL_REACTIONRECORD_H

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <ThermoFun/Model/Convention.h>
#include <ThermoFun/Model/Covariance.h>
#include <ThermoFun/Model/Applicability.h>
#include <ThermoFun/Model/DataFormat.h>
#include <ThermoFun/Model/MethodApplication.h>
#include <ThermoFun/Model/Quantity.h>
#include <ThermoFun/Model/ReferenceConditions.h>
#include <ThermoFun/Model/Vocabulary.h>

namespace ThermoFun::v2 {

/// A reaction of the database, as the file says. All values are in SI units.
struct ReactionRecord
{
    std::string symbol;                  ///< a legacy reaction has the symbol of the substance it defines
    std::string sourcetdb;
    int level = 0;
    std::string name;
    std::string equation;                ///< as written, for example "1 Na+ + 1 HCO3- = NaHCO3@"
    /// Symbols and coefficients in the order of the file: a coefficient below 0 is a reactant, above 0 a product
    std::vector<std::pair<std::string, double>> stoichiometry;
    ReferenceConditions reference_conditions;
    std::map<ReactionPropertyName, Quantity> reaction_properties;   ///< logK, dG, dH, dS, dCp, dV
    std::vector<MethodApplication> methods;
    Applicability applicability;         ///< the range of the record; the legacy key limitsTP of the record
    std::optional<Convention> convention;
    std::optional<PropertySource> preferred_property_source;   ///< Direct or Reactants; empty: the default rule
    std::vector<CovarianceBlock> covariance; ///< correlations between values and parameters
    std::string data_source;
    std::vector<std::string> data_sources;
    bool deprecated = false;
    std::string extra_json;              ///< fields that the format does not define, as JSON text
    DataFormat source_format = DataFormat::Legacy;
    std::string original_text;

    auto operator==(const ReactionRecord&) const -> bool = default;
};

} // namespace ThermoFun::v2

#endif
