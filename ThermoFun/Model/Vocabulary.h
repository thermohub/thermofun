// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_VOCABULARY_H
#define THERMOFUN_MODEL_VOCABULARY_H

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The closed lists of words of the data format. Each list is an enum class; the words are what v2 files contain.
// All words and aliases are in one table per list in Vocabulary.cpp. A reader accepts a word or an alias in any
// case; a writer writes the canonical word. The legacy codes are in IO/Legacy/LegacyVocabulary.h.

namespace ThermoFun::v2 {

/// Standard-state properties of a substance
enum class PropertyName { G, H, S, Cp, V, U, A, Cv };

/// Standard-state properties of a reaction
enum class ReactionPropertyName { logK, lnK, dG, dH, dS, dCp, dV, dU, dA };

/// How much a value is trusted
enum class Quality { Core, Recommended, Supplemental };

/// How a value was obtained
enum class Origin { Measured, Entered, Calculated, Estimated, Converted };

/// Where the properties of a substance or a reaction come from
enum class PropertySource
{
    Direct,
    Reaction,   ///< substance: from its defining reaction
    Reactants   ///< reaction: from its reactants
};

/// The state of a thermodynamic dataset
enum class DatasetStatus { Draft, Published };

/// The kind of a method record
enum class MethodKind { Symbolic, Reference, Composite };

/// What a method calculates the properties of
enum class OperatesOn { Substance, Reaction };

/// The class of a substance
enum class SubstanceClass { Component, GasFluid, AqueousSolute, WaterSolvent, SurfaceSpecies, SiteMoiety, Other };

/// The aggregate state of a substance
enum class AggregateState
{
    Gas, Liquid, Glass, Crystal, Aqueous, Surface, IonExchange, Plasma, Solid, Condensed, Fluid, LiquidCrystal,
    AmorphousSolid, Monomeric, Polymeric, SolidSolution, Other
};

/// The class of an element
enum class ElementClass { Element, Charge, Other };

/// How a table is interpolated
enum class Interpolation { Lagrange, CubicSpline, Bilinear };

/// The reference of the energies (G, H, S) of a substance
enum class EnergyReference { BensonHelgeson, BermanBrown, SER, AssignedEnthalpy, GibbsHelmholtzFormation, DFTFormation0K };

/// The standard-state pressure
enum class StandardPressure
{
    OneBar,  ///< 1e5 Pa
    OneAtm   ///< 101325 Pa
};

/// The concentration scale of aqueous solutes
enum class ConcentrationScale { Molality, MoleFraction, Molarity };

/// The reference of the properties of water
enum class WaterReference
{
    Geochemical,  ///< the reference of HKF and HGK (1974) at the triple point
    SteamTables   ///< IAPWS: U and S of liquid water are 0 at the triple point
};

// The functions below exist for each enum above.

/// The canonical word of a value, as written in v2 files.
template <class E> auto toString(E value) -> std::string;

/// The value of a word or an alias, in any case. Empty if the word is not in the list.
template <class E> auto fromString(std::string_view word) -> std::optional<E>;

/// The canonical words of the list, in the order of the enum.
template <class E> auto words() -> std::vector<std::string>;

/// The canonical word nearest to the given word, for "did you mean", or "" if no word is near.
template <class E> auto nearestWord(std::string_view word) -> std::string;

/// The name of the list as a key of a v2 file, for example "aggregate_state".
template <class E> auto vocabularyName() -> std::string;

/// A message for a word that is not in the list, with the nearest word and the allowed words, for example
/// `"crystl" is not a value of aggregate_state. Did you mean "crystal"? Allowed: gas, liquid, ...`.
template <class E> auto unknownWordMessage(std::string_view word) -> std::string;

/// The SI unit (UCUM) of a property.
auto siUnit(PropertyName p) -> std::string;
auto siUnit(ReactionPropertyName p) -> std::string;

} // namespace ThermoFun::v2

#endif
