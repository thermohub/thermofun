// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_CONVENTIONS_MERGE_H
#define THERMOFUN_CONVENTIONS_MERGE_H

#include <ThermoFun/Model/Convention.h>

namespace ThermoFun::v2 {

/// The convention that the engine uses inside: benson_helgeson, 1bar, molality, geochemical.
inline auto canonicalConvention() -> Convention
{
    return {EnergyReference::BensonHelgeson, StandardPressure::OneBar, ConcentrationScale::Molality, WaterReference::Geochemical};
}

/// The fields of `specific` that have a value, completed with those of `general`.
inline auto mergeConvention(const Convention& specific, const Convention& general) -> Convention
{
    Convention c = specific;
    if (!c.energy_reference) c.energy_reference = general.energy_reference;
    if (!c.standard_pressure) c.standard_pressure = general.standard_pressure;
    if (!c.concentration_scale) c.concentration_scale = general.concentration_scale;
    if (!c.water_reference) c.water_reference = general.water_reference;
    return c;
}

} // namespace ThermoFun::v2

#endif
