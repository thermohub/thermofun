#include "Reactions/FrantzMarshall.h"

namespace TCorrPT {

auto thermoPropertiesFrantzMarshall(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Reaction reaction, PropertiesSolvent wp) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;

//    auto ref_tpr = reaction.thermo_ref_prop();
    auto RHO = wp.density; // check units
    auto ALP = wp.Alpha;
    auto BET = wp.Beta;
    auto dALPdT = wp.dAldT;
    auto MFcoef = reaction.thermo_parameters().reaction_FM_coeff;

    auto dRHOdT = -ALP*RHO;
    auto dRHOdP = BET*RHO;
    auto d2RHOdT2 = RHO*(pow(ALP,2)-dALPdT);

    auto a = MFcoef[0];
    auto b = MFcoef[1];
    auto c = MFcoef[2];
    auto d = MFcoef[3];
    auto e = MFcoef[4];
    auto f = MFcoef[5];
    auto g = MFcoef[6];

    auto logK = a + b/TK + c/pow(TK,2.) + d/pow(TK,3.) + ( e + f/TK + g/pow(TK,2.) )*log10(RHO);

    auto J  = a + b/TK + c/pow(TK,2.) + d/pow(TK,3.) + ( e + f/TK + g/pow(TK,2.) )*log10(RHO);
    auto dJ = - b/pow(TK,2.) - 2.*c/pow(TK,3.) - 3.*d/pow(TK,4.)
              + ( -f/pow(TK,2.) - 2.*g/pow(TK,3.) )*log10(RHO)
              + ( e + f/TK + g/pow(TK,2.) ) * 1./(RHO*log(10.)) * dRHOdT;
    auto d2J = 2.*b/pow(TK,3.) + 6.*c/pow(TK,4.) + 12.*d/pow(TK,5.)
               + ( 2.*f/pow(TK,3.) + 6.*g/pow(TK,4.) )*log10(RHO)
               + 2.*( -f/pow(TK,2.) -2.*g/pow(TK,3.) ) * 1./(RHO*log(10.)) * dRHOdT
               - ( e + f/TK + g/pow(TK,2.) ) * 1./(pow(RHO,2.)*log(10.)) * pow (dRHOdT,2.)
               + ( e + f/TK + g/pow(TK,2.) ) * 1./(RHO*log(10.)) * d2RHOdT2;

    auto dGr  = - R_CONSTANT*TK*log(10.)*logK;
    auto dSr  = R_CONSTANT*log(10.)*( J + dJ*TK );
    auto dCPr = R_CONSTANT*log(10.)*TK*( 2.*dJ + d2J*TK );
    auto dVr  = - R_CONSTANT*log(10.)*TK*( ( e + f/TK + g/pow(TK,2.) ) * 1./(RHO*log(10.)) * dRHOdP );
    auto dHr  = dGr + dSr*TK;
    auto dUr  = dHr - Pbar*dVr;
    auto dAr  = dUr - TK*dSr;

    tpr.ln_equilibrium_constant    = logK*lg_to_ln;
    tpr.reaction_enthalpy          = dHr;
    tpr.reaction_entropy           = dSr;
    tpr.reaction_gibbs_energy      = dGr;
    tpr.reaction_heat_capacity_cp  = dCPr;
    tpr.reaction_volume            = dVr;
    tpr.reaction_helmholtz_energy  = dAr;
    tpr.reaction_internal_energy   = dUr;

    return tpr;
}


}
