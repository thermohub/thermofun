#include "LogK_function_of_T.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesReaction_LogK_fT(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Reaction reaction, MethodCorrT_Thrift::type CE) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;

    auto Rln10   = R_CONSTANT * lg_to_ln;
    auto R_T     = TK * R_CONSTANT;
    auto ref_tpr = reaction.thermo_ref_prop();
    auto A       = reaction.thermo_parameters().reaction_logK_fT_coeff;
    auto Cp      = reaction.thermo_parameters().reaction_Cp_fT_coeff;
    auto dVr     = ref_tpr.reaction_volume;   //Gr = rc[q].Gs[0];
    auto dHr     = ref_tpr.reaction_enthalpy;
    auto dSr     = ref_tpr.reaction_entropy;
    auto dCpr    = ref_tpr.reaction_heat_capacity_cp;
    auto lgK     = ref_tpr.ln_equilibrium_constant*lg_to_ln;
    auto Tr      = reaction.referenceT();

    switch (CE)
    {
        case MethodCorrT_Thrift::type::CTM_EK0: // 1-term lgK = const
                // lgK = A[0];
            dCpr = 0.0;
            dHr  = 0.0;
                // dGr_d = -dSr * T;
            break;
        case MethodCorrT_Thrift::type::CTM_EK1: // 1-term dGr = const
                // lgK = A[2]/T;
            dCpr = 0.0;
            dSr  = 0.0;
                // dGr_d = dHr;
            lgK  = - dHr / TK / Rln10;
          break;
        case MethodCorrT_Thrift::type::CTM_EK2:  // 2-term or 1-term lgK=const at dHr=0
                // lgK = A[0] + A[2]/T;
            dCpr = 0.0;
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
            lgK = A[0] + A[1] * TK + A[2]/TK + A[3] * log(TK) + A[4] / (TK*TK) +
                  A[5] * (TK*TK) + A[6] / pow(TK,0.5);
            dHr = Rln10 *( A[1]*(TK*TK) - A[2] + A[3]*TK - 2.0*A[4]/TK
                     + 2.0*A[5]*(TK*TK*TK) - 0.5*A[6]*pow(TK,0.5) );
            dSr = Rln10 * ( A[0] + 2.0*A[1]*TK + A[3]*(1.0+log(TK)) -
                       A[4]/(TK*TK) + 3.0*A[5]*(TK*TK) + 0.5*A[6]/pow(TK,0.5) );
                // dGr_d = dHr - dSr * T;
                // if( rc[q].DCp )
            dCpr = Cp[0] + Cp[1]*TK + Cp[2]/(TK*TK) + Cp[4]*(TK*TK) + Cp[3]/pow(TK,0.5);
                // dHr = Rln10 * T_2 * ( A[1] - A[2]/T_2 + A[3]/T -
                // 2.0*A[4]/T_3 + 2.0*A[5]*T - 0.5*A[6]/T_15 );
                // dSr = Rln10 * ( A[0] + 2.0*A[1]*T + A[3]*(1.0+lnT) -
                // A[4]/T_2 + 3.0*A[5]*T_2 + 0.5*A[6]/T_05 );
          break;
       default:
       ; // error message ?
    }
    // Calculation of dGr
    auto dGr  = -R_T * lgK * lg_to_ln;
    auto dUr  = dHr - Pbar*dVr;
    auto dAr  = dUr - TK*dSr;

    // Loading output data
    tpr.ln_equilibrium_constant = lgK*lg_to_ln;
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
