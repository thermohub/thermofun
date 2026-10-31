// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The keys of the v2 format. The reader and the writer use these names and no string literal for a key.

#ifndef THERMOFUN_IO_V2_V2KEYS_H
#define THERMOFUN_IO_V2_V2KEYS_H

namespace ThermoFun::v2 {
namespace v2key {

// a quantity
inline constexpr const char* value = "value";
inline constexpr const char* unit = "unit";
inline constexpr const char* uncertainty = "uncertainty";
inline constexpr const char* quality = "quality";
inline constexpr const char* origin = "origin";
inline constexpr const char* calculation_path = "calculation_path";
inline constexpr const char* source = "source";

// an uncertainty
inline constexpr const char* standard = "standard";
inline constexpr const char* expanded = "expanded";
inline constexpr const char* coverage_factor = "coverage_factor";
inline constexpr const char* distribution = "distribution";
inline constexpr const char* degrees_of_freedom = "degrees_of_freedom";
inline constexpr const char* kind = "kind";

// scoped default units
inline constexpr const char* units = "units";
inline constexpr const char* energy = "energy";
inline constexpr const char* pressure = "pressure";
inline constexpr const char* temperature = "temperature";
inline constexpr const char* length = "length";
inline constexpr const char* mass = "mass";
inline constexpr const char* quantity = "quantity";

}
}

#endif
