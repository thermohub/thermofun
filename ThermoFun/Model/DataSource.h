// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_DATASOURCE_H
#define THERMOFUN_MODEL_DATASOURCE_H

#include <optional>
#include <string>

namespace ThermoFun {

/// A citation. The symbol follows the pattern YYYYAAA/BBB when possible.
struct DataSource
{
    std::string symbol;  ///< for example "2017MIR/WAG"
    std::string title, authors, journal, doi, bibtex;
    std::optional<int> year;
    std::string extra_json;

    auto operator==(const DataSource&) const -> bool = default;
};

}

#endif
