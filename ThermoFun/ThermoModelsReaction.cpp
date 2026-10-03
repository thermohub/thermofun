#include "ThermoProperties.h"
#include "ThermoModelsReaction.h"
#include "Reactions/FrantzMarshall.h"
#include "Reactions/RyzhenkoBryzgalyn.h"
#include "Reactions/LogK_function_of_T.h"
#include "Reactions/DolejsManning2010.h"

namespace ThermoFun {

//=======================================================================================================
// Dolejs and Manning (2010)
//
// Added: DM 09.07.2019
//=======================================================================================================
struct ReactionDolejsManning10::Impl
{
    /// the substance instance
   Reaction reaction;

   Impl()
   {}

   Impl(const Reaction& reaction)
   : reaction(reaction)
   {}
};

ReactionDolejsManning10::ReactionDolejsManning10(const Reaction &reaction)
: pimpl(new Impl(reaction))
{}


auto ReactionDolejsManning10::thermoProperties(double T, double P, PropertiesSolvent wp) -> ThermoPropertiesReaction
{
    auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesFrantzMarshall(pass.T, pass.P / bar_to_Pa, pimpl->reaction, lift(pass, wp));
    });

    // the logarithm of the equilibrium constant and the isochoric heat capacity are not calculated
    tpr.log_equilibrium_constant = Reaktoro_::ThermoProperty();
    tpr.reaction_heat_capacity_cv = Reaktoro_::ThermoProperty();

    return tpr;
}

//=======================================================================================================
//
//
// Added: DM 09.11.2016
//=======================================================================================================
struct ReactionFrantzMarshall::Impl
{
    /// the substance instance
   Reaction reaction;

   Impl()
   {}

   Impl(const Reaction& reaction)
   : reaction(reaction)
   {}
};

ReactionFrantzMarshall::ReactionFrantzMarshall(const Reaction &reaction)
: pimpl(new Impl(reaction))
{}


auto ReactionFrantzMarshall::thermoProperties(double T, double P, PropertiesSolvent wp) -> ThermoPropertiesReaction
{
    auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesFrantzMarshall(pass.T, pass.P / bar_to_Pa, pimpl->reaction, lift(pass, wp));
    });

    // the logarithm of the equilibrium constant and the isochoric heat capacity are not calculated
    tpr.log_equilibrium_constant = Reaktoro_::ThermoProperty();
    tpr.reaction_heat_capacity_cv = Reaktoro_::ThermoProperty();

    return tpr;
}

//=======================================================================================================
//
//
// Added: DM 09.11.2016
//=======================================================================================================
struct ReactionRyzhenkoBryzgalin::Impl
{
    /// the substance instance
   Reaction reaction;

   Impl()
   {}

   Impl(const Reaction& reaction)
   : reaction(reaction)
   {}
};

ReactionRyzhenkoBryzgalin::ReactionRyzhenkoBryzgalin(const Reaction &reaction)
: pimpl(new Impl(reaction))
{}


auto ReactionRyzhenkoBryzgalin::thermoProperties(double T, double P, PropertiesSolvent wp) -> ThermoPropertiesReaction
{
    auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesRyzhenkoBryzgalin(pass.T, pass.P / bar_to_Pa, pimpl->reaction, lift(pass, wp));
    });

    // the isochoric heat capacity is not calculated
    tpr.reaction_heat_capacity_cv = Reaktoro_::ThermoProperty();

    return tpr;
}

//=======================================================================================================
//
//
// Added: DM 09.11.2016
//=======================================================================================================
struct Reaction_LogK_fT::Impl
{
    /// the substance instance
   Reaction reaction;

   Impl()
   {}

   Impl(const Reaction& reaction)
   : reaction(reaction)
   {}
};

Reaction_LogK_fT::Reaction_LogK_fT(const Reaction &reaction)
: pimpl(new Impl(reaction))
{}


auto Reaction_LogK_fT::thermoProperties(double T, double P, MethodCorrT_Thrift::type methodT) -> ThermoPropertiesReaction
{
    const auto inputs = prepareLogK_fT(pimpl->reaction, T, methodT);

    auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesReaction_LogK_fT(pass.T, pass.P / bar_to_Pa, pimpl->reaction, inputs.method);
    });

    setStatusLogK_fT(tpr, inputs);

    return tpr;
}

//=======================================================================================================
//
//
// Added: DM 09.11.2016
//=======================================================================================================
struct ReactionFromReactantsProperties::Impl
{
    /// the substance instance
   Reaction reaction;

   Impl()
   {}

   Impl(const Reaction& reaction)
   : reaction(reaction)
   {}
};

ReactionFromReactantsProperties::ReactionFromReactantsProperties(const Reaction &reaction)
: pimpl(new Impl(reaction))
{}


auto ReactionFromReactantsProperties::thermoProperties(double T, double P) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;
    return tpr;

//    return thermoPropertiesFromReactantsProperties(t, p, pimpl->reaction);
}

//=======================================================================================================
//
//
// Added: DM 09.11.2016
//=======================================================================================================
struct Reaction_Vol_fT::Impl
{
    /// the substance instance
   Reaction reaction;

   Impl()
   {}

   Impl(const Reaction& reaction)
   : reaction(reaction)
   {}
};

Reaction_Vol_fT::Reaction_Vol_fT(const Reaction &reaction)
: pimpl(new Impl(reaction))
{}


auto Reaction_Vol_fT::thermoProperties(double T, double P) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;
    return tpr;

//    return thermoPropertiesReaction_Vol_fT(t, p, pimpl->reaction);
}



} // namespace ThermoFuns
