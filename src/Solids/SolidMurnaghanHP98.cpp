#include "SolidMurnaghanHP98.h"

namespace TCorrPT {

auto thermoPropertiesMinMurnaghanEOSHP98(Reaktoro::Temperature T, Reaktoro::Pressure P, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto Pst = subst.referenceP() / bar_to_Pa; // in bar
    auto Tst = subst.referenceT(); // in K
    auto Vst = subst.thermoReferenceProperties().volume; // j/bar
    auto P_Pst = P - Pst;
    auto T_Tst = T -Tst;
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
        auto kap = k0 * ( 1. - (1.5e-4)*(T-Tst) );
//        aW.twp->Bet = 1./(P/1000) * (1. - pow( (1.- 4.*(P/1000)/(kap + 4.*(P/1000) )), 0.25 ));  // compressibility at T (to check)
        auto Vt = Vst * (1.+ a0*(T-Tst)-20.*a0*(pow(T,0.5)-pow(Tst,0.5)) );

        // increment thermodynamic properties
        dg = ( (1./3.)*Vt*kap*(pow((1.+4.*(P/1000)/kap),0.75 )- 1.) * (1000.) );
        ds = ( - ( (Vst*kap/3.)*(a0-10.*a0*pow(T,-0.5)) * (pow((1.+4.*(P/1000)/kap),0.75)-1.)
                   - (Vt*k0/3.)*(1.5e-4) * (pow((1.+4.*(P/1000)/kap),0.75)-1.)
                   + Vt*(P/1000)*(1.5e-4)/(1.-(1.5e-4)*(T-Tst)) * pow((1.+4.*(P/1000)/kap),(-0.25)) ) * (1000.) );
        dh = dg + T*ds;
        dv = ( Vt*pow( (1.- 4.*(P/1000)/(kap + 4.*(P/1000) )),0.25 ));
        dcp = - T * ( (5./3.)*Vst*a0*kap/pow(T,1.5) * (pow((1.+4.*(P/1000)/kap),0.75)-1.)
                    - (2./3.)*Vst*(a0-10.*a0/pow(T,0.5))*k0*(1.5e-4) * (pow((1.+4*(P/1000)/kap),0.75)-1.)
                    + 2.*Vst*(a0-10.*a0/pow(T,0.5))*(P/1000)*(1.5e-4) / (1.-(1.5e-4)*(T-Tst)) * pow((1.+4.*(P/1000)/kap),(-0.25))
                    - Vt*pow((P/1000),2.)*pow((1.5e-4),2.) / ( pow((1.-(1.5e-4)*(T-Tst)),3.)*k0 )
                    * pow((1.+4*(P/1000)/kap),(-1.25)) ) * (1000.);

        tps.gibbs_energy     += dg;
        tps.entropy          += ds;
        tps.enthalpy         += dh;
        tps.volume           += dv;
        tps.heat_capacity_cp += dcp;

    } else // Molar volume assumed independent of T and P
    {
        tps.volume           = Vst;
        tps.gibbs_energy    += Vst * (P - Pst);
        tps.enthalpy        += Vst * (P - Pst);
        tps.internal_energy  = tps.enthalpy - P*tps.volume;
        tps.helmholtz_energy = tps.internal_energy - (T+273.15)*tps.entropy;
    }
    return tps;
}

}
