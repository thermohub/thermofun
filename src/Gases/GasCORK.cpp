#include "Gases/GasCORK.h"
#include "Gases/s_solmod_.h"

namespace TCorrPT {

double (*Fugpure)[6];

///// Calculates properties of pure fluids when called from DCthermo
//long int CORKCalcFugPure( double Tk, double Pbar, double Tmin, double *Cpg, double *FugProps )
//{
//        long int retCode = 0;
//        double Coeff[7];

//        for( int ii=0; ii<7; ii++ )
//                Coeff[ii] = Cpg[ii];

//        if( (Tk >= Tmin) && (Tk < 1e4) && (Pbar >= 1e-5) && (Pbar < 1e5) )
//        {
//                retCode = FugacityPT( 0, Coeff );
//                for( int i=0; i<6; i++ )
//                        FugProps[i] = Fugpure[0][i];
//                return retCode;
//        }

//        else
//        {
//                for( int i=1; i<6; i++ )
//                        FugProps[i] = 0.;
//                FugProps[0] = 1.;
//                FugProps[4] = 8.31451*Tk/Pbar;
//                return -1;
//        }
//}

auto thermoPropertiesGasCORK(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
/*
    char pct[6];
    double FugProps[6];
    solmod::TCORKcalc myCORK( 1, p.val, (t.val+273.15), (pct[3]) );  // modified 05.11.2010 (TW)
    double TClow = subst.thermoParameters().temperature_intervals[0][0];
    double * CPg =  &subst.thermoParameters().critical_parameters[0];
//    aW.twp->CPg = dcp->CPg;
//    aW.twp->TClow = dcp->TCint[0];
    myCORK.CORKCalcFugPure( (TClow+273.15), (CPg), FugProps );
     myCORK.~TCORKcalc();

//    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (t+273.15) * log( FugProps[0] );
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
//    aW.twp->Fug = FugProps[0] * (aW.twp->P);
//    aW.twp->CPg = NULL;

*/
    return tps;
}
}
