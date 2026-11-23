// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_SUMMARY_H
#define THERMOFUN_ENGINE_SUMMARY_H

#include <map>
#include <string>
#include <vector>

namespace ThermoFun {

/// What the results of an engine depend on, to repeat a calculation or to compare two results (engine.summary()).
struct EngineSummary
{
    std::string thermofun_version;
    std::string format_version;     ///< the format of the loaded data: "legacy", or the version of the v2 file
    std::string dataset;            ///< symbol of the dataset
    std::string dataset_version;
    std::string content_hash;       ///< SHA-256 of the v2 text of all records, with exact numbers
    std::map<std::string, std::string> implementations;   ///< method id -> implementation that the preferences ask for
    std::string physical_constants;  ///< the set of the constants of physics of the engine (option physical_constants)
    std::string water_symbol;        ///< the record of the solvent, empty if the database has none
    std::vector<std::string> water_methods;   ///< the methods of that record
    std::map<std::string, std::string> options;       ///< the options of the engine that are not at their default
    std::map<std::string, std::string> conventions;   ///< the conventions of the engine

    /// The summary as short lines of text, one after the other: what goes at the top of an output
    auto text() const -> std::string;
};

} // namespace ThermoFun

#endif
