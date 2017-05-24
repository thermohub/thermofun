#include "Substances/Gases/GasPR78.h"
#include "Substances/Gases/s_solmod_.h"

namespace ThermoFun {

auto thermoPropertiesGasPR78(Reaktoro_::Temperature t, Reaktoro_::Pressure p, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    double FugProps[6];
    solmod::TPR78calc myPR78( 1, (p.val), (t.val+273.15) );
    double TClow = subst.thermoParameters().temperature_intervals[0][0];
    double * CPg = new double[7];
    for (unsigned int i = 0; i < 7; i++)
    {
        CPg[i] = subst.thermoParameters().critical_parameters[i];
    }

    myPR78.PR78CalcFugPure( (TClow/*+273.15*/), (CPg), FugProps );

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (t+273.15) * log( FugProps[0] );
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (p);
    // back correction
    tps.gibbs_energy -= R_CONSTANT * (t+273.15) * log(Fug/p);

    subst.checkCalcMethodBounds("PR78 Peng-Robinson fluid model", t.val, p.val, tps);

    return tps;
}

}
