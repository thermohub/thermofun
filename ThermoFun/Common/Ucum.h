// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_UCUM_H
#define THERMOFUN_UCUM_H

#include <array>
#include <string>

namespace ThermoFun {
namespace units {

/// Translate a UCUM unit string to the syntax of the unit converter (Units.hpp).
/// A string that already has the converter syntax (it has "*" or "^") is returned as it is.
/// Throws std::invalid_argument and names the token if the unit is not supported.
auto fromUcum(const std::string& ucum) -> std::string;

/// Convert a value from one unit to another. Both units are in UCUM or in converter syntax.
/// Throws std::invalid_argument if the units have different dimensions or one of them is not supported.
auto convertAny(double value, const std::string& from, const std::string& to) -> double;

/// Convert a difference, for example an uncertainty or a temperature interval: scale only, no offset.
auto convertDifference(double delta, const std::string& from, const std::string& to) -> double;

/// The dimension of a unit: exponents of kg, m, s, K, mol, A, cd.
/// An exponent may be a fraction (K^0.5 occurs in legacy data).
auto dimension(const std::string& unit) -> std::array<double, 7>;

}
}

#endif
