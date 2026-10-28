// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_CONVENTIONS_CONVENTION_H
#define THERMOFUN_CONVENTIONS_CONVENTION_H

#include <optional>
#include <string>

namespace ThermoFun {

/// The convention of a set of values. A field without a value is inherited from the record, the dataset or the default.
/// The words are in docs/redesign/14-conventions.md. Reading, merging and conversion come with the conventions layer.
struct Convention
{
    std::optional<std::string> energy_reference;     ///< "benson_helgeson", "berman_brown", "ser", ...
    std::optional<std::string> standard_pressure;    ///< "1bar" or "1atm"
    std::optional<std::string> concentration_scale;  ///< "molality", "mole_fraction" or "molarity"
    std::optional<std::string> water_reference;      ///< "geochemical" or "steam_tables"

    auto operator==(const Convention&) const -> bool = default;
};

}

#endif
