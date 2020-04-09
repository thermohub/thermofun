#include "SolidHPLandau.h"
#include "Common/Exception.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesHPLandau(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    Reaktoro_::ThermoScalar Tcr, Qq, dQq;
    vector<double> transProp = subst.thermoParameters().m_landau_phase_trans_props;
//    auto (P/1000) = Reaktoro::Pressure (p.val /1000);  // in kbar
    auto TrK = subst.referenceT();

    if (transProp.size() < 3)
    {
        std::cout << transProp.size() << " " << std::endl;
        errorModelParameters("transition properties", "HP Landau", __LINE__, __FILE__);
    }

    // Parameters of lambda-transition
    auto Tcr0 = transProp[0];   // given in centigrade
    auto Smax = transProp[1];
    auto Vmax = transProp[2];

    auto k298 = subst.thermoParameters().isothermal_compresibility; // This is the bulk modulus k in kbar at 298 K!
    auto a0 = subst.thermoParameters().isobaric_expansivity*1e-05; // This is the a parameter (at one bar) in 1/K!

    if ( Smax <= 0)
        Smax = 1.0e-20;

    if(  Vmax == 0.0 )
    {
        Tcr = Tcr0;
    }
    else
        Tcr = Tcr0 + Vmax/Smax * (Pbar);

    Tcr += C_to_K;
    Tcr0 += C_to_K;     // dT is a constant equal 273.15
    auto Q298 = pow( 1.- TrK/Tcr0, 0.25 );

    if( TK<Tcr )
    {
        Qq = pow( 1.- TK/Tcr, 0.25 );
        dQq = - TK*Vmax/(4.*pow((1.+TK/Tcr),0.75)*pow(Tcr,2.)*Smax);
    }
    else
    {
        Qq = 0.;
        dQq = 0.;
    }

    auto kT = k298 * (1. - (1.5e-4) * ( TK-TrK ));
    auto v_bis = Vmax *pow(Q298,2.) * ( 1.+a0*(TK-TrK) - 20.*a0*( sqrt(TK) - sqrt(TrK) ));
    auto ivdp = (v_bis*kT/3.) * ( pow((1.+4.*(Pbar/1000)/kT),0.75) - 1. ) * (1000.);
    // idvdtdp = Vmax*(P/1000)*(a0-10.*a0/sqrt(T)) * (1000.);
    auto idvdtdp = ( ((Vmax*pow(Q298,2.))*kT/3.)*(a0-10.*a0*pow(TK,-0.5)) * (pow((1.+4.*(Pbar/1000)/kT),0.75)-1.)
                     - ((v_bis)*k298/3)*(1.5e-4) * (pow((1.+4.*(Pbar/1000)/kT),0.75)-1.)
                     + (v_bis)*(Pbar/1000)*(1.5e-4)/(1.-(1.5e-4)*(TK-TrK)) * pow((1.+4.*(Pbar/1000)/kT),-0.25) ) * (1000.);
    auto smq = Smax * Tcr0 *( pow(Q298,2.) - pow(Q298,6.)/3. );

    // increment thermodynamic properties
    if (Qq == 0. && dQq == 0.)
    {
        double Qq_ = 0.0; double dQq_ = 0.0;
        tps.gibbs_energy += ( smq - TK*Smax*pow(Q298,2.) + ivdp + Smax*((TK-Tcr)*pow(Qq_,2.) + Tcr*pow(Qq_,6.)/3.) );
        tps.entropy      += ( Smax * ( pow(Q298,2.) - pow(Qq_,2.) ) - idvdtdp );
        tps.enthalpy     += ( smq - Smax*Tcr*( pow(Qq_,2.) - pow(Qq_,6.)/3.) + ivdp - TK*idvdtdp );
        tps.volume        = ( v_bis + 2.*Smax*(TK-Tcr)*Qq_*dQq_ - Smax*pow(Qq_,2.)*Vmax/Smax + Tcr*pow(Qq_,2.)*dQq_
                              + Vmax/Smax*pow(Qq_,3.)/3. );  // in J/bar
    } else
    {
        tps.gibbs_energy += ( smq - TK*Smax*pow(Q298,2.) + ivdp + Smax*((TK-Tcr)*pow(Qq,2.) + Tcr*pow(Qq,6.)/3.) );
        tps.entropy      += ( Smax * ( pow(Q298,2.) - pow(Qq,2.) ) - idvdtdp );
        tps.enthalpy     += ( smq - Smax*Tcr*( pow(Qq,2.) - pow(Qq,6.)/3.) + ivdp - TK*idvdtdp );
        tps.volume        = ( v_bis + 2.*Smax*(TK-Tcr)*Qq*dQq - Smax*pow(Qq,2.)*Vmax/Smax + Tcr*pow(Qq,2.)*dQq
                              + Vmax/Smax*pow(Qq,3.)/3. );  // in J/bar
    }
    if( TK<Tcr )  // Cp is corrected at subcritical T only
        tps.heat_capacity_cp += ( TK*Smax/(2.*sqrt(Tcr)*sqrt(Tcr-TK)) );

    tps.internal_energy  = tps.enthalpy - Pbar*tps.volume;
    tps.helmholtz_energy = tps.internal_energy - TK*tps.entropy;

    subst.checkCalcMethodBounds("Holland and Powell Landau model", TK.val, Pbar.val*bar_to_Pa, tps);

    return tps;
}

}
