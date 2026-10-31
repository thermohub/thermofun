// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_IO_LEGACY_LEGACYWRITER_H
#define THERMOFUN_IO_LEGACY_LEGACYWRITER_H

#include <string>

#include <ThermoFun/Common/Diagnostics.h>
#include <ThermoFun/IO/V2/V2Writer.h>

namespace ThermoFun::v2 {

struct LegacyWriteOptions
{
    /// A record from a legacy file that nobody edited is written as its original text.
    bool use_original_text = true;
};

/// One record as legacy JSON text. What the legacy format cannot hold (a method with no legacy name, a table of log K, the data
/// about a value, covariance) is left out, and a warning LEG-WRT-001 names it.
auto writeLegacyElement(const ElementRecord& record, const LegacyWriteOptions& options = {}, Diagnostics* diagnostics = nullptr) -> std::string;
auto writeLegacySubstance(const SubstanceRecord& record, const LegacyWriteOptions& options = {}, Diagnostics* diagnostics = nullptr) -> std::string;
auto writeLegacyReaction(const ReactionRecord& record, const LegacyWriteOptions& options = {}, Diagnostics* diagnostics = nullptr) -> std::string;

/// A whole file: the dataset, `elements`, `substances` and `reactions`.
auto writeLegacyDocument(const V2Records& records, const LegacyWriteOptions& options = {}, Diagnostics* diagnostics = nullptr) -> std::string;

} // namespace ThermoFun::v2

#endif
