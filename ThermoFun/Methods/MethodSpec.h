// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_METHODSPEC_H
#define THERMOFUN_METHODS_METHODSPEC_H

#include <optional>
#include <string>
#include <vector>

#include <ThermoFun/Model/Vocabulary.h>

namespace ThermoFun::model {

/// What a method computes.
enum class MethodRole
{
    SubstanceProperties,  ///< G, H, S, Cp, V of a substance
    SolventProperties,    ///< equation of state of water: properties of the substance and of the solvent
    SolventElectro,       ///< dielectric model of water
    ReactionProperties    ///< logK, dG, dH, dS, dCp, dV of a reaction
};

/// What a method contributes to one property of the result that it receives (decision 0031, row 11: `contributes_to`).
enum class Contribution
{
    None,  ///< does not touch the property
    Sets,  ///< gives the whole value: replaces the property
    Adds,  ///< adds an increment to the value before it
    Uses   ///< uses the property of the incoming result as an input only
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
    bool model_parameter = false;  ///< a parameter of the model, the same for every record, built in (Methods/ParameterSets)
    bool used = true;         ///< false: a record can give it (legacy files do), but the formulas do not read it
};

/// What a substance method contributes to G, H, S, Cp and V. U, A and Cv are always computed at the end.
struct SubstanceContributions
{
    Contribution G = Contribution::None, H = Contribution::None, S = Contribution::None, Cp = Contribution::None, V = Contribution::None;
    bool base = false;       ///< makes the complete set of properties
    bool transform = false;  ///< uses the incoming result as input (any Uses)
    int rank = 0;            ///< order among the methods of the same group; the lowest runs first
};

/// What a reaction method contributes to logK, dG, dH, dS, dCp and dV.
/// What the result of a method depends on, beside its parameters. A correction for pressure is what P says.
enum class Dependence
{
    Unset,
    None,   ///< the same values at every T and P
    T,
    P,
    TP
};

struct ReactionContributions
{
    Contribution logK = Contribution::None, dG = Contribution::None, dH = Contribution::None, dS = Contribution::None, dCp = Contribution::None,
           dV = Contribution::None;
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

/// A term of a method (decision 0030): a named part of the contribution of the method to one property, for example the Born
/// term of G of HKF88.
struct TermSpec
{
    std::string name;
    std::string property;   ///< the property it is part of: "G", "H", "S", "Cp", "V"
    std::string si_unit;
    std::string description;
};

/// A variable of a method (decision 0030): a quantity of the model that the terms come from, not summed, for example the g
/// function of HKF88.
struct VariableSpec
{
    std::string name;
    std::string si_unit;
    std::string description;
};

/// All that ThermoFun knows about one built-in method, without its code and without any legacy name.
/// The range in which a model was published or tested (decision 0029): outside of it the value is calculated, with a note.
/// A bound that is not given is no limit. K and Pa.
struct ModelRange
{
    std::optional<double> t_min, t_max, p_min, p_max;
    std::string source;  ///< where the range is stated

    auto empty() const -> bool { return !t_min && !t_max && !p_min && !p_max; }
    auto holds(double T, double P) const -> bool
    {
        return (!t_min || T >= *t_min) && (!t_max || T <= *t_max) && (!p_min || P >= *p_min) && (!p_max || P <= *p_max);
    }
};

struct MethodSpec
{
    std::string id;                              ///< for example "HKF88"
    std::vector<std::string> aliases;            ///< other accepted ids; the legacy names are in IO/Legacy
    std::string title;                           ///< one line
    std::string summary;                         ///< what it does, one sentence for the reference page
    std::string citation;                        ///< the source of the model; empty if no publication defines it
    std::string group;                           ///< the section of the reference, for example "Minerals"
    std::string same_as;                         ///< the id of a method with the same calculation and parameters (a legacy name); empty if none
    OperatesOn operates_on = OperatesOn::Substance;
    std::vector<SubstanceClass> applies_to_classes;   ///< empty: any class
    MethodRole role = MethodRole::SubstanceProperties;
    Dependence depends_on = Dependence::Unset;
    SubstanceContributions contributes_to;          ///< substances
    ReactionContributions reaction_contributes_to;            ///< reactions
    MethodNeeds needs;
    std::optional<EnergyReference> native_convention; ///< empty: that of the reference values
    bool piecewise = false;                      ///< the parameters come in temperature segments
    bool regional = false;                       ///< used only in its own region (decision 0028): the next base method of the record elsewhere
    ModelRange range;                            ///< the published range of the model: a note outside of it (decision 0029)
    bool implemented = true;                     ///< false: the id is known, but there is no code
    std::vector<ParameterSpec> parameters;
    std::vector<TermSpec> terms;
    std::vector<VariableSpec> variables;
    std::vector<std::string> implementations;    ///< for example {"reaktoro", "gems"}; the first is the default
};

/// True if methods of this kind can be combined by region (decision 0028): they give the value directly at each T and P,
/// so the method may change from point to point. Water equations of state and dielectric models; not the methods of a
/// substance, whose values are integrated from the reference state.
inline auto combinableByRegion(const MethodSpec& spec) -> bool
{
    return spec.role == MethodRole::SolventProperties || spec.role == MethodRole::SolventElectro;
}

} // namespace ThermoFun::model

#endif
