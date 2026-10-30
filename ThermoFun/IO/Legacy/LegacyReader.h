// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_IO_LEGACY_LEGACYREADER_H
#define THERMOFUN_IO_LEGACY_LEGACYREADER_H

#include <string>

#include <ThermoFun/Common/Diagnostics.h>
#include <ThermoFun/Model/Records.h>

namespace ThermoFun::v2 {

struct LegacyReadOptions
{
    /// A legacy "error" is read as an expanded uncertainty with this coverage factor. 1: the error is the standard uncertainty.
    double error_coverage_factor = 1.0;
};

// The functions read one record of the legacy format, given as JSON text. A record that cannot be read gives
// an error diagnostic and a record with the fields that could be read, as the legacy parser does.
// What has no place in a record (keys that ThermoFun does not read, values that no method uses, methods that do
// nothing) is not stored: the diagnostics report it, as the migration report.
// All values of a record are in SI units. A volume is in m3/mol.
// A lower or an upper temperature limit that the file does not give is minus or plus infinity in a Segment.

auto readLegacyElement(const std::string& text, const LegacyReadOptions& options = {}, Diagnostics* diagnostics = nullptr)
    -> ElementRecord;
auto readLegacySubstance(const std::string& text, const LegacyReadOptions& options = {}, Diagnostics* diagnostics = nullptr)
    -> SubstanceRecord;
auto readLegacyReaction(const std::string& text, const LegacyReadOptions& options = {}, Diagnostics* diagnostics = nullptr)
    -> ReactionRecord;

/// The data about the dataset in the top-level keys of a legacy document.
auto readLegacyDocumentInfo(const std::string& text) -> ThermoDataSetInfo;

} // namespace ThermoFun::v2

#endif
