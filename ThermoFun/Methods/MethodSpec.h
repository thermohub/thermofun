// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_METHODSPEC_H
#define THERMOFUN_METHODS_METHODSPEC_H

#include <optional>
#include <string>
#include <vector>

#include <ThermoFun/Model/Vocabulary.h>

namespace ThermoFun::v2 {

/// What a method computes.
enum class MethodRole
{
    SubstanceProperties,  ///< G, H, S, Cp, V of a substance
    SolventProperties,    ///< equation of state of water: properties of the substance and of the solvent
    SolventElectro,       ///< dielectric model of water
    ReactionProperties    ///< logK, dG, dH, dS, dCp, dV of a reaction
};

/// What a method does to one property of the result that it receives.
enum class Effect
{
    None,  ///< does not touch the property
    Set,   ///< replaces the property
    Add,   ///< adds an increment to the property
    Read   ///< uses the property of the incoming result as an input
};

/// One named parameter of a method.
struct ParameterSpec
{
    std::string name;         ///< as in the source paper, for example "a1"
    std::string si_unit;      ///< the unit of the stored value, in UCUM, for example "J/(mol.Pa)"; "1" if none
    std::string source_unit;  ///< the unit of the source paper and of the formulas, for example "cal/(mol.bar)"
    std::string description;  ///< one line
    bool required = true;
    std::optional<double> default_si;
    bool fittable = true;     ///< false for integers and switches, for example a number of atoms
};

/// The effects of a substance method on G, H, S, Cp and V. U, A and Cv are always computed at the end.
struct MethodEffects
{
    Effect G = Effect::None, H = Effect::None, S = Effect::None, Cp = Effect::None, V = Effect::None;
    bool base = false;       ///< makes the complete set of properties
    bool transform = false;  ///< uses the incoming result as input (any Read effect)
    int rank = 0;            ///< order among the methods of the same group; the lowest runs first
};

/// The effects of a reaction method on logK, dG, dH, dS, dCp and dV.
struct ReactionEffects
{
    Effect logK = Effect::None, dG = Effect::None, dH = Effect::None, dS = Effect::None, dCp = Effect::None,
           dV = Effect::None;
    bool base = false;
    bool final = false;  ///< no step runs after this method
    int rank = 0;
};

/// What a method needs from outside its record.
struct MethodNeeds
{
    bool solvent = false;               ///< the state of the solvent at T, P
    bool dielectric = false;            ///< with its dielectric constant and Born functions
    bool solvent_at_reference = false;  ///< the state of the solvent at the reference T, P
    bool ideal_gas_water = false;       ///< the properties of water as an ideal gas
    bool element_entropies = false;     ///< the entropies of the elements, for a conversion of conventions
};

/// A named output of a method beside the standard properties, for example the density of water.
struct AuxiliaryOutputSpec
{
    std::string name;
    std::string si_unit;
    std::string description;
};

/// All that ThermoFun knows about one built-in method, without its code and without any legacy name.
struct MethodSpec
{
    std::string id;                              ///< for example "HKF88"
    std::vector<std::string> aliases;            ///< other accepted ids; the legacy names are in IO/Legacy
    std::string title;                           ///< one line
    std::string summary;                         ///< what it does, one sentence for the reference page
    std::string citation;                        ///< the source of the model; empty if no publication defines it
    OperatesOn operates_on = OperatesOn::Substance;
    std::vector<SubstanceClass> applies_to_classes;   ///< empty: any class
    MethodRole role = MethodRole::SubstanceProperties;
    MethodEffects effects;                       ///< substances
    ReactionEffects reaction_effects;            ///< reactions
    MethodNeeds needs;
    std::optional<EnergyReference> native_convention; ///< empty: that of the reference values
    bool piecewise = false;                      ///< the parameters come in temperature segments
    bool implemented = true;                     ///< false: the id is known, but there is no code
    std::vector<ParameterSpec> parameters;
    std::vector<AuxiliaryOutputSpec> auxiliary_outputs;
    std::vector<std::string> implementations;    ///< for example {"reaktoro", "gems"}; the first is the default
};

} // namespace ThermoFun::v2

#endif
