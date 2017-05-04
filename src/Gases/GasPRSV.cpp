#include "Gases/GasPRSV.h"
#include "Gases/s_solmod_.h"

namespace ThermoFun {

auto thermoPropertiesGasPRSV(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    double FugProps[6];
    solmod::TPRSVcalc myPRSV( 1, (p.val), (t.val+273.15) );
    double TClow = subst.thermoParameters().temperature_intervals[0][0];
    double * CPg = new double[7];
    for (unsigned int i = 0; i < 7; i++)
    {
        CPg[i] = subst.thermoParameters().critical_parameters[i];
    }

    myPRSV.PRSVCalcFugPure( (TClow/*+273.15*/), (CPg), FugProps );

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (t+273.15) * log( FugProps[0] );
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (p);
    tps.gibbs_energy -= R_CONSTANT * (t+273.15) * log(Fug/p);

//    setMessage(Reaktoro_::Status::calculated,"PRSV fluid model: calculated", tps );

    return tps;
}

}
