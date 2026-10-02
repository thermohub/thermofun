#include "Common/Exception.h"
#include "ThermoProperties.h"
#include "ThermoModelsReaction.h"
#include "Reactions/FrantzMarshall.h"
#include "Reactions/RyzhenkoBryzgalyn.h"
#include "Reactions/LogK_function_of_T.h"
#include "Reactions/DolejsManning2010.h"
#include "Reactions/Volume_function_of_T.h"

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
        return thermoPropertiesDolejsManning2010(pass.T, pass.P / bar_to_Pa, pimpl->reaction, lift(pass, wp));
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

    setStatusLogK_fT(tpr, inputs, T, P);

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


auto ReactionFromReactantsProperties::thermoProperties(double T, double P, const std::vector<std::pair<ThermoPropertiesSubstance, double>>& components, const std::vector<std::string>& symbols) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;
    tpr.reaction_heat_capacity_cp = 0.0;
    tpr.reaction_gibbs_energy = 0.0;
    tpr.reaction_enthalpy = 0.0;
    tpr.reaction_entropy = 0.0;
    tpr.reaction_volume = 0.0;
    tpr.ln_equilibrium_constant = 0.0;
    tpr.log_equilibrium_constant = 0.0;
    tpr.reaction_heat_capacity_cv = 0.0;
    tpr.reaction_internal_energy = 0.0;
    tpr.reaction_helmholtz_energy = 0.0;

    const auto& reaction = pimpl->reaction;
    std::string message = "Calculated from the reaction components: " + reaction.symbol() + "; ";

        // the values and derivatives of the properties of the reaction
        if (!components.empty())
        {
            auto values = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
                ThermoPropertiesReactionAD a;
                a.reaction_heat_capacity_cp = 0.0;
                a.reaction_gibbs_energy = 0.0;
                a.reaction_enthalpy = 0.0;
                a.reaction_entropy = 0.0;
                a.reaction_volume = 0.0;

                for (const auto& component : components)
                {
                    const auto& tps = component.first;
                    const auto coeff = component.second;

                    a.reaction_heat_capacity_cp += pass(tps.heat_capacity_cp)*coeff;
                    a.reaction_gibbs_energy     += pass(tps.gibbs_energy)*coeff;
                    a.reaction_enthalpy         += pass(tps.enthalpy)*coeff;
                    a.reaction_entropy          += pass(tps.entropy)*coeff;
                    a.reaction_volume           += pass(tps.volume)*coeff;
                    a.reaction_heat_capacity_cv  = pass(tps.heat_capacity_cv)*coeff;
                    a.reaction_internal_energy   = pass(tps.internal_energy)*coeff;
                    a.reaction_helmholtz_energy  = pass(tps.helmholtz_energy)*coeff;
                }
                a.ln_equilibrium_constant  = a.reaction_gibbs_energy / -(R_CONSTANT*pass.T);
                a.log_equilibrium_constant = a.ln_equilibrium_constant * ln_to_lg;
                return a;
            });

            // the errors and statuses are set below
            auto copyValues = [](Reaktoro_::ThermoProperty& to, const Reaktoro_::ThermoProperty& from) {
                to.val = from.val; to.ddt = from.ddt; to.ddp = from.ddp;
            };
            copyValues(tpr.reaction_heat_capacity_cp, values.reaction_heat_capacity_cp);
            copyValues(tpr.reaction_gibbs_energy, values.reaction_gibbs_energy);
            copyValues(tpr.reaction_enthalpy, values.reaction_enthalpy);
            copyValues(tpr.reaction_entropy, values.reaction_entropy);
            copyValues(tpr.reaction_volume, values.reaction_volume);
            copyValues(tpr.ln_equilibrium_constant, values.ln_equilibrium_constant);
            copyValues(tpr.log_equilibrium_constant, values.log_equilibrium_constant);
            copyValues(tpr.reaction_heat_capacity_cv, values.reaction_heat_capacity_cv);
            copyValues(tpr.reaction_internal_energy, values.reaction_internal_energy);
            copyValues(tpr.reaction_helmholtz_energy, values.reaction_helmholtz_energy);
        }

        // the errors and statuses, and the messages of the properties that are not defined
        for (size_t i = 0; i < components.size(); ++i)
        {
            const auto& tps = components[i].first;
            const auto& substance = symbols[i];

            tpr.reaction_heat_capacity_cp.propagateFrom(tpr.reaction_heat_capacity_cp, tps.heat_capacity_cp);
            tpr.reaction_gibbs_energy.propagateFrom(tpr.reaction_gibbs_energy, tps.gibbs_energy);
            tpr.reaction_enthalpy.propagateFrom(tpr.reaction_enthalpy, tps.enthalpy);
            tpr.reaction_entropy.propagateFrom(tpr.reaction_entropy, tps.entropy);
            tpr.reaction_volume.propagateFrom(tpr.reaction_volume, tps.volume);
            tpr.ln_equilibrium_constant.propagateFrom(tpr.reaction_gibbs_energy);
            tpr.log_equilibrium_constant.propagateFrom(tpr.ln_equilibrium_constant);
            tpr.reaction_heat_capacity_cv.propagateFrom(tps.heat_capacity_cv);
            tpr.reaction_internal_energy.propagateFrom(tps.internal_energy);
            tpr.reaction_helmholtz_energy.propagateFrom(tps.helmholtz_energy);

            setMessage(tps.heat_capacity_cp.sta.first, "Cp of component " + substance, message+tps.heat_capacity_cp.sta.second, tpr.reaction_heat_capacity_cp.sta.second);
            setMessage(tps.gibbs_energy.sta.first,     "G0 of component " + substance, message+tps.gibbs_energy.sta.second,     tpr.reaction_gibbs_energy.sta.second);
            setMessage(tps.enthalpy.sta.first,         "H0 of component " + substance, message+tps.enthalpy.sta.second,         tpr.reaction_enthalpy.sta.second);
            setMessage(tps.entropy.sta.first,          "S0 of component " + substance, message+tps.entropy.sta.second,          tpr.reaction_entropy.sta.second);
            setMessage(tps.volume.sta.first,           "V0 of component " + substance, message+tps.volume.sta.second,           tpr.reaction_volume.sta.second);
            setMessage(tps.gibbs_energy.sta.first,     "G0 of component " + substance, message+tps.gibbs_energy.sta.second,     tpr.log_equilibrium_constant.sta.second);
            setMessage(tps.gibbs_energy.sta.first,     "G0 of component " + substance, message+tps.gibbs_energy.sta.second,     tpr.ln_equilibrium_constant.sta.second);
        }

    // the errors: first-order propagation of the errors of the reactants (independent), weighted by the
    // stoichiometric coefficients; ln K = -G/(R T)
    {
        double cp = 0, g = 0, h = 0, sv = 0, v = 0;
        for (const auto& component : components)
        {
            const auto& tps = component.first;
            const double nu = component.second;
            cp += nu*nu*tps.heat_capacity_cp.err*tps.heat_capacity_cp.err;
            g  += nu*nu*tps.gibbs_energy.err*tps.gibbs_energy.err;
            h  += nu*nu*tps.enthalpy.err*tps.enthalpy.err;
            sv += nu*nu*tps.entropy.err*tps.entropy.err;
            v  += nu*nu*tps.volume.err*tps.volume.err;
        }
        tpr.reaction_heat_capacity_cp.setError({{1.0, std::sqrt(cp)}});
        tpr.reaction_gibbs_energy.setError({{1.0, std::sqrt(g)}});
        tpr.reaction_enthalpy.setError({{1.0, std::sqrt(h)}});
        tpr.reaction_entropy.setError({{1.0, std::sqrt(sv)}});
        tpr.reaction_volume.setError({{1.0, std::sqrt(v)}});
        tpr.ln_equilibrium_constant.setError({{1.0/(R_CONSTANT*T), std::sqrt(g)}});
        tpr.log_equilibrium_constant.setError({{ln_to_lg, tpr.ln_equilibrium_constant.err}});
        if (!components.empty())
        {
            const auto& last = components.back();
            tpr.reaction_heat_capacity_cv.setError({{last.second, last.first.heat_capacity_cv.err}});
            tpr.reaction_internal_energy.setError({{last.second, last.first.internal_energy.err}});
            tpr.reaction_helmholtz_energy.setError({{last.second, last.first.helmholtz_energy.err}});
        }
    }

    return tpr;
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


auto Reaction_Vol_fT::thermoProperties(double T, double P, const ThermoPropertiesReaction& tprIn) -> ThermoPropertiesReaction
{
    // P in Pa, the model works with the pressure in bar
    const auto reaction = pimpl->reaction;
    auto tpr = tprIn;
    tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesReaction_Vol_fT(pass.T, pass.P / 1e5, reaction, lift(pass, tprIn));
    });

    auto keep = [](Reaktoro_::ThermoProperty& o, const Reaktoro_::ThermoProperty& i) { o.sta = i.sta; o.err = i.err; };
    keep(tpr.reaction_heat_capacity_cv, tprIn.reaction_heat_capacity_cv);
    tpr.reaction_volume.asCalculated();
    tpr.reaction_gibbs_energy.propagateFrom(tprIn.reaction_gibbs_energy, tpr.reaction_volume);
    tpr.reaction_enthalpy.propagateFrom(tprIn.reaction_enthalpy, tpr.reaction_volume);
    tpr.reaction_entropy.propagateFrom(tprIn.reaction_entropy, tpr.reaction_volume);
    tpr.reaction_heat_capacity_cp.propagateFrom(tprIn.reaction_heat_capacity_cp, tpr.reaction_volume);
    tpr.ln_equilibrium_constant.propagateFrom(tprIn.ln_equilibrium_constant, tpr.reaction_volume);
    tpr.log_equilibrium_constant.propagateFrom(tpr.ln_equilibrium_constant);
    tpr.reaction_internal_energy.propagateFrom(tpr.reaction_enthalpy, tpr.reaction_volume);
    tpr.reaction_helmholtz_energy.propagateFrom(tpr.reaction_internal_energy, tpr.reaction_entropy);

    // first-order propagation: the volume model has no uncertainty; the errors of the input are kept, U and A weighted
    const double Pbar = P / 1e5;
    auto keepErr = [](Reaktoro_::ThermoProperty& o, const Reaktoro_::ThermoProperty& i) { o.err = i.err; };
    keepErr(tpr.reaction_gibbs_energy, tprIn.reaction_gibbs_energy);
    keepErr(tpr.reaction_enthalpy, tprIn.reaction_enthalpy);
    keepErr(tpr.reaction_entropy, tprIn.reaction_entropy);
    keepErr(tpr.reaction_heat_capacity_cp, tprIn.reaction_heat_capacity_cp);
    keepErr(tpr.ln_equilibrium_constant, tprIn.ln_equilibrium_constant);
    keepErr(tpr.log_equilibrium_constant, tprIn.log_equilibrium_constant);
    tpr.reaction_internal_energy.setError({{1.0, tpr.reaction_enthalpy.err}, {Pbar, tpr.reaction_volume.err}});
    tpr.reaction_helmholtz_energy.setError({{1.0, tpr.reaction_internal_energy.err}, {T, tpr.reaction_entropy.err}});
    return tpr;
}

} // namespace ThermoFuns
