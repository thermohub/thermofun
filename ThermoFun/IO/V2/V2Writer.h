// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_IO_V2_V2WRITER_H
#define THERMOFUN_IO_V2_V2WRITER_H

#include <string>
#include <vector>

#include <ThermoFun/Model/Records.h>

namespace ThermoFun::v2 {

struct V2WriterOptions
{
    /// True: a value from a single-precision source is written as the shortest decimal of that float, for example -56.484.
    /// Such a file reads back up to about 6e-8 relative from the value in memory (decision 0023).
    /// False: every value reads back as the same double, for example -56.484001159668.
    bool readable = true;
};

/// All records of a document.
struct V2Records
{
    ThermoDataSetInfo dataset;
    std::vector<ElementRecord> elements;
    std::vector<SubstanceRecord> substances;
    std::vector<ReactionRecord> reactions;
    std::vector<MethodDefinition> methods;
    std::vector<MethodParameterSet> parameter_sets;
    std::vector<DataSource> data_sources;
};

/// A document as JSON text. The keys are in the order of the format; the records keep the order of the vectors.
auto writeV2Json(const V2Records& records, const V2WriterOptions& options = {}) -> std::string;

/// The same document as YAML text, in block style, with short mappings and all lists of numbers on one line.
auto writeV2Yaml(const V2Records& records, const V2WriterOptions& options = {}) -> std::string;

} // namespace ThermoFun::v2

#endif
