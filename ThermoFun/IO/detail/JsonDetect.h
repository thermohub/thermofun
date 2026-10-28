// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Format detection for code that has parsed the JSON already. This header is not installed.
#ifndef THERMOFUN_IO_DETAIL_JSONDETECT_H
#define THERMOFUN_IO_DETAIL_JSONDETECT_H

#include <nlohmann/json.hpp>

#include <ThermoFun/Model/DataFormat.h>

namespace ThermoFun::v2 {
namespace detail {

/// See detectDocumentFormat(const std::string&).
auto detectDocumentFormat(const nlohmann::json& j) -> DataFormat;

/// See detectRecordFormat(const std::string&). A record that is not an object gives Auto.
auto detectRecordFormat(const nlohmann::json& j) -> DataFormat;

}
}

#endif
