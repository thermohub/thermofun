// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_IO_REFERENCECODE_H
#define THERMOFUN_IO_REFERENCECODE_H

#include <regex>
#include <string>

namespace ThermoFun::model {

/// True for an NEA reference code (TDB-5 1.11): the year (4 digits, or 2 in older volumes), the first three letters of the
/// first author and of the second one after "/", and a number for a second, third, ... reference with the same code:
/// 1995ROB/HEM, 1995ROB/HEM2, 1865KOP, 61ROS/ROS. A data_sources entry that is not one is free text.
inline auto isReferenceCode(const std::string& text) -> bool
{
    static const std::regex code("^([0-9]{4}|[0-9]{2})[A-Z]{3}(/[A-Z]{3})?[0-9]*$");
    return std::regex_match(text, code);
}

} // namespace ThermoFun::model

#endif
