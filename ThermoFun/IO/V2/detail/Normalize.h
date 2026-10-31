// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Reading and writing a quantity of the v2 format: a number, a string "number unit" or an object.
// The value in the record is in the SI unit of its slot.

#ifndef THERMOFUN_IO_V2_NORMALIZE_H
#define THERMOFUN_IO_V2_NORMALIZE_H

#include <map>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include <ThermoFun/Common/Diagnostics.h>
#include <ThermoFun/Model/Quantity.h>

namespace ThermoFun::v2 {

/// The default units of a part of a file. The nearest scope that names a dimension decides.
/// The dimensions are energy, pressure, temperature, length, mass and quantity.
struct UnitScope
{
    std::map<std::string, std::string> defaults;
    const UnitScope* parent = nullptr;
};

/// Reads a `units:` mapping. A problem is added to the diagnostics.
auto readUnitScope(const nlohmann::json& node, const UnitScope* parent, Diagnostics& diag, const std::string& path) -> UnitScope;

/// The unit of a slot with the default units of the scope: "J/(mol.K)" with {energy: cal} gives "cal/(mol.K)".
auto scopedUnit(const std::string& si_unit, const UnitScope& scope) -> std::string;

/// Reads a quantity. The result is empty, with an error in the diagnostics, if the node cannot be read.
auto readQuantity(const nlohmann::json& node, const std::string& slot_si_unit, const UnitScope& scope, Diagnostics& diag,
                  const std::string& path) -> std::optional<Quantity>;

/// The shortest form that reads back the same: a bare number, or an object with the keys that are set.
auto writeQuantity(const Quantity& q, const std::string& slot_si_unit) -> nlohmann::json;

}

#endif
