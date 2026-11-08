// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "LegacyVocabulary.h"

#include <utility>
#include <vector>

namespace ThermoFun {

namespace {

const std::vector<std::pair<model::SubstanceClass, SubstanceClass::type>> substanceClasses = {
    {model::SubstanceClass::Component, SubstanceClass::COMPONENT},
    {model::SubstanceClass::GasFluid, SubstanceClass::GASFLUID},
    {model::SubstanceClass::AqueousSolute, SubstanceClass::AQSOLUTE},
    {model::SubstanceClass::WaterSolvent, SubstanceClass::AQSOLVENT},
    {model::SubstanceClass::SurfaceSpecies, SubstanceClass::SURFSPECIES},
    {model::SubstanceClass::SiteMoiety, SubstanceClass::SITEMOIETY},
    {model::SubstanceClass::Other, SubstanceClass::OTHER_SC},
};

const std::vector<std::pair<model::AggregateState, AggregateState::type>> aggregateStates = {
    {model::AggregateState::Gas, AggregateState::GAS},
    {model::AggregateState::Liquid, AggregateState::LIQUID},
    {model::AggregateState::Glass, AggregateState::GLASS},
    {model::AggregateState::Crystal, AggregateState::CRYSTAL},
    {model::AggregateState::Aqueous, AggregateState::AQUEOUS},
    {model::AggregateState::Surface, AggregateState::SURFACE},
    {model::AggregateState::IonExchange, AggregateState::IONEX},
    {model::AggregateState::Plasma, AggregateState::PLASMA},
    {model::AggregateState::Solid, AggregateState::SOLID},
    {model::AggregateState::Condensed, AggregateState::CONDENSED},
    {model::AggregateState::Fluid, AggregateState::FLUID},
    {model::AggregateState::LiquidCrystal, AggregateState::LIQUIDCRYSTAL},
    {model::AggregateState::AmorphousSolid, AggregateState::AMORPHOUSSOLID},
    {model::AggregateState::Monomeric, AggregateState::MONOMERIC},
    {model::AggregateState::Polymeric, AggregateState::POLYMERIC},
    {model::AggregateState::SolidSolution, AggregateState::SOLIDSOLUTION},
    {model::AggregateState::Other, AggregateState::OTHER},
};

const int elementClassOther = 101;

} // namespace

auto substanceClassFromLegacy(int code) -> model::SubstanceClass
{
    for (const auto& [value, legacy] : substanceClasses)
        if (static_cast<int>(legacy) == code)
            return value;
    return model::SubstanceClass::Other;
}

auto toLegacy(model::SubstanceClass value) -> SubstanceClass::type
{
    for (const auto& [v, legacy] : substanceClasses)
        if (v == value)
            return legacy;
    return SubstanceClass::OTHER_SC;
}

auto aggregateStateFromLegacy(int code) -> model::AggregateState
{
    for (const auto& [value, legacy] : aggregateStates)
        if (static_cast<int>(legacy) == code)
            return value;
    return model::AggregateState::Other;
}

auto toLegacy(model::AggregateState value) -> AggregateState::type
{
    for (const auto& [v, legacy] : aggregateStates)
        if (v == value)
            return legacy;
    return AggregateState::OTHER;
}

auto elementClassFromLegacy(int code) -> model::ElementClass
{
    if (code == 0)
        return model::ElementClass::Element;
    if (code == 4)
        return model::ElementClass::Charge;
    return model::ElementClass::Other;
}

auto toLegacy(model::ElementClass value) -> int
{
    switch (value)
    {
    case model::ElementClass::Element: return 0;
    case model::ElementClass::Charge: return 4;
    case model::ElementClass::Other: return elementClassOther;
    }
    return elementClassOther;
}

auto legacyKey(model::PropertyName p) -> std::string
{
    switch (p)
    {
    case model::PropertyName::G: return "sm_gibbs_energy";
    case model::PropertyName::H: return "sm_enthalpy";
    case model::PropertyName::S: return "sm_entropy_abs";
    case model::PropertyName::Cp: return "sm_heat_capacity_p";
    case model::PropertyName::V: return "sm_volume";
    case model::PropertyName::U: case model::PropertyName::A: case model::PropertyName::Cv: return "";
    }
    return "";
}

auto legacyKey(model::ReactionPropertyName p) -> std::string
{
    switch (p)
    {
    case model::ReactionPropertyName::logK: return "logKr";
    case model::ReactionPropertyName::dG: return "drsm_gibbs_energy";
    case model::ReactionPropertyName::dH: return "drsm_enthalpy";
    case model::ReactionPropertyName::dS: return "drsm_entropy";
    case model::ReactionPropertyName::dCp: return "drsm_heat_capacity_p";
    case model::ReactionPropertyName::dV: return "drsm_volume";
    case model::ReactionPropertyName::lnK: case model::ReactionPropertyName::dU: case model::ReactionPropertyName::dA: return "";
    }
    return "";
}

} // namespace ThermoFun
