#include "ThermoModelsReaction.h"

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


auto ReactionFrantzMarshall::thermoProperties(double T, double P) -> ThermoPropertiesReaction
{
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

//    return thermoPropertiesFrantzMarshall(t, p, pimpl->reaction);
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


auto ReactionRyzhenkoBryzgalin::thermoProperties(double T, double P) -> ThermoPropertiesReaction
{
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

//    return thermoPropertiesRyzhenkoBryzgalin(t, p, pimpl->reaction);
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


auto Reaction_LogK_fT::thermoProperties(double T, double P) -> ThermoPropertiesReaction
{
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

//    return thermoPropertiesReaction_LogK_fT(t, p, pimpl->reaction);
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
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

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
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

//    return thermoPropertiesFrantzMarshall(t, p, pimpl->reaction);
}



} // namespace TCorrPTs
