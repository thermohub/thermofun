// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef THERMOMODELREACTION_H
#define THERMOMODELREACTION_H
#include <ThermoFun/ThermoProperties.h>

#include <memory>
#include <string>
#include <vector>

// ThermoFun includes
#include <ThermoFun/GlobalVariables.h>

namespace ThermoFun {
class Reaction;

/**
 * @brief The ThermoModelsReaction class
 */
class ThermoModelsReaction
{
public:
    ThermoModelsReaction();

};

class ReactionDolejsManning10
{
public:
    /// Construct a default ReactionDolejsManning10 instance
    ReactionDolejsManning10();

    /// Construct a ReactionDolejsManning10 instance from a reaction instance
    explicit ReactionDolejsManning10(const Reaction& reaction);

    /// Returns the thermodynamic properties of the reaction.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    auto thermoProperties (double T, double P, PropertiesSolvent wp) -> ThermoPropertiesReaction;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class ReactionFrantzMarshall
{
public:
    /// Construct a default ReactionFrantzMarshall instance
    ReactionFrantzMarshall();

    /// Construct a ReactionFrantzMarshall instance from a reaction instance
    explicit ReactionFrantzMarshall(const Reaction& reaction);

    /// Returns the thermodynamic properties of the reaction.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    auto thermoProperties (double T, double P, PropertiesSolvent wp) -> ThermoPropertiesReaction;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class ReactionRyzhenkoBryzgalin
{
public:
    /// Construct a default ReactionRyzhenkoBryzgalin instance
    ReactionRyzhenkoBryzgalin();

    /// Construct a ReactionRyzhenkoBryzgalin instance from a reaction instance
    explicit ReactionRyzhenkoBryzgalin(const Reaction& reaction);

    /// Returns the thermodynamic properties of the reaction.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    auto thermoProperties (double T, double P, PropertiesSolvent wp) -> ThermoPropertiesReaction;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class Reaction_LogK_fT
{
public:
    /// Construct a default ReactionReaction_LogK_fT instance
    Reaction_LogK_fT();

    /// Construct a ReactionReaction_LogK_fT instance from a reaction instance
    explicit Reaction_LogK_fT(const Reaction& reaction);

    /// Returns the thermodynamic properties of the reaction.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    auto thermoProperties (double T, double P, MethodCorrT_Thrift::type methodT) -> ThermoPropertiesReaction;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class ReactionFromReactantsProperties
{
public:
    /// Construct a default ReactionReactionFromReactantsProperties instance
    ReactionFromReactantsProperties();

    /// Construct a ReactionReactionFromReactantsProperties instance from a reaction instance
    explicit ReactionFromReactantsProperties(const Reaction& reaction);

    /// Returns the thermodynamic properties of the reaction, calculated from the properties of its reactants.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param components The properties of the reactants with their stoichiometric coefficients
    /// @param symbols The symbols of the reactants (same order as the components)
    auto thermoProperties (double T, double P, const std::vector<std::pair<ThermoPropertiesSubstance, double>>& components, const std::vector<std::string>& symbols) -> ThermoPropertiesReaction;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class Reaction_Vol_fT
{
public:
    /// Construct a default ReactionReaction_Vol_fT instance
    Reaction_Vol_fT();

    /// Construct a ReactionReaction_Vol_fT instance from a reaction instance
    explicit Reaction_Vol_fT(const Reaction& reaction);

    /// Returns the properties of the reaction corrected for the reaction volume V(T,P).
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param tpr The properties of the reaction at the reference pressure
    auto thermoProperties (double T, double P, const ThermoPropertiesReaction& tpr) -> ThermoPropertiesReaction;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};



} // namespace ThermoFun

#endif // THERMOMODELREACTION_H
