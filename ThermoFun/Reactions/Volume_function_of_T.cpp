#include "Volume_function_of_T.h"
#include "ThermoParameters.h"

namespace ThermoFun {

// Reaction volume as a polynomial of T and P (a: 1/K, 1/K^2, 1/K^3, 1/bar, 1/bar^2), dT = T - Tst, dP = P - Pst:
//   V(T,P) = Vst (1 + a0 dT + a1 dT^2 + a2 dT^3 + a3 dP + a4 dP^2)
// The correction of the properties at (T,Pst) is the integral of V over P:
//   dG = Vst [ (1 + a0 dT + a1 dT^2 + a2 dT^3) dP + a3 dP^2/2 + a4 dP^3/3 ]
// with dS = -d(dG)/dT, dH = dG - T d(dG)/dT, dCp = -T d2(dG)/dT2, ln K = -G/(RT) (the derivatives follow by autodiff).
auto thermoPropertiesReaction_Vol_fT(real TK, real Pbar, Reaction reaction, ThermoPropertiesReactionAD tpr) -> ThermoPropertiesReactionAD
{
    const double Vst = reaction.thermoReferenceProperties().reaction_volume.val; // J/bar
    const double Tst = reaction.referenceT();                                    // K
    const double Pst = reaction.referenceP() / 1e5;                              // bar
    const auto a = reaction.thermoParameters().reaction_V_fT_coeff;

    auto coeff = [&](size_t i) { return i < a.size() ? a[i] : 0.0; };

    const real dT = TK - Tst;
    const real dP = Pbar - Pst;

    const real f    = 1.0 + coeff(0)*dT + coeff(1)*dT*dT + coeff(2)*dT*dT*dT;
    const real dfdT = coeff(0) + 2.0*coeff(1)*dT + 3.0*coeff(2)*dT*dT;
    const real d2fdT2 = 2.0*coeff(1) + 6.0*coeff(2)*dT;

    const real V  = Vst * (f + coeff(3)*dP + coeff(4)*dP*dP);
    const real dG = Vst * (f*dP + coeff(3)*dP*dP/2.0 + coeff(4)*dP*dP*dP/3.0);
    const real dGdT = Vst * dfdT * dP;
    const real d2GdT2 = Vst * d2fdT2 * dP;

    tpr.reaction_volume = V;
    tpr.reaction_gibbs_energy += dG;
    tpr.reaction_entropy -= dGdT;
    tpr.reaction_enthalpy += dG - TK*dGdT;
    tpr.reaction_heat_capacity_cp -= TK*d2GdT2;
    tpr.ln_equilibrium_constant -= dG / (R_CONSTANT*TK);
    tpr.log_equilibrium_constant = tpr.ln_equilibrium_constant * ln_to_lg;
    tpr.reaction_internal_energy  = tpr.reaction_enthalpy - Pbar*tpr.reaction_volume;
    tpr.reaction_helmholtz_energy = tpr.reaction_internal_energy - TK*tpr.reaction_entropy;

    return tpr;
}

}
