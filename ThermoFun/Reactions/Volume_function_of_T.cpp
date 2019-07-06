#include "Volume_function_of_T.h"
#include "ThermoParameters.h"


namespace ThermoFun {

auto thermoPropertiesReaction_Vol_fT(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Reaction reaction, ThermoPropertiesReaction tpr) -> ThermoPropertiesReaction
{

    auto prop_ref = reaction.thermo_ref_prop();
    auto Pst = reaction.referenceP();
    auto Vst = prop_ref.reaction_volume;
    auto Tst = reaction.referenceT();
    auto a = reaction.thermo_parameters().reaction_V_fT_coeff;

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
    auto VP = Vst * (Pbar - Pst);
    auto VT = Vst * (TK-Tst);
    tpr.reaction_volume   += VP;
    tpr.reaction_enthalpy += VP;
    for( unsigned i=0; i<a.size(); i++ )
    {
        switch( i ) //calc delta coef Vp(T)
        {
        case 0:
            tpr.reaction_volume += a[i] * VT;
            // aW.twp->dS  -= a * VP;
            // aW.twp->dG += a * T_Tst * VP;
            // aW.twp->dH -= a * Tst * VP;
            break;
        case 1:
            tpr.reaction_volume += a[i] * VT * (TK-Tst);
            // aW.twp->dS  -= a * VP * 2. * T_Tst;
            // aW.twp->dG += a * VP * Ts2;
            // aW.twp->dH -= a * VP * ( T + Tst ) * T_Tst;
            break;
        case 2:
            tpr.reaction_volume += a[i] * VT * (TK-Tst)*(TK-Tst);
            // aW.twp->dS  -= a * VP * 3.* Ts2;
            // aW.twp->dG += a * VP * Ts2 * T_Tst;
            // aW.twp->dH += a * VP * Ts2 * ( 2.*T + Tst );
            break;
        case 3:
            tpr.reaction_volume += a[i] * VP;
            // aW.twp->dG += a * VP * P_Pst / 2.;
            // aW.twp->dH += a * VP * P_Pst / 2.;
            break;
        case 4:
            tpr.reaction_volume += a[i] * VP * (Pbar-Pst);
            // aW.twp->dG += a * VP * ( aW.twp->P*aW.twp->P - Pst*Pst ) / 3.;
            // aW.twp->dH += a * VP * ( aW.twp->P*aW.twp->P - Pst*Pst ) / 3.;
            break;
        }
    }

//    aW.twp->Alp = aC;
//    aW.twp->Bet = aE;
    tpr.ln_equilibrium_constant -= tpr.reaction_volume * (Pbar - Pst) / (R_CONSTANT*TK);

return tpr;

}

}
