// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_DATAFORMAT_H
#define THERMOFUN_MODEL_DATAFORMAT_H

#include <string>

namespace ThermoFun::model {

/// The format of a data file or of a record. Auto: the reader finds it.
enum class DataFormat
{
    Auto,
    Legacy,
    V2
};

/// The file a record was read from: its format and the text of the record, to write it back as it was. Not data:
/// equality ignores it, so records with the same data are equal whatever file they came from (review of v2, Q7).
struct FileOrigin
{
    DataFormat format = DataFormat::Legacy;
    std::string text;

    auto operator==(const FileOrigin&) const -> bool { return true; }
};

} // namespace ThermoFun::model

#endif
