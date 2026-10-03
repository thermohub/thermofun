// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "LogK_function_of_T.h"
#include "ThermoParameters.h"
#include "Reaction.h"
#include <algorithm>

namespace ThermoFun {

MethodCorrT_Thrift::type determineMethod(const Reaktoro_::ThermoProperty& dGr,
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
        return MethodCorrT_Thrift::type::CTM_EK3;
    }

    // --- EK2: both dHr and dSr defined ---
    if (has_dHr && has_dSr)
    {
        return MethodCorrT_Thrift::type::CTM_EK2;
    }

    // --- EK1: one defined AND that one equals zero ---
    if ((has_dHr && dHr.val == 0.0) ||
        (has_dSr && dSr.val == 0.0))
    {
        return MethodCorrT_Thrift::type::CTM_EK1;
    }

    // --- EK0: none defined ---
    return MethodCorrT_Thrift::type::CTM_EK0;
}

auto prepareLogK_fT(Reaction reaction, double T, MethodCorrT_Thrift::type CE) -> LogKInputs
{
    using Status = Reaktoro_::Status;

    auto ref_tpr = reaction.thermoReferenceProperties();
    LogKInputs in;
    in.dVr  = ref_tpr.reaction_volume;
    in.dHr  = ref_tpr.reaction_enthalpy;
    in.dSr  = ref_tpr.reaction_entropy;
    in.dCpr = ref_tpr.reaction_heat_capacity_cp;
    in.dGr  = ref_tpr.reaction_gibbs_energy;
    in.lgK  = ref_tpr.log_equilibrium_constant;

    const bool has_dHr  = in.dHr.sta.first  != Status::notdefined;
    const bool has_dSr  = in.dSr.sta.first  != Status::notdefined;
    const bool has_dGr  = in.dGr.sta.first  != Status::notdefined;

    if(has_dSr && has_dGr && !has_dHr)
    {
        in.dHr.val = in.dGr.val + in.dSr.val * T;
        in.dHr.propagateFrom(in.dGr, in.dSr);
        in.dHr.setError({{1.0, in.dGr.err}, {T, in.dSr.err}});
    }
    if(has_dHr && has_dGr && !has_dSr)
    {
        in.dSr.val = (in.dHr.val - in.dGr.val) / T;
        in.dSr.propagateFrom(in.dHr, in.dGr);
        in.dSr.setError({{1.0/T, in.dHr.err}, {1.0/T, in.dGr.err}});
    }

    if (CE == MethodCorrT_Thrift::type::CTM_EK3)
        CE = determineMethod(in.dGr, in.dHr, in.dSr, in.dCpr);

    in.method = CE;
    return in;
}

auto setStatusLogK_fT(ThermoPropertiesReaction& tpr, const LogKInputs& in, double T, double P) -> void
{
    Reaktoro_::ThermoProperty H = in.dHr, S = in.dSr, G = in.dGr, Cp = in.dCpr, V = in.dVr, Lg = in.lgK;

    switch (in.method)
    {
        case MethodCorrT_Thrift::type::CTM_EK0: // lgK is the reference value
            break;
        case MethodCorrT_Thrift::type::CTM_EK1:
            H = G;
            Lg.propagateFrom(H);
            break;
        case MethodCorrT_Thrift::type::CTM_EK2:
            Lg.propagateFrom(S, H);
            break;
        case MethodCorrT_Thrift::type::CTM_EK3:
            Lg.propagateFrom(S, H, Cp);
            S.propagateFrom(S, Cp);
            H.propagateFrom(H, Cp);
            break;
        case MethodCorrT_Thrift::type::CTM_LGK:
        case MethodCorrT_Thrift::type::CTM_LGX:
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

auto thermoPropertiesReaction_LogK_fT(real TK, real Pbar, Reaction reaction, MethodCorrT_Thrift::type CE) -> ThermoPropertiesReactionAD
{
    ThermoPropertiesReactionAD tpr;
    using Status = Reaktoro_::Status;

    auto Rln10   = R_CONSTANT * lg_to_ln;
    auto R_T     = TK * R_CONSTANT;
    auto ref_tpr = reaction.thermoReferenceProperties();
    auto A       = reaction.thermoParameters().reaction_logK_fT_coeff;
    auto CpCoeff = reaction.thermoParameters().reaction_Cp_fT_coeff;
    // the reference properties are constants
    real dVr     = Reaktoro_::constant(ref_tpr.reaction_volume);   //Gr = rc[q].Gs[0];
    real dHr     = Reaktoro_::constant(ref_tpr.reaction_enthalpy);
    real dSr     = Reaktoro_::constant(ref_tpr.reaction_entropy);
    real dCpr    = Reaktoro_::constant(ref_tpr.reaction_heat_capacity_cp);
    real dGr     = Reaktoro_::constant(ref_tpr.reaction_gibbs_energy);
    real lgK     = Reaktoro_::constant(ref_tpr.log_equilibrium_constant);
    auto Tr      = reaction.referenceT();


    const bool has_dHr  = ref_tpr.reaction_enthalpy.sta.first  != Status::notdefined;
    const bool has_dSr  = ref_tpr.reaction_entropy.sta.first  != Status::notdefined;
    const bool has_dGr  = ref_tpr.reaction_gibbs_energy.sta.first != Status::notdefined;

    if(has_dSr && has_dGr && !has_dHr)
        dHr = dGr + dSr * TK;
    if(has_dHr && has_dGr && !has_dSr)
        dSr = (dHr-dGr)/TK;

    // the method CE was determined from the reference properties (see prepareLogK_fT)

    /// deal with Cp and A parameters conversion
    switch (CE)
    {
        case MethodCorrT_Thrift::type::CTM_EK0: // 1-term lgK = const
        // lgK = A[0];
        // dCpr = 0.0;
        // dHr  = 0.0;
            break;
        case MethodCorrT_Thrift::type::CTM_EK1: // 1-term dGr = const
                // lgK = A[2]/T;
            // dCpr = 0.0;
            // dSr  = 0.0;
            dHr = dGr;
                // dGr_d = dHr;
            lgK  = - dHr / TK / Rln10;
          break;
        case MethodCorrT_Thrift::type::CTM_EK2:  // 2-term or 1-term lgK=const at dHr=0
                // lgK = A[0] + A[2]/T;
            // dCpr = 0.0;
                // dGr_d = dHr - dSr * T;
            lgK  = (dSr - dHr/TK ) / Rln10;
          break;
        case MethodCorrT_Thrift::type::CTM_EK3:  // 3-term
                // lgK = A[0] + A[2]/T + A[3] * lnT;
            lgK  = ( dSr - dHr/TK - dCpr * ( 1 - Tr/TK - log( TK/Tr ))) / Rln10;
            dSr += dCpr * log( TK / Tr );
            dHr += dCpr * (TK - Tr );
                // dGr_d = dHr - dSr * T;
          break;
       case MethodCorrT_Thrift::type::CTM_LGK:  // full 7-term logK approx
       case MethodCorrT_Thrift::type::CTM_LGX:  // (derived from dCp=f(T))
            if (A.size() == 0 && CpCoeff.size() > 0)
            {
                //from Cp to A
                A.resize(7, 0.0);
                CpCoeff.resize(5, 0.0);

                // calculation of logK=f(T) coeffs (only first 5 Cp coefficients, conforming to Haas-Fisher function)
                A[0] = (( dSr - CpCoeff[0] - CpCoeff[0]*log(TK) - CpCoeff[1]*TK + CpCoeff[2]/(2.0*TK*TK)
                              + 2.0*CpCoeff[3]/pow(TK,0.5) - CpCoeff[4]*TK*TK/2.0 ) / Rln10).val();
                A[1] = CpCoeff[1]/(2.0*Rln10);
                A[2] = (-( dHr - CpCoeff[0]*TK - CpCoeff[1]*TK*TK/2.0 + CpCoeff[2]/TK
                           - 2.0*CpCoeff[3]*pow(TK,0.5) - CpCoeff[4]*TK*TK*TK/3.0 ) / Rln10).val();
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
