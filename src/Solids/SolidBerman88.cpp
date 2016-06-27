#include "SolidBerman88.h"

namespace TCorrPT {

auto thermoPropertiesMinBerman88(Reaktoro::Temperature t, Reaktoro::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto Pst = subst.referenceP() / bar_to_Pa; // in bar
    auto Tst = subst.referenceT(); // in K
    auto Vst = subst.thermoReferenceProperties().volume; // j/bar
    auto T   = t;
    auto P = p / bar_to_Pa; // in bar
    auto P_Pst = P - Pst;
    auto T_Tst = T -Tst;
    auto Ts2   = Tst*Tst;

    auto vc = subst.thermoParameters().volume_coeff;

    if (vc.size() != 5)
    {
        // error
    }

    // expansion/compressibility
    auto aE = vc[0] + vc[1]*2.*T_Tst + vc[2]*3.*(T_Tst*T_Tst);
    auto aC = - vc[3] - vc[4]*2.*P;

    // increment thermodynamic properties
    tps.volume = subst.thermoReferenceProperties().volume;  // added by KD 22.11.04
    if( P_Pst != 0.0 ||  T_Tst != 0.0 )
    {  // can be calculated
        auto VP = Vst * P_Pst;
        auto VT = Vst * T_Tst;
        tps.gibbs_energy += ( VP );
        tps.enthalpy     += ( VP );
        tps.volume       += ( vc[0]*VT + vc[1]*VT*T_Tst + vc[2]*VT*Ts2 + vc[3]*VP + vc[4]*VP*P_Pst );
        tps.entropy      += ( - vc[0]*VP - vc[1]*VP*2.*T_Tst - vc[2]*VP*3*Ts2 );
        tps.gibbs_energy += ( vc[0]*T_Tst*VP + vc[1]*VP*Ts2 + vc[2]*VP*Ts2*T_Tst + vc[3]*VP*P_Pst / 2.
                            + vc[4]*VP*(P_Pst*P_Pst) / 3. );
        tps.enthalpy     += ( - vc[0]*Tst*VP - vc[1]*VP*(T + Tst)* T_Tst + vc[2]*VP*Ts2*(2.*T + Tst)
                            + vc[3]*VP*P_Pst / 2. + vc[4]*VP*(P_Pst*P_Pst) / 3. );
    }

    // maybe this should be added back?
//    aW.twp->Alp = aC;
//    aW.twp->Bet = aE;

       // dc[q].Comp = (float)aW.twp->Alp;
       // dc[q].Expa = (float)aW.twp->Bet;
}

}
