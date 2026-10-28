// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_VOCABULARY_H
#define THERMOFUN_MODEL_VOCABULARY_H

#include <optional>
#include <string>

#include "GlobalVariables.h"

// The words of the data format (v2 strings) and their legacy integer codes.
// The values are provisional. All tables, with their aliases, are in Vocabulary.cpp.
// A reader accepts an alias and uses the canonical word. A writer writes the canonical word.

namespace ThermoFun {

/// Standard-state properties of a substance
enum class PropertyName
{
    G,
    H,
    S,
    Cp,
    V,
    U,
    A,
    Cv
};

/// Standard-state properties of a reaction
enum class ReactionPropertyName
{
    logK,
    lnK,
    dG,
    dH,
    dS,
    dCp,
    dV,
    dU,
    dA
};

enum class Quality
{
    Core,
    Recommended,
    Supplemental
};

enum class Origin
{
    Measured,
    Entered,
    Calculated,
    Estimated,
    Converted
};

/// Where the properties of a substance or a reaction come from
enum class PropertySource
{
    Direct,
    Reaction,   ///< substance: from its defining reaction
    Reactants   ///< reaction: from its reactants
};

enum class DatasetStatus
{
    Draft,
    Published
};

enum class MethodKind
{
    Symbolic,
    Reference,
    Composite
};

enum class OperatesOn
{
    Substance,
    Reaction
};

// For each enum: toString() gives the canonical word, and xFromString() reads a word or an alias.
// xFromString() gives an empty value for a word that is not in the vocabulary.
auto toString(PropertyName p) -> std::string;
auto propertyNameFromString(const std::string& s) -> std::optional<PropertyName>;
auto toString(ReactionPropertyName p) -> std::string;
auto reactionPropertyNameFromString(const std::string& s) -> std::optional<ReactionPropertyName>;
auto toString(Quality v) -> std::string;
auto qualityFromString(const std::string& s) -> std::optional<Quality>;
auto toString(Origin v) -> std::string;
auto originFromString(const std::string& s) -> std::optional<Origin>;
auto toString(PropertySource v) -> std::string;
auto propertySourceFromString(const std::string& s) -> std::optional<PropertySource>;
auto toString(DatasetStatus v) -> std::string;
auto datasetStatusFromString(const std::string& s) -> std::optional<DatasetStatus>;
auto toString(MethodKind v) -> std::string;
auto methodKindFromString(const std::string& s) -> std::optional<MethodKind>;
auto toString(OperatesOn v) -> std::string;
auto operatesOnFromString(const std::string& s) -> std::optional<OperatesOn>;

/// The SI unit (UCUM) of a property, and its key in a legacy record
auto siUnit(PropertyName p) -> std::string;
auto legacyKey(PropertyName p) -> std::string;
auto siUnit(ReactionPropertyName p) -> std::string;
auto legacyKey(ReactionPropertyName p) -> std::string;

// Substance class, aggregate state and element class. A word that is not in the vocabulary becomes "other".

/// The word of a legacy code of the substance class, for example 2 gives "aqueous_solute"
auto substanceClassFromLegacy(int code) -> std::string;
/// The legacy code of a word or an alias. A word that is not known gives OTHER_SC.
auto legacySubstanceClass(const std::string& word) -> SubstanceClass::type;
/// The canonical word of a word or an alias. A word that is not known gives "other".
auto canonicalSubstanceClass(const std::string& word) -> std::string;
/// True if the word or the alias is in the vocabulary.
auto isSubstanceClass(const std::string& word) -> bool;

auto aggregateStateFromLegacy(int code) -> std::string;
auto legacyAggregateState(const std::string& word) -> AggregateState::type;
auto canonicalAggregateState(const std::string& word) -> std::string;
auto isAggregateState(const std::string& word) -> bool;

/// Only the codes 0 (element) and 4 (charge) have a word. The meaning of 1, 2 and 3 is not verified, so
/// they give "other". A reader keeps the raw code of such an element.
auto elementClassFromLegacy(int code) -> std::string;
/// The legacy code of a word. "other" gives 101.
auto legacyElementClass(const std::string& word) -> int;
auto canonicalElementClass(const std::string& word) -> std::string;
auto isElementClass(const std::string& word) -> bool;

}

#endif
