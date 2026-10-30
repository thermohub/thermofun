// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Tests of Model/Quantity.h, Model/Vocabulary.h and IO/Legacy/LegacyVocabulary.h: every value of every list goes
// round trip, aliases, unknown words and the legacy codes.

#include <string>
#include <vector>

#include "IO/Legacy/LegacyVocabulary.h"
#include "Model/Quantity.h"
#include "Model/Vocabulary.h"
#include "check.h"

using namespace ThermoFun;

namespace {

void testQuantity()
{
    v2::Quantity q;
    CHECK(q.isBare());
    q.value = 5.0;
    CHECK(q.isBare());  // a value alone is bare
    q.single_precision_source = true;
    CHECK(q.isBare());  // it changes how the number is written, nothing else
    q.unit_as_written = "kJ/mol";
    CHECK(!q.isBare());
    q.unit_as_written.clear();
    q.uncertainty = v2::Uncertainty{};
    CHECK(!q.isBare());
    q.uncertainty.reset();
    q.meta.source = "1995ROB/HEM";
    CHECK(!q.isBare());
    q.meta.source.clear();
    q.meta.quality = v2::Quality::Core;
    CHECK(!q.isBare());
    q.meta.quality.reset();
    q.meta.origin = v2::Origin::Measured;
    CHECK(!q.isBare());
}

// Each value must go round trip, and the list of the test must be the whole list of the library.
template <class E>
void roundTrip(const std::vector<E>& values, const std::vector<std::string>& expected)
{
    CHECK(values.size() == expected.size());
    CHECK(v2::words<E>() == expected);
    for (size_t i = 0; i < values.size(); ++i)
    {
        CHECK(v2::toString(values[i]) == expected[i]);
        CHECK(v2::fromString<E>(expected[i]) == values[i]);
        CHECK(v2::fromString<E>(v2::toString(values[i])) == values[i]);
    }
    CHECK(!v2::fromString<E>("no_such_word"));
    CHECK(!v2::fromString<E>(""));
    CHECK(!v2::vocabularyName<E>().empty());
}

void testLists()
{
    using namespace v2;
    using PN = PropertyName;
    roundTrip<PN>({PN::G, PN::H, PN::S, PN::Cp, PN::V, PN::U, PN::A, PN::Cv}, {"G", "H", "S", "Cp", "V", "U", "A", "Cv"});
    using RP = ReactionPropertyName;
    roundTrip<RP>({RP::logK, RP::lnK, RP::dG, RP::dH, RP::dS, RP::dCp, RP::dV, RP::dU, RP::dA},
                  {"logK", "lnK", "dG", "dH", "dS", "dCp", "dV", "dU", "dA"});
    roundTrip<Quality>({Quality::Core, Quality::Recommended, Quality::Supplemental}, {"core", "recommended", "supplemental"});
    roundTrip<Origin>({Origin::Measured, Origin::Entered, Origin::Calculated, Origin::Estimated, Origin::Converted},
                      {"measured", "entered", "calculated", "estimated", "converted"});
    roundTrip<PropertySource>({PropertySource::Direct, PropertySource::Reaction, PropertySource::Reactants},
                              {"direct", "reaction", "reactants"});
    roundTrip<DatasetStatus>({DatasetStatus::Draft, DatasetStatus::Published}, {"draft", "published"});
    roundTrip<MethodKind>({MethodKind::Symbolic, MethodKind::Reference, MethodKind::Composite}, {"symbolic", "reference", "composite"});
    roundTrip<OperatesOn>({OperatesOn::Substance, OperatesOn::Reaction}, {"substance", "reaction"});
    using SC = v2::SubstanceClass;
    roundTrip<SC>({SC::Component, SC::GasFluid, SC::AqueousSolute, SC::WaterSolvent, SC::SurfaceSpecies, SC::SiteMoiety, SC::Other},
                  {"component", "gas_fluid", "aqueous_solute", "water_solvent", "surface_species", "site_moiety", "other"});
    using AS = v2::AggregateState;
    roundTrip<AS>({AS::Gas, AS::Liquid, AS::Glass, AS::Crystal, AS::Aqueous, AS::Surface, AS::IonExchange, AS::Plasma, AS::Solid,
                   AS::Condensed, AS::Fluid, AS::LiquidCrystal, AS::AmorphousSolid, AS::Monomeric, AS::Polymeric,
                   AS::SolidSolution, AS::Other},
                  {"gas", "liquid", "glass", "crystal", "aqueous", "surface", "ion_exchange", "plasma", "solid", "condensed", "fluid",
                   "liquid_crystal", "amorphous_solid", "monomeric", "polymeric", "solid_solution", "other"});
    roundTrip<ElementClass>({ElementClass::Element, ElementClass::Charge, ElementClass::Other}, {"element", "charge", "other"});
    roundTrip<Interpolation>({Interpolation::Lagrange, Interpolation::CubicSpline, Interpolation::Bilinear},
                             {"lagrange", "cubic_spline", "bilinear"});
    using ER = EnergyReference;
    roundTrip<ER>({ER::BensonHelgeson, ER::BermanBrown, ER::SER, ER::AssignedEnthalpy, ER::GibbsHelmholtzFormation, ER::DFTFormation0K},
                  {"benson_helgeson", "berman_brown", "ser", "assigned_enthalpy", "gibbs_helmholtz_formation", "dft_formation_0K"});
    roundTrip<StandardPressure>({StandardPressure::OneBar, StandardPressure::OneAtm}, {"1bar", "1atm"});
    roundTrip<ConcentrationScale>({ConcentrationScale::Molality, ConcentrationScale::MoleFraction, ConcentrationScale::Molarity},
                                  {"molality", "mole_fraction", "molarity"});
    roundTrip<WaterReference>({WaterReference::Geochemical, WaterReference::SteamTables}, {"geochemical", "steam_tables"});
}

void testAliasesAndCase()
{
    using namespace v2;
    CHECK(fromString<v2::AggregateState>("crystalline") == v2::AggregateState::Crystal);
    CHECK(fromString<Interpolation>("spline") == Interpolation::CubicSpline);
    CHECK(fromString<StandardPressure>("1 bar") == StandardPressure::OneBar);
    CHECK(fromString<StandardPressure>("1 atm") == StandardPressure::OneAtm);
    // the words of the old engine conventions
    CHECK(fromString<EnergyReference>("BERMAN_BROWN") == EnergyReference::BermanBrown);
    CHECK(fromString<EnergyReference>("BENSON_HELGESON") == EnergyReference::BensonHelgeson);
    CHECK(fromString<WaterReference>("STEAM_TABLES") == WaterReference::SteamTables);
    CHECK(fromString<WaterReference>("GEOCHEMICAL") == WaterReference::Geochemical);
    // any case, and '-' for '_'
    CHECK(fromString<v2::AggregateState>("GAS") == v2::AggregateState::Gas);
    CHECK(fromString<WaterReference>("Steam-Tables") == WaterReference::SteamTables);
    CHECK(fromString<EnergyReference>("dft_formation_0k") == EnergyReference::DFTFormation0K);
    // a writer writes the canonical word, never the alias
    CHECK(toString(*fromString<v2::AggregateState>("crystalline")) == "crystal");
}

void testUnknownWords()
{
    using namespace v2;
    CHECK(nearestWord<v2::AggregateState>("crystl") == "crystal");
    CHECK(nearestWord<EnergyReference>("berman_browne") == "berman_brown");
    CHECK(nearestWord<WaterReference>("steam_table") == "steam_tables");
    CHECK(nearestWord<v2::AggregateState>("xxxxxxxxxx").empty());  // nothing near
    CHECK(nearestWord<PropertyName>("X").empty());                 // no guess for one letter
    CHECK(nearestWord<v2::AggregateState>("cristalline") == "crystal");  // near an alias: the canonical word

    const auto message = unknownWordMessage<WaterReference>("steam_table");
    CHECK(message == "\"steam_table\" is not a value of water_reference. Did you mean \"steam_tables\"? "
                     "Allowed: geochemical, steam_tables.");
    CHECK(unknownWordMessage<WaterReference>("zzzzzzzz").find("Did you mean") == std::string::npos);
}

void testUnits()
{
    using namespace v2;
    CHECK(siUnit(PropertyName::G) == "J/mol");
    CHECK(siUnit(PropertyName::S) == "J/(mol.K)");
    CHECK(siUnit(PropertyName::V) == "m3/mol");
    CHECK(siUnit(ReactionPropertyName::logK) == "1");
    CHECK(siUnit(ReactionPropertyName::dCp) == "J/(mol.K)");
    CHECK(siUnit(ReactionPropertyName::dV) == "m3/mol");
}

void testLegacy()
{
    // every word has a code, and the code gives the word back
    for (const auto& word : v2::words<v2::SubstanceClass>())
    {
        const auto value = *v2::fromString<v2::SubstanceClass>(word);
        CHECK(substanceClassFromLegacy(static_cast<int>(toLegacy(value))) == value);
    }
    for (const auto& word : v2::words<v2::AggregateState>())
    {
        const auto value = *v2::fromString<v2::AggregateState>(word);
        CHECK(aggregateStateFromLegacy(static_cast<int>(toLegacy(value))) == value);
    }
    for (const auto& word : v2::words<v2::ElementClass>())
    {
        const auto value = *v2::fromString<v2::ElementClass>(word);
        CHECK(elementClassFromLegacy(toLegacy(value)) == value);
    }
    CHECK(toLegacy(v2::SubstanceClass::AqueousSolute) == SubstanceClass::AQSOLUTE);
    CHECK(toLegacy(v2::AggregateState::Gas) == AggregateState::GAS);
    CHECK(toLegacy(v2::ElementClass::Charge) == 4);
    // a code that is not known gives Other
    CHECK(substanceClassFromLegacy(77) == v2::SubstanceClass::Other);
    CHECK(aggregateStateFromLegacy(77) == v2::AggregateState::Other);
    CHECK(elementClassFromLegacy(2) == v2::ElementClass::Other);
    CHECK(toLegacy(v2::ElementClass::Other) == 101);
    // the keys of legacy records
    CHECK(legacyKey(v2::PropertyName::G) == "sm_gibbs_energy");
    CHECK(legacyKey(v2::PropertyName::V) == "sm_volume");
    CHECK(legacyKey(v2::PropertyName::U).empty());
    CHECK(legacyKey(v2::ReactionPropertyName::logK) == "logKr");
    CHECK(legacyKey(v2::ReactionPropertyName::dU).empty());
}

} // namespace

void runVocabulary()
{
    testQuantity();
    testLists();
    testAliasesAndCase();
    testUnknownWords();
    testUnits();
    testLegacy();
}
