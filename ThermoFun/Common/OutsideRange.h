// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_COMMON_OUTSIDERANGE_H
#define THERMOFUN_COMMON_OUTSIDERANGE_H

#include <string>

#include <ThermoFun/Common/Notes.h>
#include <ThermoFun/Common/ThermoScalar.hpp>

namespace ThermoFun {

/// The words that start the message of a value that a model calculated outside its range (of T, P, the density of water,
/// its T intervals or table). The status of such a value stays `calculated`: the value is there, and the message says why it
/// can be wrong. All methods write these messages with one function, so outsideRange() finds them all.
inline constexpr const char* outsideRangeMark = rangeNoteMark;

/// The message of a point outside the range of a model, as the methods write it.
inline auto outsideRangeMessage(const std::string& what) -> std::string
{
    return outsideRangeMark + what;
}

/// True if the value was calculated outside the range of its model. Its status message (sta.second) says which range.
inline auto outsideRange(const Reaktoro_::ThermoScalar& x) -> bool
{
    return x.sta.second.find(outsideRangeMark) != std::string::npos;
}

} // namespace ThermoFun

#endif
