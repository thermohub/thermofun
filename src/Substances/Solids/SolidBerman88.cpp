#include "SolidBerman88.h"
#include "Common/Exception.h"

namespace ThermoFun {

auto thermoPropertiesMinBerman88(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto Pst = subst.referenceP() / bar_to_Pa; // in bar
    auto Tst = subst.referenceT(); // in K
    auto Vst = subst.thermoReferenceProperties().volume; // j/bar
    auto T   = Reaktoro_::Temperature (TK.val);
    auto P   = Reaktoro_::Pressure (Pbar.val); // in bar
    auto Ts2   = Tst*Tst;

    auto vc = subst.thermoParameters().volume_coeff;

    if (vc.size() != 5)
    {
        errorModelParameters("five volume coefficients", "dVdP Berman88", __LINE__, __FILE__);
    }

    // expansion/compressibility
    auto aE = vc[0] + vc[1]*2.*(T-Tst) + vc[2]*3.*((T-Tst)*(T-Tst));
    auto aC = - vc[3] - vc[4]*2.*P;

    // increment thermodynamic properties
    tps.volume = subst.thermoReferenceProperties().volume;  // added by KD 22.11.04
    if( (P-Pst) != 0.0 ||  (T-Tst) != 0.0 )
    {  // can be calculated
        Reaktoro_::ThermoScalar VP = Vst * (P - Pst);
        Reaktoro_::ThermoScalar VT = Vst * (T - Tst);
        tps.gibbs_energy += ( VP );
        tps.enthalpy     += ( VP );
        tps.volume       += ( vc[0]*VT + vc[1]*VT*(T-Tst) + vc[2]*VT*Ts2 + vc[3]*VP + vc[4]*VP*(P-Pst) );
        tps.entropy      += ( - vc[0]*VP - vc[1]*VP*2.*(T-Tst) - vc[2]*VP*3*Ts2 );
        tps.gibbs_energy += ( vc[0]*(T-Tst)*VP + vc[1]*VP*Ts2 + vc[2]*VP*Ts2*(T-Tst) + vc[3]*VP*(P-Pst) / 2.
                            + vc[4]*VP*((P-Pst)*(P-Pst)) / 3. );
        tps.enthalpy     += ( - vc[0]*Tst*VP - vc[1]*VP*(T + Tst)* (T-Tst) + vc[2]*VP*Ts2*(2.*T + Tst)
                            + vc[3]*VP*(P-Pst) / 2. + vc[4]*VP*((P-Pst)*(P-Pst)) / 3. );
        tps.internal_energy  = tps.enthalpy - Pbar*tps.volume;
        tps.helmholtz_energy = tps.internal_energy - TK*tps.entropy;
    }

    subst.checkCalcMethodBounds("Berman multisite model", TK.val-C_to_K, P.val, tps);


    // maybe this should be added back?
//    aW.twp->Alp = aC;
//    aW.twp->Bet = aE;

       // dc[q].Comp = (float)aW.twp->Alp;
       // dc[q].Expa = (float)aW.twp->Bet;
    return tps;
}

}
