#include "DolejsManning2010.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesDolejsManning2010(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Reaction reaction, PropertiesSolvent wp) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction tpr;

//    auto ref_tpr = reaction.thermoReferenceProperties();
    auto RHOw = wp.density/1000; // in g/cm3
    auto ALPw = wp.Alpha;
    auto BETw = wp.Beta*1e05;
    auto dALPdTw = wp.dAldT;
    auto DMcoef = reaction.thermoParameters().reaction_DM10_coeff;

    auto dRHOdT = -ALPw*RHOw;
    auto dRHOdP = BETw*RHOw;
    auto d2RHOdT2 = RHOw*(pow(ALPw,2)-dALPdTw);

    auto a = DMcoef[0];
    auto b = DMcoef[1];
    auto c = DMcoef[2];
    auto d = DMcoef[3];
    auto e = DMcoef[4];


    auto lnK = (-1/R_CONSTANT)*(a/TK + b + c*log(TK) + d*TK + e*log(RHOw));
    auto dSr = -b - c*(1 + log(TK)) - 2*d*TK - e*log(RHOw) + e*TK*ALPw;
    auto dHr = a - c*TK - d*pow(TK,2.) + e*pow(TK,2.)*ALPw;
    auto dCPr = -c - 2*d*TK + 2*e*TK*ALPw + e*pow(TK,2.)*dALPdTw;
    auto dVr = e*TK*BETw;
    auto dGr  = dHr - TK*dSr;
    auto logK = lnK * ln_to_lg;
    auto dUr  = dHr - Pbar*dVr;
    auto dAr  = dUr - TK*dSr;

    // if (std::fabs((-R_CONSTANT*TK*lnK) - dGr) > 0.1)
           // Error( GetName(),"DM denisty model: dGr and lnK not consistent!");

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
