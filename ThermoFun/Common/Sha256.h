// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_COMMON_SHA256_H
#define THERMOFUN_COMMON_SHA256_H

#include <string>

namespace ThermoFun {

/// The SHA-256 of a text, as 64 lower-case hexadecimal characters
auto sha256(const std::string& text) -> std::string;

}

#endif
