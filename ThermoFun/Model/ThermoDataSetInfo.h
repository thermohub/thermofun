// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_THERMODATASETINFO_H
#define THERMOFUN_MODEL_THERMODATASETINFO_H

#include <string>
#include <vector>

#include <ThermoFun/Model/Convention.h>
#include <ThermoFun/Model/Quantity.h>
#include <ThermoFun/Model/ReferenceConditions.h>
#include <ThermoFun/Model/Vocabulary.h>

namespace ThermoFun::v2 {

/// Data about the dataset that is loaded, and its conventions.
struct ThermoDataSetInfo
{
    std::string symbol;                 ///< for example "aq17", "cemdata18"
    std::string name;
    std::string version;                ///< required if the status is Published
    DatasetStatus status = DatasetStatus::Draft;
    std::string schema_version;         ///< version of the v2 format of the file, empty for a legacy file
    std::string standard_state_convention = "IUPAC_1bar";
    Convention convention;              ///< convention of the dataset; the records may declare their own
    ReferenceConditions reference_conditions;
    std::vector<std::string> sourcetdb;
    std::string solvent;                ///< symbol of the aqueous solvent, for example "H2O@"; empty: search for it
    ValueMeta defaults;                 ///< data of values that a value does not give itself
    std::string default_data_source;
    std::string water_triple_point_reference = "HKF_HGK_1974";   ///< as DatabaseConventions today
    std::vector<std::string> legacy_datasources;                 ///< the key "datasources" of a legacy file
    std::string legacy_date;                                     ///< the key "date" of a legacy file
    std::string extra_json;

    auto operator==(const ThermoDataSetInfo&) const -> bool = default;
};

} // namespace ThermoFun::v2

#endif
