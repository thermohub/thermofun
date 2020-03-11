#include "Volume_function_of_T.h"
#include "ThermoParameters.h"


namespace ThermoFun {

auto thermoPropertiesReaction_Vol_fT(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Reaction reaction, ThermoPropertiesReaction tpr) -> ThermoPropertiesReaction
{

    auto prop_ref = reaction.thermoReferenceProperties();
    auto Pst = reaction.referenceP();
    auto Vst = prop_ref.reaction_volume;
    auto Tst = reaction.referenceT();
    auto a = reaction.thermoParameters().reaction_V_fT_coeff;

//    if( (methodP == MethodCorrP_Thrift::type::CPM_VKE || methodP == MethodCorrP_Thrift::type::CPM_VBE) && rc[q].DVt )
//    {  // calc on equation V(P,T)
    auto aC = Reaktoro_::ThermoScalar(0.);
    auto aE = Reaktoro_::ThermoScalar(0.);
    for( unsigned i=0; i<a.size(); i++ )
    { // see all  coef Vp(T,P)
        switch( i ) // calc delta
        {
        case 0:
            aE += a[i];
            break;
        case 1:
            aE += a[i] * 2. * (TK-Tst);
            break;
        case 2:
            aE += a[i] * 3. * (TK-Tst) * (TK-Tst);
            break;
        case 3:
            aC -= a[i];
            break;
        case 4:
            aC -= a[i] * 2. * Pbar;
            break;
        }
    }
    auto VP = Vst * (Pbar - Pst); // J
    auto VT = Vst * (TK-Tst); // J*K/bar
    tpr.reaction_volume   += VP;
    tpr.reaction_enthalpy += VP;
    for( unsigned i=0; i<a.size(); i++ )
    {
        switch( i ) //calc delta coef Vp(T)
        {
        case 0:
            tpr.reaction_volume += a[i] * VT;
            // tpr.reaction_entropy -= a[i] * VP;
            // tpr.reaction_gibbs_energy += a[i] * T_Tst * VP;
            // tpr.reaction_enthalpy -= a[i] * Tst * VP;
            break;
        case 1:
            tpr.reaction_volume += a[i] * VT * (TK-Tst);
            // tpr.reaction_entropy  -= a[i] * VP * 2. * T_Tst;
            // tpr.reaction_gibbs_energy += a[i] * VP * Ts2;
            // tpr.reaction_enthalpy -= a[i] * VP * ( T + Tst ) * T_Tst;
            break;
        case 2:
            tpr.reaction_volume += a[i] * VT * (TK-Tst)*(TK-Tst);
            // tpr.reaction_entropy  -= a[i] * VP * 3.* Ts2;
            // tpr.reaction_gibbs_energy += a[i] * VP * Ts2 * T_Tst;
            // tpr.reaction_enthalpy -= a[i] * VP * Ts2 * ( 2.*T + Tst );
            break;
        case 3:
            tpr.reaction_volume += a[i] * VP;
            // atpr.reaction_gibbs_energy += a[i] * VP * P_Pst / 2.;
            // tpr.reaction_enthalpy -= a[i] * VP * P_Pst / 2.;
            break;
        case 4:
            tpr.reaction_volume += a[i] * VP * (Pbar-Pst);
            // tpr.reaction_gibbs_energy += a[i] * VP * ( Pbar*Pbar - Pst*Pst ) / 3.;
            // tpr.reaction_enthalpy -= a[i] * VP * ( Pbar*Pbar - Pst*Pst ) / 3.;
            break;
        }
    }

//    aW.twp->Alp = aC;
//    aW.twp->Bet = aE;
    tpr.ln_equilibrium_constant -= tpr.reaction_volume * (Pbar - Pst) / (R_CONSTANT*TK);
    tpr.log_equilibrium_constant = tpr.ln_equilibrium_constant/lg_to_ln;
    tpr.reaction_entropy = (tpr.reaction_enthalpy - tpr.reaction_gibbs_energy)/TK;
    tpr.reaction_internal_energy  = tpr.reaction_enthalpy - Pbar*tpr.reaction_volume;
    tpr.reaction_helmholtz_energy = tpr.reaction_internal_energy - TK*tpr.reaction_entropy;

return tpr;

}

}
