#include "SolidBMGottschalk.h"

namespace TCorrPT {

auto thermoPropertiesMinBMGottschalk (Reaktoro::Temperature t, Reaktoro::Pressure p, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{

    /*
    if( fabs( P_Pst ) > PRESSURE_PREC || fabs( T_Tst ) > TEMPER_PREC )
    {
       double VV0=0.0, GG0=0.0, HH0=0.0, SS0=0.0, aC=0.0, aE=0.0;

       BirchMurnaghan( 0.1*Pst, 0.1*P, Tst, T, Vst*10., dc[q].ODc,
                       VV0, aC, aE, GG0, HH0, SS0 );
       // increments to V, G, H, S
       aW.twp->V += Vst+VV0/10.;    // fixed by KD on 22.11.04
           // aW.twp->V += VV0/10.;
       aW.twp->S += SS0;
       aW.twp->G += GG0;
       aW.twp->H += HH0;
       aW.twp->Alp = aC;
       aW.twp->Bet = aE;
    }
    */

}

}
