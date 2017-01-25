#include "RyzhenkoBryzgalyn.h"

namespace ThermoFun {

auto thermoPropertiesRyzhenkoBryzgalin(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Reaction reaction, PropertiesSolvent wp) -> ThermoPropertiesReaction
{

    ThermoPropertiesReaction tpr;

//    auto ref_tpr = reaction.thermo_ref_prop();
    auto RHO = wp.density; // check units
    auto ALP = wp.Alpha;
    auto BET = wp.Beta;
    auto dALPdT = wp.dAldT;
    auto RBcoef = reaction.thermo_parameters().reaction_RB_coeff;

    auto dRHOdT = -ALP*RHO;
    auto dRHOdP = BET*RHO;
    auto d2RHOdT2 = RHO*(pow(ALP,2)-dALPdT);

    auto pK298 = RBcoef[0];
    auto A = RBcoef[1];
    auto B = RBcoef[2];

    auto a = -4.098;
    auto b = -3245.2;
    auto c = 2.2362e+5;
    auto d = -3.984e+7;
    auto e = 13.957;
    auto f = -1262.3;
    auto g = 8.5641e+5;
    auto RHO298 = 0.997061;
    auto zzH2O = 1.0107;

    auto pKw298 = -(a + b/298.15 + c/pow(298.15,2.) + d/pow(298.15,3.)
        + ( e + f/298.15 + g/pow(298.15,2.))*log10(RHO298));

    auto pKwTP = -(a + b/TK + c/pow(TK,2.) + d/pow(TK,3.)
        + ( e + f/TK + g/pow(TK,2.))*log10(RHO));

    auto zzA    = A + B/TK;
    auto BB     = ( pKwTP - pKw298*(298.15)/TK ) / zzH2O;
    auto pKTP   = pK298*(298.15/TK ) + zzA*BB;
    auto logKTP = - pKTP;
    auto dGr    = pKTP*log(10.)*R_CONSTANT*TK;
    auto dG298  = pK298*log(10.)*R_CONSTANT*298.15;

    auto I   = a*TK + b + c/TK + d/pow(TK,2.)
               + ( e*TK + f + g/TK ) * log10(RHO) - (-pKw298)*298.15;
    auto J   = A + B/TK;
    auto dI  = a - c/pow(TK,2.) - 2.*d/pow(TK,3.) + ( e - g/pow(TK,2.) ) * log10(RHO)
               + ( e*TK + f + g/TK) * 1./(RHO*log(10.)) * dRHOdT;
    auto dJ  = -B/pow(TK,2.);
    auto d2I = 2.*c/pow(TK,3.) + 6.*d/pow(TK,4.) + 2.*g/pow(TK,3.)*log10(RHO)
               + 2.*( e - g/pow(TK,2.) ) * 1./(RHO*log(10.)) * dRHOdT
               - ( e*TK + f + g/TK ) * 1./(pow(RHO,2.)*log(10.)) * pow(dRHOdT,2.)
               + ( e*TK + f + g/TK ) * 1./(RHO*log(10.)) * d2RHOdT2;
    auto d2J = 2.*B/pow(TK,3.);

    auto dSr  = R_CONSTANT*log(10.)/zzH2O * ( dI*J + dJ*I );
    auto dCPr = R_CONSTANT*log(10.)/zzH2O * TK * ( d2I*J + 2.*dI*dJ + d2J*I );
    auto dVr  = - R_CONSTANT*log(10.)/zzH2O * ( e*TK + f + g/TK ) * 1./(RHO*log(10.))
                * dRHOdP * ( A + B/TK);
    auto dHr  = dGr + dSr*TK;
    auto dUr  = dHr - Pbar*dVr;
    auto dAr  = dUr - TK*dSr;

    tpr.ln_equilibrium_constant    = logKTP*lg_to_ln;
    tpr.reaction_gibbs_energy      = dGr;
    tpr.reaction_entropy           = dSr;
    tpr.reaction_enthalpy          = dHr;
    tpr.reaction_heat_capacity_cp  = dCPr;
    tpr.reaction_volume            = dVr;
    tpr.reaction_helmholtz_energy  = dAr;
    tpr.reaction_internal_energy   = dUr;

    return tpr;
}

}
