// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "LegacyVocabulary.h"

#include <utility>
#include <vector>

namespace ThermoFun {

namespace {

const std::vector<std::pair<v2::SubstanceClass, SubstanceClass::type>> substanceClasses = {
    {v2::SubstanceClass::Component, SubstanceClass::COMPONENT},
    {v2::SubstanceClass::GasFluid, SubstanceClass::GASFLUID},
    {v2::SubstanceClass::AqueousSolute, SubstanceClass::AQSOLUTE},
    {v2::SubstanceClass::WaterSolvent, SubstanceClass::AQSOLVENT},
    {v2::SubstanceClass::SurfaceSpecies, SubstanceClass::SURFSPECIES},
    {v2::SubstanceClass::SiteMoiety, SubstanceClass::SITEMOIETY},
    {v2::SubstanceClass::Other, SubstanceClass::OTHER_SC},
};

const std::vector<std::pair<v2::AggregateState, AggregateState::type>> aggregateStates = {
    {v2::AggregateState::Gas, AggregateState::GAS},
    {v2::AggregateState::Liquid, AggregateState::LIQUID},
    {v2::AggregateState::Glass, AggregateState::GLASS},
    {v2::AggregateState::Crystal, AggregateState::CRYSTAL},
    {v2::AggregateState::Aqueous, AggregateState::AQUEOUS},
    {v2::AggregateState::Surface, AggregateState::SURFACE},
    {v2::AggregateState::IonExchange, AggregateState::IONEX},
    {v2::AggregateState::Plasma, AggregateState::PLASMA},
    {v2::AggregateState::Solid, AggregateState::SOLID},
    {v2::AggregateState::Condensed, AggregateState::CONDENSED},
    {v2::AggregateState::Fluid, AggregateState::FLUID},
    {v2::AggregateState::LiquidCrystal, AggregateState::LIQUIDCRYSTAL},
    {v2::AggregateState::AmorphousSolid, AggregateState::AMORPHOUSSOLID},
    {v2::AggregateState::Monomeric, AggregateState::MONOMERIC},
    {v2::AggregateState::Polymeric, AggregateState::POLYMERIC},
    {v2::AggregateState::SolidSolution, AggregateState::SOLIDSOLUTION},
    {v2::AggregateState::Other, AggregateState::OTHER},
};

const int elementClassOther = 101;

} // namespace

auto substanceClassFromLegacy(int code) -> v2::SubstanceClass
{
    for (const auto& [value, legacy] : substanceClasses)
        if (static_cast<int>(legacy) == code)
            return value;
    return v2::SubstanceClass::Other;
}

auto toLegacy(v2::SubstanceClass value) -> SubstanceClass::type
{
    for (const auto& [v, legacy] : substanceClasses)
        if (v == value)
            return legacy;
    return SubstanceClass::OTHER_SC;
}

auto aggregateStateFromLegacy(int code) -> v2::AggregateState
{
    for (const auto& [value, legacy] : aggregateStates)
        if (static_cast<int>(legacy) == code)
            return value;
    return v2::AggregateState::Other;
}

auto toLegacy(v2::AggregateState value) -> AggregateState::type
{
    for (const auto& [v, legacy] : aggregateStates)
        if (v == value)
            return legacy;
    return AggregateState::OTHER;
}

auto elementClassFromLegacy(int code) -> v2::ElementClass
{
    if (code == 0)
        return v2::ElementClass::Element;
    if (code == 4)
        return v2::ElementClass::Charge;
    return v2::ElementClass::Other;
}

auto toLegacy(v2::ElementClass value) -> int
{
    switch (value)
    {
    case v2::ElementClass::Element: return 0;
    case v2::ElementClass::Charge: return 4;
    case v2::ElementClass::Other: return elementClassOther;
    }
    return elementClassOther;
}

auto legacyKey(v2::PropertyName p) -> std::string
{
    switch (p)
    {
    case v2::PropertyName::G: return "sm_gibbs_energy";
    case v2::PropertyName::H: return "sm_enthalpy";
    case v2::PropertyName::S: return "sm_entropy_abs";
    case v2::PropertyName::Cp: return "sm_heat_capacity_p";
    case v2::PropertyName::V: return "sm_volume";
    case v2::PropertyName::U: case v2::PropertyName::A: case v2::PropertyName::Cv: return "";
    }
    return "";
}

auto legacyKey(v2::ReactionPropertyName p) -> std::string
{
    switch (p)
    {
    case v2::ReactionPropertyName::logK: return "logKr";
    case v2::ReactionPropertyName::dG: return "drsm_gibbs_energy";
    case v2::ReactionPropertyName::dH: return "drsm_enthalpy";
    case v2::ReactionPropertyName::dS: return "drsm_entropy";
    case v2::ReactionPropertyName::dCp: return "drsm_heat_capacity_p";
    case v2::ReactionPropertyName::dV: return "drsm_volume";
    case v2::ReactionPropertyName::lnK: case v2::ReactionPropertyName::dU: case v2::ReactionPropertyName::dA: return "";
    }
    return "";
}

} // namespace ThermoFun
