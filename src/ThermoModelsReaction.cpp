#include "ThermoProperties.h"
#include "ThermoModelsReaction.h"
#include "Reactions/FrantzMarshall.h"
#include "Reactions/RyzhenkoBryzgalyn.h"
#include "Reactions/LogK_function_of_T.h"

namespace TCorrPT {

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
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P);

    return thermoPropertiesFrantzMarshall(t, p, pimpl->reaction, wp);
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
    auto TK = Reaktoro_::Temperature(T + C_to_K);
    auto Pbar = Reaktoro_::Pressure(P);

    return thermoPropertiesRyzhenkoBryzgalin(TK, Pbar, pimpl->reaction, wp);
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
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P);

    return thermoPropertiesReaction_LogK_fT(t, p, pimpl->reaction, methodT);
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
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P);

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
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P);

//    return thermoPropertiesReaction_Vol_fT(t, p, pimpl->reaction);
}



} // namespace TCorrPTs
