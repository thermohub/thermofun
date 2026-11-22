// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "ConventionConverter.h"
#include "Common/Constants.h"

#include <stdexcept>

namespace ThermoFun::model {

namespace {

const double kMolarMassWater = H2OMolarMassAtomicWeights;  // g/mol

/// G of a reference minus G of Berman-Brown: a constant and a term linear in T. All in terms of the entropies of the elements.
auto energyOffset(EnergyReference reference, const ConversionSubject& subject) -> ConventionShift
{
    ConventionShift offset;
    if (!subject.element_entropy)
        throw std::runtime_error("CNV-001: a change of the energy reference needs the entropies of the elements, and this calculation has none.");
    if (reference == EnergyReference::BensonHelgeson)
        offset.G0 = subject.Tr * subject.element_entropy();
    else if (reference == EnergyReference::GibbsHelmholtzFormation)
        offset.G_per_K = subject.element_entropy();
    return offset;
}

/// The change of the energy reference from `from` to `to`
auto energyShift(EnergyReference from, EnergyReference to, const ConversionSubject& subject) -> ConventionShift
{
    if (from == to)
        return {};
    const auto a = energyOffset(from, subject), b = energyOffset(to, subject);
    return {b.G0 - a.G0, b.G_per_K - a.G_per_K, 0.0};
}

/// The change of the standard state for a substance in this convention, seen from the canonical one
auto standardStateShift(const Convention& c, const ConversionSubject& subject) -> ConventionShift
{
    ConventionShift shift;
    // G(p°) = G(1 bar) + R T ln(p° / 1 bar) for a gas, any p° (GB4 4.11.1 (v)); 1 atm gives R T ln(1.01325)
    if (subject.gas && c.standard_state.pressure && *c.standard_state.pressure != 1e5)
        shift.G_per_K = R_CONSTANT * std::log(*c.standard_state.pressure / 1e5);
    if (subject.solute && c.standard_state.concentration_scale == ConcentrationScale::MoleFraction)
        shift.G_per_K = R_CONSTANT * std::log(1000.0 / kMolarMassWater);   // G(x) = G(m) + R T ln(1000/Mw)
    if (subject.solute && c.standard_state.concentration_scale == ConcentrationScale::Molarity)
        shift.molarity = -1.0;   // G(c) = G(m) - R T ln(rho/1000)
    return shift;
}

} // namespace

auto shiftToCanonical(const Convention& from, const ConversionSubject& subject) -> ConventionShift
{
    auto shift = energyShift(from.energy_reference.elements.value_or(EnergyReference::BensonHelgeson), EnergyReference::BensonHelgeson, subject);
    const auto state = standardStateShift(from, subject);
    shift.G_per_K -= state.G_per_K;
    shift.molarity -= state.molarity;
    return shift;
}

auto shiftFromCanonical(const Convention& to, const ConversionSubject& subject) -> ConventionShift
{
    auto shift = energyShift(EnergyReference::BensonHelgeson, to.energy_reference.elements.value_or(EnergyReference::BensonHelgeson), subject);
    const auto state = standardStateShift(to, subject);
    shift.G_per_K += state.G_per_K;
    shift.molarity += state.molarity;
    return shift;
}

} // namespace ThermoFun::model
