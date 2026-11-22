// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_CONVENTIONS_CONVENTION_H
#define THERMOFUN_CONVENTIONS_CONVENTION_H

#include <optional>

#include <ThermoFun/Model/Vocabulary.h>

namespace ThermoFun::model {

/// The convention of a set of values. A field without a value is inherited from the record, the dataset or the default.
/// Merging and conversion are in the layer Conventions/.
/// The standard state of the values (decision 0031, row 6): the standard pressure p° and, for solutes, the concentration
/// scale. A field without a value is the default (1 bar, molality), or the value of a more general level.
struct StandardState
{
    std::optional<StandardPressure> pressure;
    std::optional<ConcentrationScale> concentration_scale;

    auto operator==(const StandardState&) const -> bool = default;
};

/// From what zero G, H and S are counted: of the elements, and of water. A field without a value is the default
/// (benson_helgeson, geochemical), or the value of a more general level.
struct EnergyReferences
{
    std::optional<EnergyReference> elements;
    std::optional<WaterReference> water;

    auto operator==(const EnergyReferences&) const -> bool = default;
};

/// The conventions of values: the blocks `standard_state` and `energy_reference` of a method, a record or a dataset. A value
/// is defined by its standard state (class, aggregate state and this standard state), its reference temperature and its
/// energy reference.
struct Convention
{
    StandardState standard_state;
    EnergyReferences energy_reference;

    auto operator==(const Convention&) const -> bool = default;
};

} // namespace ThermoFun::model

#endif
