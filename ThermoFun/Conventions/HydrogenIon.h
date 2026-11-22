// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_CONVENTIONS_HYDROGENION_H
#define THERMOFUN_CONVENTIONS_HYDROGENION_H

#include <algorithm>
#include <map>
#include <string>

#include <ThermoFun/Model/SubstanceRecord.h>

namespace ThermoFun::model {

/// True for the record of the hydrogen ion. The rule looks at the class and the formula, not at the symbol. A record may have no
/// charge: it is then the charge of the formula.
inline auto isHydrogenIon(const SubstanceRecord& record) -> bool
{
    return record.class_ == SubstanceClass::AqueousSolute && record.formula == "H+" && record.charge.value_or(1) == 1;
}

/// True for the record of the electron: an aqueous solute with the formula "-" or "e-". The formula gives the charge -1; the charge
/// of the record is not looked at (the 0.x setters write 0 for a record that gives none).
inline auto isElectron(const SubstanceRecord& record) -> bool
{
    return record.class_ == SubstanceClass::AqueousSolute && (record.formula == "-" || record.formula == "e-");
}

/// The reference of the ions of a record (decision 0031, row 6): its own, else that of the dataset, else hydrogen_ion. It does not
/// depend on the energy reference of the elements.
inline auto ionsOf(const SubstanceRecord& record, const Convention& dataset) -> Ions
{
    const auto own = record.convention ? record.convention->energy_reference.ions : std::nullopt;
    return own ? *own : dataset.energy_reference.ions.value_or(Ions::HydrogenIon);
}

/// True if the record is in the convention that fixes the hydrogen ion at 0 (ions: hydrogen_ion, the default).
inline auto hydrogenIonConventionIsZero(const SubstanceRecord& record, const Convention& dataset) -> bool
{
    return ionsOf(record, dataset) == Ions::HydrogenIon;
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

/// True if the properties of a substance are all 0 at all T and P: a hydrogen ion with ions: hydrogen_ion (NEA TDB-5 1.6). Values
/// that the record gives are not used then (validate warns, CNV-007); with ions: none the record says what the properties are.
inline auto isZeroHydrogenIon(const SubstanceRecord& record, const Convention& dataset) -> bool
{
    return isHydrogenIon(record) && hydrogenIonConventionIsZero(record, dataset);
}

/// True if the properties of a substance come from the standard hydrogen electrode, H+ + e- = 1/2 H2(g) with drG = 0 at every T
/// (NEA TDB-5 1.7.e): an electron with ions: hydrogen_ion.
inline auto isHydrogenElectrode(const SubstanceRecord& record, const Convention& dataset) -> bool
{
    return isElectron(record) && ionsOf(record, dataset) == Ions::HydrogenIon;
}

/// A formula without the valences that a GEMS formula writes between bars: "H|0|2" -> "H2"
inline auto withoutValences(const std::string& formula) -> std::string
{
    std::string out;
    bool inside = false;
    for (const char c : formula)
        if (c == '|')
            inside = !inside;
        else if (!inside)
            out += c;
    return out;
}

/// True for the record of hydrogen gas, H2(g), the gas of the hydrogen electrode
inline auto isHydrogenGas(const SubstanceRecord& record) -> bool
{
    return record.aggregate_state == AggregateState::Gas && withoutValences(record.formula) == "H2" && record.charge.value_or(0) == 0;
}

} // namespace ThermoFun::model

#endif
