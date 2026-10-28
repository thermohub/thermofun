// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Tests of Model/Quantity.h and Model/Vocabulary.h: every word goes round trip, aliases and unknown words.

#include <string>

#include "Model/Quantity.h"
#include "Model/Vocabulary.h"
#include "check.h"

using namespace ThermoFun;

namespace {

void testQuantity()
{
    Quantity q;
    CHECK(q.isBare());
    q.value = 5.0;
    CHECK(q.isBare());  // a value alone is bare
    q.unit_as_written = "kJ/mol";
    CHECK(!q.isBare());
    q.unit_as_written.clear();
    q.uncertainty = Uncertainty{};
    CHECK(!q.isBare());
    q.uncertainty.reset();
    q.meta.source = "1995ROB/HEM";
    CHECK(!q.isBare());

    Uncertainty u;
    CHECK(u.coverage_factor_as_written == 1.0);
    CHECK(u.kind == Uncertainty::Kind::Primary);
    CHECK(!u.degrees_of_freedom.has_value());
}

void testPlainVocabularies()
{
    for (auto p : {PropertyName::G, PropertyName::H, PropertyName::S, PropertyName::Cp, PropertyName::V,
                   PropertyName::U, PropertyName::A, PropertyName::Cv})
    {
        CHECK(propertyNameFromString(toString(p)) == p);
        CHECK(!siUnit(p).empty());
    }
    for (auto p : {ReactionPropertyName::logK, ReactionPropertyName::lnK, ReactionPropertyName::dG,
                   ReactionPropertyName::dH, ReactionPropertyName::dS, ReactionPropertyName::dCp,
                   ReactionPropertyName::dV, ReactionPropertyName::dU, ReactionPropertyName::dA})
    {
        CHECK(reactionPropertyNameFromString(toString(p)) == p);
        CHECK(!siUnit(p).empty());
    }
    CHECK(!propertyNameFromString("g").has_value());  // words are case sensitive
    CHECK(toString(PropertyName::Cp) == "Cp");
    CHECK(siUnit(PropertyName::V) == "m3/mol");
    CHECK(legacyKey(PropertyName::G) == "sm_gibbs_energy");
    CHECK(legacyKey(ReactionPropertyName::logK) == "logKr");
    CHECK(legacyKey(ReactionPropertyName::dV) == "drsm_volume");
    CHECK(siUnit(ReactionPropertyName::logK) == "1");

    for (auto v : {Quality::Core, Quality::Recommended, Quality::Supplemental})
        CHECK(qualityFromString(toString(v)) == v);
    for (auto v : {Origin::Measured, Origin::Entered, Origin::Calculated, Origin::Estimated, Origin::Converted})
        CHECK(originFromString(toString(v)) == v);
    for (auto v : {PropertySource::Direct, PropertySource::Reaction, PropertySource::Reactants})
        CHECK(propertySourceFromString(toString(v)) == v);
    for (auto v : {DatasetStatus::Draft, DatasetStatus::Published})
        CHECK(datasetStatusFromString(toString(v)) == v);
    for (auto v : {MethodKind::Symbolic, MethodKind::Reference, MethodKind::Composite})
        CHECK(methodKindFromString(toString(v)) == v);
    for (auto v : {OperatesOn::Substance, OperatesOn::Reaction})
        CHECK(operatesOnFromString(toString(v)) == v);
    CHECK(!qualityFromString("bad").has_value());
}

void testSubstanceClass()
{
    for (int code : {0, 1, 2, 3, 4, 5, 101})
    {
        const std::string word = substanceClassFromLegacy(code);
        CHECK(isSubstanceClass(word));
        CHECK(legacySubstanceClass(word) == code);
        CHECK(canonicalSubstanceClass(word) == word);
    }
    CHECK(substanceClassFromLegacy(0) == "component");
    CHECK(substanceClassFromLegacy(1) == "gas_fluid");
    CHECK(substanceClassFromLegacy(2) == "aqueous_solute");
    CHECK(substanceClassFromLegacy(3) == "water_solvent");
    CHECK(substanceClassFromLegacy(77) == "other");  // a code that is not known
    CHECK(!isSubstanceClass("mineral"));
    CHECK(legacySubstanceClass("mineral") == SubstanceClass::OTHER_SC);
    CHECK(canonicalSubstanceClass("mineral") == "other");
}

void testAggregateState()
{
    for (int code : {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 101})
    {
        const std::string word = aggregateStateFromLegacy(code);
        CHECK(isAggregateState(word));
        CHECK(legacyAggregateState(word) == code);
        CHECK(canonicalAggregateState(word) == word);
    }
    CHECK(aggregateStateFromLegacy(3) == "crystal");
    CHECK(aggregateStateFromLegacy(4) == "aqueous");
    CHECK(aggregateStateFromLegacy(7) == "other");  // 7 is not a legacy code
    // the alias of the design summary
    CHECK(isAggregateState("crystalline"));
    CHECK(canonicalAggregateState("crystalline") == "crystal");
    CHECK(legacyAggregateState("crystalline") == AggregateState::CRYSTAL);
    CHECK(legacyAggregateState("plasmoid") == AggregateState::OTHER);
}

void testElementClass()
{
    CHECK(elementClassFromLegacy(0) == "element");
    CHECK(elementClassFromLegacy(4) == "charge");
    // the meaning of 1, 2 and 3 is not verified
    for (int code : {1, 2, 3, 99})
        CHECK(elementClassFromLegacy(code) == "other");
    CHECK(legacyElementClass("element") == 0);
    CHECK(legacyElementClass("charge") == 4);
    CHECK(legacyElementClass("other") == 101);
    CHECK(legacyElementClass("isotope") == 101);
    CHECK(canonicalElementClass("isotope") == "other");
    CHECK(isElementClass("charge"));
    CHECK(!isElementClass("isotope"));
}

}

void runVocabulary()
{
    testQuantity();
    testPlainVocabularies();
    testSubstanceClass();
    testAggregateState();
    testElementClass();
}
