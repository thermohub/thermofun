// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The reader of the v2 format: the text of a file or of one record into records.

#ifndef THERMOFUN_IO_V2_V2READER_H
#define THERMOFUN_IO_V2_V2READER_H

#include <string>
#include <vector>

#include <ThermoFun/Common/Diagnostics.h>
#include <ThermoFun/Model/Records.h>

namespace ThermoFun::v2 {

/// The records of one v2 file
struct V2Document
{
    ThermoDataSetInfo info;
    bool has_dataset = false;   ///< the file has a thermodataset block
    std::vector<ElementRecord> elements;
    std::vector<SubstanceRecord> substances;
    std::vector<ReactionRecord> reactions;
    std::vector<MethodDefinition> methods;           ///< records of the project scope
    std::vector<MethodParameterSet> parameter_sets;
    std::vector<DataSource> data_sources;
};

/// Reads a document. A problem is added to the diagnostics; a record with an error is not in the result.
/// The default units of the file and the reference conditions of the dataset are applied.
auto readV2Document(const std::string& text, Diagnostics& diagnostics, const std::string& file = "") -> V2Document;

/// Reads one record. The result has an empty symbol if the record has an error.
auto readV2Element(const std::string& text, Diagnostics& diagnostics) -> ElementRecord;
auto readV2Substance(const std::string& text, Diagnostics& diagnostics) -> SubstanceRecord;
auto readV2Reaction(const std::string& text, Diagnostics& diagnostics) -> ReactionRecord;

/// Checks a method record of a file against the built-in method of the same id (V2-MTH-010 and V2-MTH-011).
/// A method that ThermoFun does not know is not checked. The problems are added to the diagnostics.
auto checkMethodDefinition(const MethodDefinition& method, Diagnostics& diagnostics) -> void;

/// Checks the references between the records of a document: the defining reaction of a substance, the
/// substances of a reaction, the data sources. Only the problems are added to the diagnostics.
auto resolveV2Document(const V2Document& document, Diagnostics& diagnostics) -> void;

}

#endif
