// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "Common/Exception.h"
#include "ThermoProperties.h"
#include "ThermoModelsReaction.h"
#include "Reaction.h"
#include "ThermoParameters.h"
#include "GlobalVariables.h"
#include "Methods/Reactions/FrantzMarshall.h"
#include "Methods/Reactions/RyzhenkoBryzgalyn.h"
#include "Methods/Reactions/LogK_function_of_T.h"
#include "Methods/Reactions/DolejsManning2010.h"
#include "Methods/Reactions/Volume_function_of_T.h"
#include "Methods/Reactions/ReactionMethods.h"
#include "ThermoPropertiesStatus.h"

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


/// The inputs of the formulas of a reaction, from the reaction
static auto reactionInputsOf(const Reaction& reaction, const std::vector<double>& coefficients) -> ReactionInputs
{
    ReactionInputs in;
    in.reference = reaction.thermoReferenceProperties();
    in.Tr = reaction.referenceT();
    in.Pr = reaction.referenceP();
    in.coefficients.assign(coefficients.begin(), coefficients.end());
    return in;
}

static auto logKFormOf(MethodCorrT_Thrift::type method) -> LogKForm
{
    switch (method)
    {
    case MethodCorrT_Thrift::type::CTM_EK0: return LogKForm::Constant;
    case MethodCorrT_Thrift::type::CTM_EK1: return LogKForm::ZeroEntropy;
    case MethodCorrT_Thrift::type::CTM_EK2: return LogKForm::VantHoff;
    case MethodCorrT_Thrift::type::CTM_EK3: return LogKForm::ConstantCp;
    default: return LogKForm::Polynomial;
    }
}

auto ReactionDolejsManning10::thermoProperties(double T, double P, PropertiesSolvent wp) -> ThermoPropertiesReaction
{
    auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesDolejsManning2010(pass.T, pass.P / bar_to_Pa, reactionInputsOf(pimpl->reaction, pimpl->reaction.thermoParameters().reaction_DM10_coeff), lift(pass, wp));
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
        return thermoPropertiesFrantzMarshall(pass.T, pass.P / bar_to_Pa, reactionInputsOf(pimpl->reaction, pimpl->reaction.thermoParameters().reaction_FM_coeff), lift(pass, wp));
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
        return thermoPropertiesRyzhenkoBryzgalin(pass.T, pass.P / bar_to_Pa, reactionInputsOf(pimpl->reaction, pimpl->reaction.thermoParameters().reaction_RB_coeff), lift(pass, wp));
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


auto Reaction_LogK_fT::thermoProperties(double T, double P) -> ThermoPropertiesReaction
{
    return thermoProperties(T, P, MethodCorrT_Thrift::type::CTM_LGK);
}

auto Reaction_LogK_fT::thermoProperties(double T, double P, MethodCorrT_Thrift::type methodT) -> ThermoPropertiesReaction
{
    auto reaction = reactionInputsOf(pimpl->reaction, pimpl->reaction.thermoParameters().reaction_logK_fT_coeff);
    const auto& cp = pimpl->reaction.thermoParameters().reaction_Cp_fT_coeff;
    reaction.cp_coefficients.assign(cp.begin(), cp.end());
    return v2::logKProperties(reaction, logKFormOf(methodT), T, P);
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
                a.reaction_heat_capacity_cv = 0.0;
                a.reaction_internal_energy = 0.0;
                a.reaction_helmholtz_energy = 0.0;

                for (const auto& component : components)
                {
                    const auto& tps = component.first;
                    const auto coeff = component.second;

                    a.reaction_heat_capacity_cp += pass(tps.heat_capacity_cp)*coeff;
                    a.reaction_gibbs_energy     += pass(tps.gibbs_energy)*coeff;
                    a.reaction_enthalpy         += pass(tps.enthalpy)*coeff;
                    a.reaction_entropy          += pass(tps.entropy)*coeff;
                    a.reaction_volume           += pass(tps.volume)*coeff;
                    a.reaction_heat_capacity_cv += pass(tps.heat_capacity_cv)*coeff;
                    a.reaction_internal_energy  += pass(tps.internal_energy)*coeff;
                    a.reaction_helmholtz_energy += pass(tps.helmholtz_energy)*coeff;
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
            tpr.reaction_heat_capacity_cv.propagateFrom(tpr.reaction_heat_capacity_cv, tps.heat_capacity_cv);
            tpr.reaction_internal_energy.propagateFrom(tpr.reaction_internal_energy, tps.internal_energy);
            tpr.reaction_helmholtz_energy.propagateFrom(tpr.reaction_helmholtz_energy, tps.helmholtz_energy);

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
        double cp = 0, g = 0, h = 0, sv = 0, v = 0, cv = 0, u = 0, am = 0;
        for (const auto& component : components)
        {
            const auto& tps = component.first;
            const double nu = component.second;
            cp += nu*nu*tps.heat_capacity_cp.err*tps.heat_capacity_cp.err;
            g  += nu*nu*tps.gibbs_energy.err*tps.gibbs_energy.err;
            h  += nu*nu*tps.enthalpy.err*tps.enthalpy.err;
            sv += nu*nu*tps.entropy.err*tps.entropy.err;
            v  += nu*nu*tps.volume.err*tps.volume.err;
            cv += nu*nu*tps.heat_capacity_cv.err*tps.heat_capacity_cv.err;
            u  += nu*nu*tps.internal_energy.err*tps.internal_energy.err;
            am += nu*nu*tps.helmholtz_energy.err*tps.helmholtz_energy.err;
        }
        tpr.reaction_heat_capacity_cp.setError({{1.0, std::sqrt(cp)}});
        tpr.reaction_gibbs_energy.setError({{1.0, std::sqrt(g)}});
        tpr.reaction_enthalpy.setError({{1.0, std::sqrt(h)}});
        tpr.reaction_entropy.setError({{1.0, std::sqrt(sv)}});
        tpr.reaction_volume.setError({{1.0, std::sqrt(v)}});
        tpr.ln_equilibrium_constant.setError({{1.0/(R_CONSTANT*T), std::sqrt(g)}});
        tpr.log_equilibrium_constant.setError({{ln_to_lg, tpr.ln_equilibrium_constant.err}});
        tpr.reaction_heat_capacity_cv.setError({{1.0, std::sqrt(cv)}});
        tpr.reaction_internal_energy.setError({{1.0, std::sqrt(u)}});
        tpr.reaction_helmholtz_energy.setError({{1.0, std::sqrt(am)}});
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
    return v2::volumePolynomialProperties(reactionInputsOf(pimpl->reaction, pimpl->reaction.thermoParameters().reaction_V_fT_coeff), T, P, tprIn);
}

} // namespace ThermoFuns
