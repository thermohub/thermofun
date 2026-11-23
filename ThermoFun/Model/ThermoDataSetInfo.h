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

namespace ThermoFun::model {

/// Data about the dataset that is loaded, and its conventions.
struct ThermoDataSetInfo
{
    std::string symbol;                 ///< for example "aq17", "cemdata18"
    std::string name;
    std::string version;                ///< required if the status is Published
    DatasetStatus status = DatasetStatus::Draft;
    std::string schema_version;         ///< version of the v2 format of the file, empty for a legacy file
    Convention convention;              ///< convention of the dataset; the records may declare their own
    ReferenceConditions reference_conditions;
    std::vector<std::string> sourcetdb;
    std::string solvent;                ///< symbol of the aqueous solvent, for example "H2O@"; empty: search for it
    ValueMeta defaults;                 ///< data of values that a value does not give itself
    std::vector<std::string> data_sources;      ///< NEA reference codes (TDB-5 1.11) of the sources, or free text; the first is the source of a value that gives none
    std::string physical_constants;             ///< the set of constants of physics the values were derived with (codata1986, codata2018); empty: not said
    std::vector<std::string> legacy_datasources;                 ///< the key "datasources" of a legacy file
    std::string legacy_date;                                     ///< the key "date" of a legacy file
    std::string extra_json;

    auto operator==(const ThermoDataSetInfo&) const -> bool = default;
};

} // namespace ThermoFun::model

#endif
