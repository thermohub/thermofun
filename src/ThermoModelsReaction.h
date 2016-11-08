#ifndef THERMOMODELREACTION_H
#define THERMOMODELREACTION_H

// TCorrPT includes
#include "Reaction.h"
#include "ThermoProperties.h"
#include <memory>

namespace TCorrPT {

//class Reaction;
//class ThermoPropertiesReaction;

/**
 * @brief The ThermoModelsReaction class
 */
class ThermoModelsReaction
{
public:
    ThermoModelsReaction();

};

class ReactionFrantzMarshall
{
public:
    /// Construct a default ReactionFrantzMarshall instance
    ReactionFrantzMarshall();

    /// Construct a ReactionFrantzMarshall instance from a reaction instance
    explicit ReactionFrantzMarshall(const Reaction& reaction);

    /// Returns the thermodynamic properties of the reaction.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P) -> ThermoPropertiesReaction;

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
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P) -> ThermoPropertiesReaction;

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
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P) -> ThermoPropertiesReaction;

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

    /// Returns the thermodynamic properties of the reaction.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P) -> ThermoPropertiesReaction;

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

    /// Returns the thermodynamic properties of the reaction.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P) -> ThermoPropertiesReaction;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};



} // namespace TCorrPT

#endif // THERMOMODELREACTION_H
