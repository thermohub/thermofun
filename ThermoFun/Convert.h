// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The conversion of a database between the legacy format and the v2 format, and its report (python -m thermofun.convert).

#ifndef THERMOFUN_CONVERT_H
#define THERMOFUN_CONVERT_H

#include <cstddef>
#include <optional>
#include <string>

#include <ThermoFun/Database.h>

namespace ThermoFun {

/// What a conversion writes.
struct ConvertOptions
{
    /// V2 or Legacy. The legacy format is one JSON file.
    model::DataFormat to = model::DataFormat::V2;
    /// true: YAML (v2 only)
    bool yaml = false;
    /// true: a dataset folder with one file for each record (v2 only), as Database::saveDataset
    bool split = false;
    /// true: short decimals, the values change a little (see Database::toJson); false: each number reads back the same
    bool readable = false;
    /// a file for the migration report (Markdown), empty for none
    std::string report;
};

/// How many numbers the readable form writes otherwise than the exact one, and the largest relative change.
struct ShortenedNumbers
{
    std::size_t count = 0;
    double largest = 0.0;
};

/// The numbers that Database::toJson(true) writes otherwise than toJson(false). Empty if the two do not have the same
/// numbers in the same places.
auto shortenedNumbers(const Database& database) -> std::optional<ShortenedNumbers>;

/// The migration report as Markdown: the counts, the quirks applied, the unread legacy keys, the unresolved references, the
/// other problems, the methods used and the notice on uncertainties.
/// @param input the file or folder that the database was read from (for the legacy diagnostics)
auto conversionReport(const Database& database, const std::string& input, const ConvertOptions& options = {}) -> std::string;

/// Reads input (a legacy or v2 file or folder), writes it to output as the options say, and the report if asked.
/// Returns the diagnostics of the load. Throws std::invalid_argument for the legacy format with yaml or split.
auto convertFile(const std::string& input, const std::string& output, const ConvertOptions& options = {}) -> Diagnostics;

} // namespace ThermoFun

#endif
