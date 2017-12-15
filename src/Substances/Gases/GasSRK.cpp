#include "Substances/Gases/GasSRK.h"
#include "Substances/Gases/s_solmod_.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesGasSRK(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    double FugProps[6];
    solmod::TSRKcalc mySRK( 1, (Pbar.val), (TK.val) );
    double TClow = subst.thermoParameters().temperature_intervals[0][0];
    double * CPg = new double[7];
    for (unsigned int i = 0; i < 7; i++)
    {
        CPg[i] = subst.thermoParameters().critical_parameters[i];
    }

    mySRK.SRKCalcFugPure( (TClow/*+273.15*/), (CPg), FugProps );

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (TK) * log( FugProps[0] );
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (Pbar);
    // back correction
    tps.gibbs_energy -= R_CONSTANT * (TK) * log(Fug/Pbar);

    subst.checkCalcMethodBounds("SRK Soave-Redlich-Kwong fluid model", TK.val, Pbar.val*bar_to_Pa, tps);

    return tps;
}

}
