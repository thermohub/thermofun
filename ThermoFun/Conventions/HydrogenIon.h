// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_CONVENTIONS_HYDROGENION_H
#define THERMOFUN_CONVENTIONS_HYDROGENION_H

#include <algorithm>
#include <map>
#include <string>

#include <ThermoFun/Model/SubstanceRecord.h>

namespace ThermoFun::v2 {

/// True for the record of the hydrogen ion. The rule looks at the class and the formula, not at the symbol. A record may have no
/// charge: it is then the charge of the formula.
inline auto isHydrogenIon(const SubstanceRecord& record) -> bool
{
    return record.class_ == SubstanceClass::AqueousSolute && record.formula == "H+" && record.charge.value_or(1) == 1;
}

/// True if a hydrogen ion is in a convention that does not fix it at 0. The geochemical convention (benson_helgeson, the default) defines its
/// properties as 0; the convention of the record goes before that of the dataset.
inline auto hydrogenIonConventionIsZero(const SubstanceRecord& record, const Convention& dataset) -> bool
{
    const auto own = record.convention ? record.convention->energy_reference : std::nullopt;
    const auto reference = own ? own : dataset.energy_reference;
    return !reference || *reference == EnergyReference::BensonHelgeson;
}

/// True if the record gives a value that is not 0: a standard-state property or a parameter of a method.
inline auto recordHasOwnValues(const SubstanceRecord& record) -> bool
{
    const auto nonZero = [](const std::map<std::string, Quantity>& parameters) {
        return std::any_of(parameters.begin(), parameters.end(), [](const auto& p) { return p.second.value != 0.0; });
    };
    if (std::any_of(record.standard_state_properties.begin(), record.standard_state_properties.end(), [](const auto& p) { return p.second.value != 0.0; }))
        return true;
    return std::any_of(record.methods.begin(), record.methods.end(), [&](const MethodApplication& a) {
        return nonZero(a.parameters) || std::any_of(a.segments.begin(), a.segments.end(), [&](const Segment& s) { return nonZero(s.parameters); }) || a.table.has_value();
    });
}

/// True if the properties of a substance are all 0 at all T and P: a hydrogen ion in the geochemical convention, of which the record does not
/// give other values. Else the record says what the properties are.
inline auto isZeroHydrogenIon(const SubstanceRecord& record, const Convention& dataset) -> bool
{
    return isHydrogenIon(record) && hydrogenIonConventionIsZero(record, dataset) && !recordHasOwnValues(record);
}

} // namespace ThermoFun::v2

#endif
