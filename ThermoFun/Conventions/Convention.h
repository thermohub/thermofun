// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_CONVENTIONS_MERGE_H
#define THERMOFUN_CONVENTIONS_MERGE_H

#include <ThermoFun/Model/Convention.h>

namespace ThermoFun::model {

/// The convention that the engine uses inside: 1bar, molality, benson_helgeson. Water has no value here: inside, water has the
/// anchor its dataset or record declares (helgeson_kirkham_1974 by default), and the engine never converts between anchors.
inline auto canonicalConvention() -> Convention
{
    return {{1e5, ConcentrationScale::Molality}, {EnergyReference::BensonHelgeson, std::nullopt, std::nullopt}};
}

/// The fields of `specific` that have a value, completed with those of `general`.
inline auto mergeConvention(const Convention& specific, const Convention& general) -> Convention
{
    Convention c = specific;
    if (!c.standard_state.pressure) c.standard_state.pressure = general.standard_state.pressure;
    if (!c.standard_state.concentration_scale) c.standard_state.concentration_scale = general.standard_state.concentration_scale;
    if (!c.energy_reference.elements) c.energy_reference.elements = general.energy_reference.elements;
    if (!c.energy_reference.water) c.energy_reference.water = general.energy_reference.water;
    if (!c.energy_reference.ions) c.energy_reference.ions = general.energy_reference.ions;
    return c;
}

} // namespace ThermoFun::model

#endif
