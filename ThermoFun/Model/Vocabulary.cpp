// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Vocabulary.h"

#include <algorithm>
#include <vector>

namespace ThermoFun {

namespace {

// One row of a vocabulary: the canonical word, the aliases that a reader accepts, and (for the
// vocabularies that have one) the legacy code.
template <class E>
struct Row
{
    E value;
    const char* word;
    std::vector<const char*> aliases;
    int legacy;
};

template <class E>
auto findWord(const std::vector<Row<E>>& table, const std::string& s) -> const Row<E>*
{
    for (const auto& row : table)
    {
        if (s == row.word)
            return &row;
        for (const char* alias : row.aliases)
            if (s == alias)
                return &row;
    }
    return nullptr;
}

template <class E>
auto findValue(const std::vector<Row<E>>& table, E value) -> const Row<E>*
{
    for (const auto& row : table)
        if (row.value == value)
            return &row;
    return nullptr;
}

template <class E>
auto wordOf(const std::vector<Row<E>>& table, E value) -> std::string
{
    const auto* row = findValue(table, value);
    return row ? row->word : "";
}

template <class E>
auto valueOf(const std::vector<Row<E>>& table, const std::string& s) -> std::optional<E>
{
    const auto* row = findWord(table, s);
    if (!row)
        return std::nullopt;
    return row->value;
}

// ---------------------------------------------------------------------------------------------
// Tables of the plain vocabularies

const std::vector<Row<PropertyName>> propertyTable = {
    {PropertyName::G, "G", {}, 0},
    {PropertyName::H, "H", {}, 0},
    {PropertyName::S, "S", {}, 0},
    {PropertyName::Cp, "Cp", {}, 0},
    {PropertyName::V, "V", {}, 0},
    {PropertyName::U, "U", {}, 0},
    {PropertyName::A, "A", {}, 0},
    {PropertyName::Cv, "Cv", {}, 0},
};

const std::vector<Row<ReactionPropertyName>> reactionPropertyTable = {
    {ReactionPropertyName::logK, "logK", {}, 0},
    {ReactionPropertyName::lnK, "lnK", {}, 0},
    {ReactionPropertyName::dG, "dG", {}, 0},
    {ReactionPropertyName::dH, "dH", {}, 0},
    {ReactionPropertyName::dS, "dS", {}, 0},
    {ReactionPropertyName::dCp, "dCp", {}, 0},
    {ReactionPropertyName::dV, "dV", {}, 0},
    {ReactionPropertyName::dU, "dU", {}, 0},
    {ReactionPropertyName::dA, "dA", {}, 0},
};

const std::vector<Row<Quality>> qualityTable = {
    {Quality::Core, "core", {}, 0},
    {Quality::Recommended, "recommended", {}, 0},
    {Quality::Supplemental, "supplemental", {}, 0},
};

const std::vector<Row<Origin>> originTable = {
    {Origin::Measured, "measured", {}, 0},
    {Origin::Entered, "entered", {}, 0},
    {Origin::Calculated, "calculated", {}, 0},
    {Origin::Estimated, "estimated", {}, 0},
    {Origin::Converted, "converted", {}, 0},
};

const std::vector<Row<PropertySource>> propertySourceTable = {
    {PropertySource::Direct, "direct", {}, 0},
    {PropertySource::Reaction, "reaction", {}, 0},
    {PropertySource::Reactants, "reactants", {}, 0},
};

const std::vector<Row<DatasetStatus>> datasetStatusTable = {
    {DatasetStatus::Draft, "draft", {}, 0},
    {DatasetStatus::Published, "published", {}, 0},
};

const std::vector<Row<MethodKind>> methodKindTable = {
    {MethodKind::Symbolic, "symbolic", {}, 0},
    {MethodKind::Reference, "reference", {}, 0},
    {MethodKind::Composite, "composite", {}, 0},
};

const std::vector<Row<OperatesOn>> operatesOnTable = {
    {OperatesOn::Substance, "substance", {}, 0},
    {OperatesOn::Reaction, "reaction", {}, 0},
};

// Units and legacy keys of the properties (same order as the enums)
struct PropertyInfo
{
    const char* unit;
    const char* legacy_key;
};
const PropertyInfo propertyInfo[] = {
    {"J/mol", "sm_gibbs_energy"},      {"J/mol", "sm_enthalpy"}, {"J/(mol.K)", "sm_entropy_abs"},
    {"J/(mol.K)", "sm_heat_capacity_p"}, {"m3/mol", "sm_volume"}, {"J/mol", ""},
    {"J/mol", ""},                     {"J/(mol.K)", ""},
};
const PropertyInfo reactionPropertyInfo[] = {
    {"1", "logKr"},          {"1", ""},       {"J/mol", "drsm_gibbs_energy"},
    {"J/mol", "drsm_enthalpy"}, {"J/(mol.K)", "drsm_entropy"}, {"J/(mol.K)", "drsm_heat_capacity_p"},
    {"m3/mol", "drsm_volume"}, {"J/mol", ""}, {"J/mol", ""},
};

// ---------------------------------------------------------------------------------------------
// Tables with legacy codes. The value of a row is the legacy code.

const std::vector<Row<int>> substanceClassTable = {
    {SubstanceClass::COMPONENT, "component", {}, SubstanceClass::COMPONENT},
    {SubstanceClass::GASFLUID, "gas_fluid", {}, SubstanceClass::GASFLUID},
    {SubstanceClass::AQSOLUTE, "aqueous_solute", {}, SubstanceClass::AQSOLUTE},
    {SubstanceClass::AQSOLVENT, "water_solvent", {}, SubstanceClass::AQSOLVENT},
    {SubstanceClass::SURFSPECIES, "surface_species", {}, SubstanceClass::SURFSPECIES},
    {SubstanceClass::SITEMOIETY, "site_moiety", {}, SubstanceClass::SITEMOIETY},
    {SubstanceClass::OTHER_SC, "other", {}, SubstanceClass::OTHER_SC},
};

const std::vector<Row<int>> aggregateStateTable = {
    {AggregateState::GAS, "gas", {}, AggregateState::GAS},
    {AggregateState::LIQUID, "liquid", {}, AggregateState::LIQUID},
    {AggregateState::GLASS, "glass", {}, AggregateState::GLASS},
    {AggregateState::CRYSTAL, "crystal", {"crystalline"}, AggregateState::CRYSTAL},
    {AggregateState::AQUEOUS, "aqueous", {}, AggregateState::AQUEOUS},
    {AggregateState::SURFACE, "surface", {}, AggregateState::SURFACE},
    {AggregateState::IONEX, "ion_exchange", {}, AggregateState::IONEX},
    {AggregateState::PLASMA, "plasma", {}, AggregateState::PLASMA},
    {AggregateState::SOLID, "solid", {}, AggregateState::SOLID},
    {AggregateState::CONDENSED, "condensed", {}, AggregateState::CONDENSED},
    {AggregateState::FLUID, "fluid", {}, AggregateState::FLUID},
    {AggregateState::LIQUIDCRYSTAL, "liquid_crystal", {}, AggregateState::LIQUIDCRYSTAL},
    {AggregateState::AMORPHOUSSOLID, "amorphous_solid", {}, AggregateState::AMORPHOUSSOLID},
    {AggregateState::MONOMERIC, "monomeric", {}, AggregateState::MONOMERIC},
    {AggregateState::POLYMERIC, "polymeric", {}, AggregateState::POLYMERIC},
    {AggregateState::SOLIDSOLUTION, "solid_solution", {}, AggregateState::SOLIDSOLUTION},
    {AggregateState::OTHER, "other", {}, AggregateState::OTHER},
};

const int elementClassOther = 101;
const std::vector<Row<int>> elementClassTable = {
    {0, "element", {}, 0},
    {4, "charge", {}, 4},
    {elementClassOther, "other", {}, elementClassOther},
};

auto wordOfCode(const std::vector<Row<int>>& table, int code) -> std::string
{
    const auto* row = findValue(table, code);
    return row ? row->word : "other";
}

auto codeOfWord(const std::vector<Row<int>>& table, const std::string& word) -> int
{
    const auto* row = findWord(table, word);
    return row ? row->legacy : findWord(table, "other")->legacy;
}

auto canonical(const std::vector<Row<int>>& table, const std::string& word) -> std::string
{
    const auto* row = findWord(table, word);
    return row ? row->word : "other";
}

}

// ---------------------------------------------------------------------------------------------

auto toString(PropertyName p) -> std::string { return wordOf(propertyTable, p); }
auto propertyNameFromString(const std::string& s) -> std::optional<PropertyName> { return valueOf(propertyTable, s); }
auto toString(ReactionPropertyName p) -> std::string { return wordOf(reactionPropertyTable, p); }
auto reactionPropertyNameFromString(const std::string& s) -> std::optional<ReactionPropertyName>
{
    return valueOf(reactionPropertyTable, s);
}
auto toString(Quality v) -> std::string { return wordOf(qualityTable, v); }
auto qualityFromString(const std::string& s) -> std::optional<Quality> { return valueOf(qualityTable, s); }
auto toString(Origin v) -> std::string { return wordOf(originTable, v); }
auto originFromString(const std::string& s) -> std::optional<Origin> { return valueOf(originTable, s); }
auto toString(PropertySource v) -> std::string { return wordOf(propertySourceTable, v); }
auto propertySourceFromString(const std::string& s) -> std::optional<PropertySource>
{
    return valueOf(propertySourceTable, s);
}
auto toString(DatasetStatus v) -> std::string { return wordOf(datasetStatusTable, v); }
auto datasetStatusFromString(const std::string& s) -> std::optional<DatasetStatus>
{
    return valueOf(datasetStatusTable, s);
}
auto toString(MethodKind v) -> std::string { return wordOf(methodKindTable, v); }
auto methodKindFromString(const std::string& s) -> std::optional<MethodKind> { return valueOf(methodKindTable, s); }
auto toString(OperatesOn v) -> std::string { return wordOf(operatesOnTable, v); }
auto operatesOnFromString(const std::string& s) -> std::optional<OperatesOn> { return valueOf(operatesOnTable, s); }

auto siUnit(PropertyName p) -> std::string { return propertyInfo[static_cast<int>(p)].unit; }
auto legacyKey(PropertyName p) -> std::string { return propertyInfo[static_cast<int>(p)].legacy_key; }
auto siUnit(ReactionPropertyName p) -> std::string { return reactionPropertyInfo[static_cast<int>(p)].unit; }
auto legacyKey(ReactionPropertyName p) -> std::string { return reactionPropertyInfo[static_cast<int>(p)].legacy_key; }

auto substanceClassFromLegacy(int code) -> std::string { return wordOfCode(substanceClassTable, code); }
auto legacySubstanceClass(const std::string& word) -> SubstanceClass::type
{
    return static_cast<SubstanceClass::type>(codeOfWord(substanceClassTable, word));
}
auto canonicalSubstanceClass(const std::string& word) -> std::string { return canonical(substanceClassTable, word); }
auto isSubstanceClass(const std::string& word) -> bool { return findWord(substanceClassTable, word) != nullptr; }

auto aggregateStateFromLegacy(int code) -> std::string { return wordOfCode(aggregateStateTable, code); }
auto legacyAggregateState(const std::string& word) -> AggregateState::type
{
    return static_cast<AggregateState::type>(codeOfWord(aggregateStateTable, word));
}
auto canonicalAggregateState(const std::string& word) -> std::string { return canonical(aggregateStateTable, word); }
auto isAggregateState(const std::string& word) -> bool { return findWord(aggregateStateTable, word) != nullptr; }

auto elementClassFromLegacy(int code) -> std::string { return wordOfCode(elementClassTable, code); }
auto legacyElementClass(const std::string& word) -> int { return codeOfWord(elementClassTable, word); }
auto canonicalElementClass(const std::string& word) -> std::string { return canonical(elementClassTable, word); }
auto isElementClass(const std::string& word) -> bool { return findWord(elementClassTable, word) != nullptr; }

}
