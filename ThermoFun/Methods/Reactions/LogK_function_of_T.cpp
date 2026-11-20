// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "LogK_function_of_T.h"
#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/Exception.h>
#include <algorithm>

namespace ThermoFun {

LogKForm determineMethod(const Reaktoro_::ThermoProperty& dGr,
                                         const Reaktoro_::ThermoProperty& dHr,
                                         const Reaktoro_::ThermoProperty& dSr,
                                         const Reaktoro_::ThermoProperty& dCpr)
{
    using Status = Reaktoro_::Status;

    const bool has_dHr  = dHr.sta.first  != Status::notdefined;
    const bool has_dSr  = dSr.sta.first  != Status::notdefined;
    const bool has_dCpr = dCpr.sta.first != Status::notdefined;

    // --- EK3: any of dCpr, dHr, dSr defined ---
    if (has_dCpr && has_dHr && has_dSr)
    {
        return LogKForm::ConstantCp;
    }

    // --- EK2: both dHr and dSr defined ---
    if (has_dHr && has_dSr)
    {
        return LogKForm::VantHoff;
    }

    // --- EK1: one defined AND that one equals zero ---
    if ((has_dHr && dHr.val == 0.0) ||
        (has_dSr && dSr.val == 0.0))
    {
        return LogKForm::ZeroEntropy;
    }

    // --- EK0: none defined ---
    return LogKForm::Constant;
}

auto prepareLogK_fT(const ReactionInputs& reaction, double T, LogKForm CE) -> LogKInputs
{
    using Status = Reaktoro_::Status;

    const auto& ref_tpr = reaction.reference;
    LogKInputs in;
    in.dVr  = ref_tpr.reaction_volume;
    in.dHr  = ref_tpr.reaction_enthalpy;
    in.dSr  = ref_tpr.reaction_entropy;
    in.dCpr = ref_tpr.reaction_heat_capacity_cp;
    in.dGr  = ref_tpr.reaction_gibbs_energy;
    in.lgK  = ref_tpr.log_equilibrium_constant;

    // The reference values at Tr, completed (review of v2, B2): dG from log K or log K from dG; then a missing dH or dS
    // from the other two at Tr. Before, dG was not derived from log K, and dH or dS were filled at the T of the call.
    const double Tr = reaction.Tr;
    const double RTrln10 = R_CONSTANT * Tr * lg_to_ln;
    const bool has_lgK = in.lgK.sta.first != Status::notdefined;
    bool has_dGr  = in.dGr.sta.first  != Status::notdefined;
    if (has_lgK && !has_dGr)
    {
        in.dGr.val = -RTrln10 * in.lgK.val;
        in.dGr.propagateFrom(in.lgK);
        in.dGr.setError({{RTrln10, in.lgK.err}});
        has_dGr = true;
    }
    else if (has_dGr && !has_lgK)
    {
        in.lgK.val = -in.dGr.val / RTrln10;
        in.lgK.propagateFrom(in.dGr);
        in.lgK.setError({{1.0 / RTrln10, in.dGr.err}});
    }
    const bool has_dHr  = in.dHr.sta.first  != Status::notdefined;
    const bool has_dSr  = in.dSr.sta.first  != Status::notdefined;

    if(has_dSr && has_dGr && !has_dHr)
    {
        in.dHr.val = in.dGr.val + in.dSr.val * Tr;
        in.dHr.propagateFrom(in.dGr, in.dSr);
        in.dHr.setError({{1.0, in.dGr.err}, {Tr, in.dSr.err}});
    }
    if(has_dHr && has_dGr && !has_dSr)
    {
        in.dSr.val = (in.dHr.val - in.dGr.val) / Tr;
        in.dSr.propagateFrom(in.dHr, in.dGr);
        in.dSr.setError({{1.0/Tr, in.dHr.err}, {1.0/Tr, in.dGr.err}});
    }
    (void)T;

    if (CE == LogKForm::ConstantCp)
        CE = determineMethod(in.dGr, in.dHr, in.dSr, in.dCpr);
    // van 't Hoff needs dH and dS (one of them and log K or dG give the other): without them log K is constant, as in the
    // rule of GEMS; before, the missing values were 0 and log K was 0 (validate warns, V2-MTH-024)
    if (CE == LogKForm::VantHoff && (in.dHr.sta.first == Status::notdefined || in.dSr.sta.first == Status::notdefined))
        CE = LogKForm::Constant;

    in.method = CE;
    return in;
}

auto setStatusLogK_fT(ThermoPropertiesReaction& tpr, const LogKInputs& in, double T, double P) -> void
{
    Reaktoro_::ThermoProperty H = in.dHr, S = in.dSr, G = in.dGr, Cp = in.dCpr, V = in.dVr, Lg = in.lgK;

    switch (in.method)
    {
        case LogKForm::Constant: // lgK is the reference value
            break;
        case LogKForm::ZeroEntropy:
            H = G;
            Lg.propagateFrom(H);
            break;
        case LogKForm::VantHoff:
            Lg.propagateFrom(S, H);
            break;
        case LogKForm::ConstantCp:
            Lg.propagateFrom(S, H, Cp);
            S.propagateFrom(S, Cp);
            H.propagateFrom(H, Cp);
            break;
        case LogKForm::Polynomial:
            Lg.asCalculated();
            H.asCalculated();
            S.asCalculated();
            Cp.asCalculated();
            break;
        default:
            ;
    }

    G.propagateFrom(Lg);
    G.setError({{R_CONSTANT*T*lg_to_ln, Lg.err}});

    Reaktoro_::ThermoProperty U, A; // not defined
    if (tpr.reaction_enthalpy.val != 0)
    {
        U.propagateFrom(H, V);
        U.setError({{1.0, H.err}, {P/bar_to_Pa, V.err}});
        A.propagateFrom(U, S);
        A.setError({{1.0, U.err}, {T, S.err}});
    }

    auto set = [](Reaktoro_::ThermoProperty& to, const Reaktoro_::ThermoProperty& from) { to.sta = from.sta; to.err = from.err; };
    set(tpr.log_equilibrium_constant, Lg);
    tpr.ln_equilibrium_constant.propagateFrom(Lg);
    tpr.ln_equilibrium_constant.setError({{lg_to_ln, Lg.err}});
    set(tpr.reaction_gibbs_energy, G);
    set(tpr.reaction_enthalpy, H);
    set(tpr.reaction_entropy, S);
    set(tpr.reaction_heat_capacity_cp, Cp);
    set(tpr.reaction_volume, V);
    set(tpr.reaction_helmholtz_energy, A);
    set(tpr.reaction_internal_energy, U);
    tpr.reaction_heat_capacity_cv = Reaktoro_::ThermoProperty(); // not calculated
}

auto thermoPropertiesReaction_LogK_fT(real TK, real Pbar, const ReactionInputs& reaction, LogKForm CE) -> ThermoPropertiesReactionAD
{
    ThermoPropertiesReactionAD tpr;
    using Status = Reaktoro_::Status;

    auto Rln10   = R_CONSTANT * lg_to_ln;
    auto R_T     = TK * R_CONSTANT;
    const auto& ref_tpr = reaction.reference;
    auto A       = reaction.coefficients;
    auto CpCoeff = reaction.cp_coefficients;
    // the reference properties are constants
    real dVr     = seeded(ref_tpr.reaction_volume, reaction.seed.dV);   //Gr = rc[q].Gs[0];
    real dHr     = seeded(ref_tpr.reaction_enthalpy, reaction.seed.dH);
    real dSr     = seeded(ref_tpr.reaction_entropy, reaction.seed.dS);
    real dCpr    = seeded(ref_tpr.reaction_heat_capacity_cp, reaction.seed.dCp);
    real dGr     = seeded(ref_tpr.reaction_gibbs_energy, reaction.seed.dG);
    real lgK     = seeded(ref_tpr.log_equilibrium_constant, reaction.seed.logK);
    auto Tr      = reaction.Tr;


    const bool has_dHr  = ref_tpr.reaction_enthalpy.sta.first  != Status::notdefined;
    const bool has_dSr  = ref_tpr.reaction_entropy.sta.first  != Status::notdefined;
    const bool has_lgK  = ref_tpr.log_equilibrium_constant.sta.first != Status::notdefined;
    bool has_dGr  = ref_tpr.reaction_gibbs_energy.sta.first != Status::notdefined;

    // the reference values at Tr, completed as in prepareLogK_fT (autodiff numbers: the derivatives by the parameters follow)
    if (has_lgK && !has_dGr)
    {
        dGr = -Rln10 * Tr * lgK;
        has_dGr = true;
    }
    if(has_dSr && has_dGr && !has_dHr)
        dHr = dGr + dSr * Tr;
    if(has_dHr && has_dGr && !has_dSr)
        dSr = (dHr-dGr)/Tr;

    // the method CE was determined from the reference properties (see prepareLogK_fT)

    /// deal with Cp and A parameters conversion
    switch (CE)
    {
        case LogKForm::Constant: // 1-term lgK = const
        // lgK = A[0];
        // dCpr = 0.0;
        // dHr  = 0.0;
            break;
        case LogKForm::ZeroEntropy: // 1-term dGr = const
                // lgK = A[2]/T;
            // dCpr = 0.0;
            // dSr  = 0.0;
            dHr = dGr;
                // dGr_d = dHr;
            lgK  = - dHr / TK / Rln10;
          break;
        case LogKForm::VantHoff:  // 2-term or 1-term lgK=const at dHr=0
                // lgK = A[0] + A[2]/T;
            // dCpr = 0.0;
                // dGr_d = dHr - dSr * T;
            lgK  = (dSr - dHr/TK ) / Rln10;
          break;
        case LogKForm::ConstantCp:  // 3-term
                // lgK = A[0] + A[2]/T + A[3] * lnT;
            lgK  = ( dSr - dHr/TK - dCpr * ( 1 - Tr/TK - log( TK/Tr ))) / Rln10;
            dSr += dCpr * log( TK / Tr );
            dHr += dCpr * (TK - Tr );
                // dGr_d = dHr - dSr * T;
          break;
       case LogKForm::Polynomial:  // (derived from dCp=f(T))
            if (A.size() == 0 && CpCoeff.size() > 0)
            {
                //from Cp to A
                A.resize(7, 0.0);
                CpCoeff.resize(5, 0.0);

                // calculation of logK=f(T) coeffs (only first 5 Cp coefficients, conforming to Haas-Fisher function)
                // the integration constants are fixed at Tr by dS and dH there (review of v2, B2: they were taken at the
                // T of the call, and the terms of dCp then cancelled)
                A[0] = (( dSr - CpCoeff[0] - CpCoeff[0]*log(Tr) - CpCoeff[1]*Tr + CpCoeff[2]/(2.0*Tr*Tr)
                              + 2.0*CpCoeff[3]/pow(Tr,0.5) - CpCoeff[4]*Tr*Tr/2.0 ) / Rln10);
                A[1] = CpCoeff[1]/(2.0*Rln10);
                A[2] = (-( dHr - CpCoeff[0]*Tr - CpCoeff[1]*Tr*Tr/2.0 + CpCoeff[2]/Tr
                           - 2.0*CpCoeff[3]*pow(Tr,0.5) - CpCoeff[4]*Tr*Tr*Tr/3.0 ) / Rln10);
                A[3] = CpCoeff[0]/Rln10;
                A[4] = CpCoeff[2]/(2.0*Rln10);
                A[5] = CpCoeff[4]/(6.0*Rln10);
                A[6] = -4.0*CpCoeff[3]/Rln10;
            }

            if (CpCoeff.size() == 0 && A.size() > 0)
            {
                //from A to Cp
                A.resize(7, 0.0);
                CpCoeff.resize(5, 0.0);

                CpCoeff[0] = Rln10 * A[3];
                CpCoeff[1] = Rln10 * 2.0 * A[1];
                CpCoeff[2] = Rln10 * 2.0 * A[4];
                CpCoeff[3] = -Rln10 * 0.25 * A[6];
                CpCoeff[4] = Rln10 * 6.0 * A[5];
            }

            if (A.empty())
                funError("Cannot calculate log K of a reaction", "It has neither the coefficients of log K nor those of its heat capacity.", __LINE__, __FILE__);
            // a record can leave out the last coefficients, if they are 0
            if (A.size() < 7)
                A.resize(7, 0.0);
            if (CpCoeff.size() < 5)
                CpCoeff.resize(5, 0.0);

            lgK = A[0] + A[1] * TK + A[2]/TK + A[3] * log(TK) + A[4] / (TK*TK) +
                  A[5] * (TK*TK) + A[6] / pow(TK,0.5);
            dHr = Rln10 *( A[1]*(TK*TK) - A[2] + A[3]*TK - 2.0*A[4]/TK
                     + 2.0*A[5]*(TK*TK*TK) - 0.5*A[6]*pow(TK,0.5) );
            dSr = Rln10 * ( A[0] + 2.0*A[1]*TK + A[3]*(1.0+log(TK)) -
                       A[4]/(TK*TK) + 3.0*A[5]*(TK*TK) + 0.5*A[6]/pow(TK,0.5) );
                // dGr_d = dHr - dSr * T;
                // if( rc[q].DCp )
            dCpr = CpCoeff[0] + CpCoeff[1]*TK + CpCoeff[2]/(TK*TK) + CpCoeff[4]*(TK*TK) + CpCoeff[3]/pow(TK,0.5);
                // dHr = Rln10 * T_2 * ( A[1] - A[2]/T_2 + A[3]/T -
                // 2.0*A[4]/T_3 + 2.0*A[5]*T - 0.5*A[6]/T_15 );
                // dSr = Rln10 * ( A[0] + 2.0*A[1]*T + A[3]*(1.0+lnT) -
                // A[4]/T_2 + 3.0*A[5]*T_2 + 0.5*A[6]/T_05 );
          break;
       default:
       ; // error message ?
    }
    // Calculation of dGr
    dGr  = -R_T * lgK * lg_to_ln;
//    if (dHr.val() == 0) dHr = dGr + dSr * TK;
    real dUr, dAr;
    if (dHr.val() != 0)
    {
        dUr  = dHr - Pbar*dVr;
        dAr  = dUr - TK*dSr;
    }

    // Loading output data
    tpr.log_equilibrium_constant   = lgK;
    tpr.ln_equilibrium_constant    = lgK*lg_to_ln;
    tpr.reaction_gibbs_energy      = dGr;
    tpr.reaction_enthalpy          = dHr;
    tpr.reaction_entropy           = dSr;
    tpr.reaction_heat_capacity_cp  = dCpr;
    tpr.reaction_volume            = dVr;
    tpr.reaction_helmholtz_energy  = dAr;
    tpr.reaction_internal_energy   = dUr;

    return tpr;
}

}
