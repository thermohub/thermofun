// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Tests of Methods/MethodCatalog.h: the approved ids, unique names, complete specifications and units.

#include <set>
#include <string>
#include <vector>

#include "Common/Ucum.h"
#include "Methods/MethodCatalog.h"
#include "check.h"

using namespace ThermoFun;
using namespace ThermoFun::v2;

namespace {

const std::vector<std::string> substanceIds = {
    "ConstantProperties", "CpPolynomial", "CpFeiSaxena86", "ConstantCp", "CpExtendedUNIQUAC", "NASA9", "TPTable",
    "HKF88", "AkinfievDiamond03", "SoluteHP98", "AndersonEtAl91",
    "LandauHP98", "LambdaBerman88", "ConstantVolume", "VolumeBerman88", "VolumeDorogokupets88", "BirchMurnaghan47",
    "MurnaghanHP98", "TaitHP11",
    "IdealGas", "PRSV86", "ChurakovGottschalk03", "SoaveRedlichKwong72", "SternerPitzer94", "PengRobinson78", "CorkHP91",
    "WaterHGK84", "WaterIAPWS95", "WaterZhangDuan05", "JohnsonNorton91", "SverjenskyEtAl14", "FernandezEtAl97"};

const std::vector<std::string> reactionIds = {
    "ConstantReactionProperties", "LogKPolynomial", "LogKNordstromMunoz88", "LogKConstant", "LogKZeroEntropy", "LogKVantHoff",
    "LogKConstantCp", "LogKTable", "MarshallFranck81", "RyzhenkoBryzgalin", "DolejsManning10", "ReactionCpPolynomial",
    "ReactionVolumePolynomial", "ReactionConstantVolume"};

void testApprovedIds()
{
    const auto& catalog = MethodCatalog::instance();
    CHECK(catalog.specs().size() == substanceIds.size() + reactionIds.size());
    for (const auto& id : substanceIds)
    {
        const auto* s = catalog.find(id);
        CHECK(s != nullptr && s->id == id && s->operates_on == OperatesOn::Substance);
    }
    for (const auto& id : reactionIds)
    {
        const auto* s = catalog.find(id);
        CHECK(s != nullptr && s->id == id && s->operates_on == OperatesOn::Reaction);
    }
}

void testNamesAreUnique()
{
    std::set<std::string> names;
    for (const auto& s : MethodCatalog::instance().specs())
    {
        CHECK(names.insert(s.id).second);
        for (const auto& alias : s.aliases)
            CHECK(names.insert(alias).second);
    }
}

void testComplete()
{
    for (const auto& s : MethodCatalog::instance().specs())
    {
        CHECK(!s.title.empty());
        CHECK(!s.summary.empty() && s.summary.back() == '.');
        // no legacy name in a specification
        for (const auto& alias : s.aliases)
            CHECK(alias.find('_') == std::string::npos);
        // a reaction method has the role of a reaction method, and the effects of its kind
        CHECK((s.operates_on == OperatesOn::Reaction) == (s.role == MethodRole::ReactionProperties));
        // the names of the parameters are unique
        std::set<std::string> parameters;
        for (const auto& p : s.parameters)
        {
            CHECK(parameters.insert(p.name).second);
            CHECK(!p.description.empty());
        }
        // a transformation reads the incoming result, and a base method does not
        CHECK(!(s.effects.base && s.effects.transform));
    }
}

void testUnits()
{
    for (const auto& s : MethodCatalog::instance().specs())
        for (const auto& p : s.parameters)
        {
            CHECK(!p.si_unit.empty() && !p.source_unit.empty());
            CHECK(units::dimension(p.si_unit) == units::dimension(p.source_unit));
        }
    // the units that the models read
    const auto* bm = MethodCatalog::instance().find("BirchMurnaghan47");
    CHECK(bm != nullptr && bm->parameters[3].name == "kt00" && bm->parameters[3].source_unit == "MPa");
    const auto* dm = MethodCatalog::instance().find("DolejsManning10");
    CHECK(dm != nullptr && dm->parameters[0].name == "a" && dm->parameters[0].source_unit == "kJ/mol");
}

void testAliasesAndImplementations()
{
    const auto& catalog = MethodCatalog::instance();
    // the ids of the first plan still read
    CHECK(catalog.find("HKFReaktoro")->id == "HKF88");
    CHECK(catalog.find("HKFGems")->id == "HKF88");
    CHECK(catalog.find("WaterWP95")->id == "WaterIAPWS95");
    CHECK(catalog.find("BirchMurnaghanGottschalk97")->id == "BirchMurnaghan47");
    CHECK(catalog.find("FrantzMarshall")->id == "MarshallFranck81");
    CHECK(catalog.find("no_such_method") == nullptr);
    // a filter by what the method operates on
    CHECK(catalog.find("HKF88", OperatesOn::Reaction) == nullptr);
    CHECK(catalog.find("HKF88", OperatesOn::Substance) != nullptr);
    // one id for the variants; the first implementation is the default
    CHECK(catalog.find("HKF88")->implementations == (std::vector<std::string>{"reaktoro", "gems"}));
    CHECK(catalog.find("WaterHGK84")->implementations.front() == "reaktoro");
    CHECK(catalog.find("JohnsonNorton91")->implementations.size() == 2);
    // not implemented: the id is known
    CHECK(!catalog.find("LambdaBerman88")->implemented);
    CHECK(!catalog.find("LogKTable")->implemented);
    CHECK(catalog.find("HKF88")->implemented);
    // "did you mean"
    CHECK(catalog.suggest("HKF8") == "HKF88");
    CHECK(catalog.suggest("waterhgk") == "WaterHGK84");
    CHECK(catalog.suggest("TaitHP12") == "TaitHP11");
    CHECK(catalog.suggest("completely_other_words").empty());
}

void testFacts()
{
    const auto& catalog = MethodCatalog::instance();
    // classes come from the data: Cp and constant volume are used by all classes
    CHECK(catalog.find("CpPolynomial")->applies_to_classes.empty());
    CHECK(catalog.find("ConstantVolume")->applies_to_classes.empty());
    CHECK(catalog.find("HKF88")->applies_to_classes == (std::vector<SubstanceClass>{SubstanceClass::AqueousSolute}));
    CHECK(catalog.find("PRSV86")->applies_to_classes == (std::vector<SubstanceClass>{SubstanceClass::GasFluid}));
    // what the methods need
    CHECK(catalog.find("HKF88")->needs.solvent);
    CHECK(catalog.find("AkinfievDiamond03")->needs.ideal_gas_water);
    CHECK(catalog.find("AndersonEtAl91")->needs.solvent_at_reference);
    CHECK(!catalog.find("CpPolynomial")->needs.solvent);
    // effects
    CHECK(catalog.find("CpPolynomial")->effects.base);
    CHECK(catalog.find("LandauHP98")->effects.G == Effect::Add);
    CHECK(catalog.find("AkinfievDiamond03")->effects.transform);
    CHECK(catalog.find("RyzhenkoBryzgalin")->reaction_effects.final);
    CHECK(catalog.find("ReactionConstantVolume")->reaction_effects.dV == Effect::Set);
    // a count of atoms cannot be fitted
    bool found = false;
    for (const auto& p : catalog.find("TaitHP11")->parameters)
        if (p.name == "n_atoms")
            found = !p.fittable;
    CHECK(found);
}

} // namespace

void runCatalog()
{
    testApprovedIds();
    testNamesAreUnique();
    testComplete();
    testUnits();
    testAliasesAndImplementations();
    testFacts();
}
