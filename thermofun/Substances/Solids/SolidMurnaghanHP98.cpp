#include "SolidMurnaghanHP98.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesMinMurnaghanEOSHP98(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto Pst = subst.referenceP() / bar_to_Pa; // in bar
    auto Tst = subst.referenceT(); // in K
    auto Vst = subst.thermoReferenceProperties().volume; // j/bar
    auto P_Pst = Pbar - Pst;
    auto T_Tst = TK -Tst;
    auto k0 = subst.thermoParameters().isothermal_compresibility; // bulk modulus k in kbar at 298 K
    auto a0 = subst.thermoParameters().isobaric_expansivity; // in 1/K

    auto dg  = tps.gibbs_energy;
    auto ds  = tps.entropy;
    auto dh  = tps.enthalpy;
    auto dv  = tps.volume;
    auto dcp = tps.heat_capacity_cp;

    if ((P_Pst != 0 || T_Tst !=0 ) && fabs(Vst.val) > 1e-09 && fabs(a0) > 1e-09 && fabs(k0) > 1e-09)
    {
//        aW.twp->Alp = a0 * (1. - 10./pow(T,0.5) );  // expansion at T
        auto kap = k0 * ( 1. - (1.5e-4)*(TK-Tst) );
//        aW.twp->Bet = 1./(P/1000) * (1. - pow( (1.- 4.*(P/1000)/(kap + 4.*(P/1000) )), 0.25 ));  // compressibility at T (to check)
        auto Vt = Vst * (1.+ a0*(TK-Tst)-20.*a0*(pow(TK,0.5)-pow(Tst,0.5)) );

        // increment thermodynamic properties
        dg = ( (1./3.)*Vt*kap*(pow((1.+4.*(Pbar/1000)/kap),0.75 )- 1.) * (1000.) );
        ds = ( - ( (Vst*kap/3.)*(a0-10.*a0*pow(TK,-0.5)) * (pow((1.+4.*(Pbar/1000)/kap),0.75)-1.)
                   - (Vt*k0/3.)*(1.5e-4) * (pow((1.+4.*(Pbar/1000)/kap),0.75)-1.)
                   + Vt*(Pbar/1000)*(1.5e-4)/(1.-(1.5e-4)*(TK-Tst)) * pow((1.+4.*(Pbar/1000)/kap),(-0.25)) ) * (1000.) );
        dh = dg + TK*ds;
        dv = ( Vt*pow( (1.- 4.*(Pbar/1000)/(kap + 4.*(Pbar/1000) )),0.25 ));
        dcp = - TK * ( (5./3.)*Vst*a0*kap/pow(TK,1.5) * (pow((1.+4.*(Pbar/1000)/kap),0.75)-1.)
                    - (2./3.)*Vst*(a0-10.*a0/pow(TK,0.5))*k0*(1.5e-4) * (pow((1.+4*(Pbar/1000)/kap),0.75)-1.)
                    + 2.*Vst*(a0-10.*a0/pow(TK,0.5))*(Pbar/1000)*(1.5e-4) / (1.-(1.5e-4)*(TK-Tst)) * pow((1.+4.*(Pbar/1000)/kap),(-0.25))
                    - Vt*pow((Pbar/1000),2.)*pow((1.5e-4),2.) / ( pow((1.-(1.5e-4)*(TK-Tst)),3.)*k0 )
                    * pow((1.+4*(Pbar/1000)/kap),(-1.25)) ) * (1000.);

        tps.gibbs_energy     += dg;
        tps.entropy          += ds;
        tps.enthalpy         += dh;
        tps.volume           += dv;
        tps.heat_capacity_cp += dcp;
        tps.internal_energy  = tps.enthalpy - Pbar*tps.volume;
        tps.helmholtz_energy = tps.internal_energy - (TK)*tps.entropy;

    } else // Molar volume assumed independent of T and P
    {
        tps.volume           = Vst;
        tps.gibbs_energy    += Vst * (Pbar - Pst);
        tps.enthalpy        += Vst * (Pbar - Pst);
        tps.internal_energy  = tps.enthalpy - Pbar*tps.volume;
        tps.helmholtz_energy = tps.internal_energy - (TK)*tps.entropy;
    }

    subst.checkCalcMethodBounds("Holland and Powell Murnaghan model", TK.val, Pbar.val*1e05, tps);

    return tps;
}

}
