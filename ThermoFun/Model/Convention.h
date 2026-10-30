// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_CONVENTIONS_CONVENTION_H
#define THERMOFUN_CONVENTIONS_CONVENTION_H

#include <optional>

#include <ThermoFun/Model/Vocabulary.h>

namespace ThermoFun::v2 {

/// The convention of a set of values. A field without a value is inherited from the record, the dataset or the default.
/// Merging and conversion are in the layer Conventions/.
struct Convention
{
    std::optional<EnergyReference> energy_reference;
    std::optional<StandardPressure> standard_pressure;
    std::optional<ConcentrationScale> concentration_scale;
    std::optional<WaterReference> water_reference;

    auto operator==(const Convention&) const -> bool = default;
};

} // namespace ThermoFun::v2

#endif
