// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_SUBSTANCERECORD_H
#define THERMOFUN_MODEL_SUBSTANCERECORD_H

#include <map>
#include <optional>
#include <string>
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

/// A substance of the database, as the file says. All values are in SI units.
struct SubstanceRecord
{
    // identity
    std::string symbol;                  ///< unique in a dataset
    SubstanceClass class_ = SubstanceClass::Other;
    std::string sourcetdb;

    // description
    std::string name;
    std::string formula;                 ///< a formula of ChemicalFun
    std::optional<int> charge;           ///< the legacy key formula_charge
    AggregateState aggregate_state = AggregateState::Other;
    std::optional<Quantity> molar_mass;  ///< kg/mol (the legacy key mass_per_mole is in g/mol)

    // standard state
    std::string standard_state_convention;   ///< empty: the convention of the dataset
    std::optional<Convention> convention;    ///< empty: inherited
    ReferenceConditions reference_conditions;
    std::map<PropertyName, Quantity> standard_state_properties;   ///< G, H, S, Cp, V

    // how the properties at T and P come
    std::vector<MethodApplication> methods;     ///< in the order of the file
    std::string defining_reaction;              ///< symbol of a reaction; the legacy key reaction
    Applicability applicability;                ///< the range of the record; the legacy key limitsTP of the record
    std::optional<PropertySource> preferred_property_source;   ///< Direct or Reaction; empty: the default rule
    std::map<std::string, Quantity> structural; ///< more data for methods that use it
    std::vector<CovarianceBlock> covariance;    ///< correlations between values and parameters

    // origin
    std::string data_source;                    ///< source of the record, used if a value gives none
    std::vector<std::string> data_sources;      ///< the legacy keys datasources and references, as written
    bool deprecated = false;
    std::string deprecated_in_favour_of;

    // to write the record back
    std::string extra_json;                     ///< fields that the format does not define, as JSON text
    DataFormat source_format = DataFormat::Legacy;
    std::string original_text;

    auto operator==(const SubstanceRecord&) const -> bool = default;
};

} // namespace ThermoFun::v2

#endif
