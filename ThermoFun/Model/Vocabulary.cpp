// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Vocabulary.h"

#include <algorithm>
#include <cctype>

namespace ThermoFun::v2 {

namespace {

// One row of a list: the value, its canonical word and the aliases a reader accepts.
template <class E>
struct Row
{
    E value;
    const char* word;
    std::vector<const char*> aliases;
};

// The name of each list (a key of a v2 file) and its rows. One specialization for each enum.
template <class E>
struct Table;

#define THERMOFUN_VOCABULARY(E, NAME, ...)                                                                            \
    template <>                                                                                                       \
    struct Table<E>                                                                                                   \
    {                                                                                                                 \
        static constexpr const char* name = NAME;                                                                     \
        static auto rows() -> const std::vector<Row<E>>&                                                              \
        {                                                                                                             \
            static const std::vector<Row<E>> r = {__VA_ARGS__};                                                       \
            return r;                                                                                                 \
        }                                                                                                             \
    };

using PN = PropertyName;
THERMOFUN_VOCABULARY(PropertyName, "property",
    {PN::G, "G", {}}, {PN::H, "H", {}}, {PN::S, "S", {}}, {PN::Cp, "Cp", {}}, {PN::V, "V", {}},
    {PN::U, "U", {}}, {PN::A, "A", {}}, {PN::Cv, "Cv", {}})

using RPN = ReactionPropertyName;
THERMOFUN_VOCABULARY(ReactionPropertyName, "reaction_property",
    {RPN::logK, "logK", {}}, {RPN::lnK, "lnK", {}}, {RPN::dG, "dG", {}}, {RPN::dH, "dH", {}}, {RPN::dS, "dS", {}},
    {RPN::dCp, "dCp", {}}, {RPN::dV, "dV", {}}, {RPN::dU, "dU", {}}, {RPN::dA, "dA", {}})

THERMOFUN_VOCABULARY(Quality, "quality",
    {Quality::Core, "core", {}}, {Quality::Recommended, "recommended", {}}, {Quality::Supplemental, "supplemental", {}})

THERMOFUN_VOCABULARY(Origin, "origin",
    {Origin::Measured, "measured", {}}, {Origin::Entered, "entered", {}}, {Origin::Calculated, "calculated", {}},
    {Origin::Estimated, "estimated", {}}, {Origin::Converted, "converted", {}})

THERMOFUN_VOCABULARY(PropertySource, "property_source",
    {PropertySource::Direct, "direct", {}}, {PropertySource::Reaction, "reaction", {}},
    {PropertySource::Reactants, "reactants", {}})

THERMOFUN_VOCABULARY(DatasetStatus, "status",
    {DatasetStatus::Draft, "draft", {}}, {DatasetStatus::Published, "published", {}})

THERMOFUN_VOCABULARY(MethodKind, "kind",
    {MethodKind::Symbolic, "symbolic", {}}, {MethodKind::Reference, "reference", {}},
    {MethodKind::Composite, "composite", {}})

THERMOFUN_VOCABULARY(OperatesOn, "operates_on",
    {OperatesOn::Substance, "substance", {}}, {OperatesOn::Reaction, "reaction", {}})

using SC = SubstanceClass;
THERMOFUN_VOCABULARY(SubstanceClass, "class",
    {SC::Component, "component", {}}, {SC::GasFluid, "gas_fluid", {}}, {SC::AqueousSolute, "aqueous_solute", {}},
    {SC::WaterSolvent, "water_solvent", {}}, {SC::SurfaceSpecies, "surface_species", {}},
    {SC::SiteMoiety, "site_moiety", {}}, {SC::Other, "other", {}})

using AS = AggregateState;
THERMOFUN_VOCABULARY(AggregateState, "aggregate_state",
    {AS::Gas, "gas", {}}, {AS::Liquid, "liquid", {}}, {AS::Glass, "glass", {}}, {AS::Crystal, "crystal", {"crystalline"}},
    {AS::Aqueous, "aqueous", {}}, {AS::Surface, "surface", {}}, {AS::IonExchange, "ion_exchange", {}},
    {AS::Plasma, "plasma", {}}, {AS::Solid, "solid", {}}, {AS::Condensed, "condensed", {}}, {AS::Fluid, "fluid", {}},
    {AS::LiquidCrystal, "liquid_crystal", {}}, {AS::AmorphousSolid, "amorphous_solid", {}},
    {AS::Monomeric, "monomeric", {}}, {AS::Polymeric, "polymeric", {}}, {AS::SolidSolution, "solid_solution", {}},
    {AS::Other, "other", {}})

THERMOFUN_VOCABULARY(ElementClass, "class",
    {ElementClass::Element, "element", {}}, {ElementClass::Charge, "charge", {}}, {ElementClass::Other, "other", {}})

THERMOFUN_VOCABULARY(Interpolation, "interpolation",
    {Interpolation::Lagrange, "lagrange", {}}, {Interpolation::CubicSpline, "cubic_spline", {"spline"}},
    {Interpolation::Bilinear, "bilinear", {}})

using ER = EnergyReference;
THERMOFUN_VOCABULARY(EnergyReference, "energy_reference",
    {ER::BensonHelgeson, "benson_helgeson", {}}, {ER::BermanBrown, "berman_brown", {}}, {ER::SER, "ser", {}},
    {ER::AssignedEnthalpy, "assigned_enthalpy", {}}, {ER::GibbsHelmholtzFormation, "gibbs_helmholtz_formation", {}},
    {ER::DFTFormation0K, "dft_formation_0K", {}})

THERMOFUN_VOCABULARY(StandardPressure, "standard_pressure",
    {StandardPressure::OneBar, "1bar", {"1 bar", "bar"}}, {StandardPressure::OneAtm, "1atm", {"1 atm", "atm"}})

THERMOFUN_VOCABULARY(ConcentrationScale, "concentration_scale",
    {ConcentrationScale::Molality, "molality", {}}, {ConcentrationScale::MoleFraction, "mole_fraction", {}},
    {ConcentrationScale::Molarity, "molarity", {}})

THERMOFUN_VOCABULARY(WaterReference, "water_reference",
    {WaterReference::Geochemical, "geochemical", {}}, {WaterReference::SteamTables, "steam_tables", {}})

#undef THERMOFUN_VOCABULARY

/// Lower case, with '-' read as '_', so that "Steam-Tables" and "STEAM_TABLES" both match "steam_tables".
auto normalized(std::string_view s) -> std::string
{
    std::string out(s);
    for (auto& c : out)
        c = c == '-' ? '_' : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

auto editDistance(const std::string& a, const std::string& b) -> size_t
{
    std::vector<size_t> row(b.size() + 1);
    for (size_t j = 0; j <= b.size(); ++j)
        row[j] = j;
    for (size_t i = 1; i <= a.size(); ++i)
    {
        size_t diagonal = row[0];
        row[0] = i;
        for (size_t j = 1; j <= b.size(); ++j)
        {
            const size_t up = row[j];
            row[j] = std::min({row[j] + 1, row[j - 1] + 1, diagonal + (a[i - 1] == b[j - 1] ? 0 : 1)});
            diagonal = up;
        }
    }
    return row[b.size()];
}

} // namespace

template <class E>
auto toString(E value) -> std::string
{
    for (const auto& row : Table<E>::rows())
        if (row.value == value)
            return row.word;
    return "";
}

template <class E>
auto fromString(std::string_view word) -> std::optional<E>
{
    const auto w = normalized(word);
    for (const auto& row : Table<E>::rows())
    {
        if (w == normalized(row.word))
            return row.value;
        for (const char* alias : row.aliases)
            if (w == normalized(alias))
                return row.value;
    }
    return std::nullopt;
}

template <class E>
auto words() -> std::vector<std::string>
{
    std::vector<std::string> out;
    for (const auto& row : Table<E>::rows())
        out.push_back(row.word);
    return out;
}

template <class E>
auto nearestWord(std::string_view word) -> std::string
{
    const auto w = normalized(word);
    // a few typing errors, not another word; no guess for words of one or two letters
    if (w.size() < 3)
        return "";
    const size_t limit = std::max<size_t>(1, w.size() / 3);
    std::string best;
    size_t bestDistance = limit + 1;
    for (const auto& row : Table<E>::rows())
    {
        std::vector<const char*> candidates = row.aliases;
        candidates.insert(candidates.begin(), row.word);
        for (const char* candidate : candidates)
        {
            const auto d = editDistance(w, normalized(candidate));
            if (d < bestDistance)
            {
                bestDistance = d;
                best = row.word;
            }
        }
    }
    return best;
}

template <class E>
auto vocabularyName() -> std::string
{
    return Table<E>::name;
}

template <class E>
auto unknownWordMessage(std::string_view word) -> std::string
{
    std::string message = "\"" + std::string(word) + "\" is not a value of " + vocabularyName<E>() + ".";
    const auto near = nearestWord<E>(word);
    if (!near.empty())
        message += " Did you mean \"" + near + "\"?";
    message += " Allowed:";
    const auto all = words<E>();
    for (size_t i = 0; i < all.size(); ++i)
        message += (i ? ", " : " ") + all[i];
    return message + ".";
}

#define THERMOFUN_INSTANTIATE(E)                                                                                      \
    template auto toString<E>(E) -> std::string;                                                                      \
    template auto fromString<E>(std::string_view) -> std::optional<E>;                                                \
    template auto words<E>() -> std::vector<std::string>;                                                             \
    template auto nearestWord<E>(std::string_view) -> std::string;                                                    \
    template auto vocabularyName<E>() -> std::string;                                                                 \
    template auto unknownWordMessage<E>(std::string_view) -> std::string;

THERMOFUN_INSTANTIATE(PropertyName)
THERMOFUN_INSTANTIATE(ReactionPropertyName)
THERMOFUN_INSTANTIATE(Quality)
THERMOFUN_INSTANTIATE(Origin)
THERMOFUN_INSTANTIATE(PropertySource)
THERMOFUN_INSTANTIATE(DatasetStatus)
THERMOFUN_INSTANTIATE(MethodKind)
THERMOFUN_INSTANTIATE(OperatesOn)
THERMOFUN_INSTANTIATE(SubstanceClass)
THERMOFUN_INSTANTIATE(AggregateState)
THERMOFUN_INSTANTIATE(ElementClass)
THERMOFUN_INSTANTIATE(Interpolation)
THERMOFUN_INSTANTIATE(EnergyReference)
THERMOFUN_INSTANTIATE(StandardPressure)
THERMOFUN_INSTANTIATE(ConcentrationScale)
THERMOFUN_INSTANTIATE(WaterReference)

#undef THERMOFUN_INSTANTIATE

auto siUnit(PropertyName p) -> std::string
{
    switch (p)
    {
    case PropertyName::G: case PropertyName::H: case PropertyName::U: case PropertyName::A: return "J/mol";
    case PropertyName::S: case PropertyName::Cp: case PropertyName::Cv: return "J/(mol.K)";
    case PropertyName::V: return "m3/mol";
    }
    return "";
}

auto siUnit(ReactionPropertyName p) -> std::string
{
    switch (p)
    {
    case ReactionPropertyName::logK: case ReactionPropertyName::lnK: return "1";
    case ReactionPropertyName::dG: case ReactionPropertyName::dH: case ReactionPropertyName::dU:
    case ReactionPropertyName::dA: return "J/mol";
    case ReactionPropertyName::dS: case ReactionPropertyName::dCp: return "J/(mol.K)";
    case ReactionPropertyName::dV: return "m3/mol";
    }
    return "";
}

} // namespace ThermoFun::v2
