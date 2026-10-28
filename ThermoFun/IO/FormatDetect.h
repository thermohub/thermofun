// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_IO_FORMATDETECT_H
#define THERMOFUN_IO_FORMATDETECT_H

#include <string>

#include <ThermoFun/Model/DataFormat.h>

namespace ThermoFun::v2 {

/// The format of a document, given as JSON text.
/// V2 if the top level is an object with the key "thermohub". Legacy for every other document.
/// Auto if the text is not JSON: the caller must decide (it may be YAML).
auto detectDocumentFormat(const std::string& jsonText) -> DataFormat;

/// The format of one record, given as JSON text.
/// V2 if the record has one of the keys of the v2 format. Legacy if it has one of the keys of the legacy format,
/// and also if it has none of them, as ThermoFun reads such a record today. Auto if the text is not a JSON object.
auto detectRecordFormat(const std::string& jsonText) -> DataFormat;

}

#endif
