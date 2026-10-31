// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_SNAPSHOT_H
#define THERMOFUN_ENGINE_SNAPSHOT_H

#include <map>
#include <string>

namespace ThermoFun {

/// What a calculation depends on, to repeat it or to compare two results.
struct CalculationSnapshot
{
    std::string thermofun_version;
    std::string format_version;     ///< the format of the loaded data: "legacy", or the version of the v2 file
    std::string dataset;            ///< symbol of the dataset
    std::string dataset_version;
    std::string content_hash;       ///< SHA-256 of the v2 text of all records, with exact numbers
    std::map<std::string, std::string> implementations;   ///< method id -> implementation that the preferences ask for
};

} // namespace ThermoFun

#endif
