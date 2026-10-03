#include "Substances/Gases/GasSRK.h"
#include "Substances/Gases/s_solmod_.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesGasSRK(real TK, real Pbar, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD
{
    real FugProps[6];
    if (Pbar.val() == 0.0)
        Pbar += 1e-5;
    solmod::TSRKcalc mySRK( 1, Pbar, TK );
    double TClow = lowerTemperatureBound(subst, "SRK Soave-Redlich-Kwong fluid model");
    real CPg[7];
    for (unsigned int i = 0; i < 7; i++)
    {
        CPg[i] = subst.thermoParameters().critical_parameters[i];
    }

    mySRK.SRKCalcFugPure( (TClow/*+273.15*/), CPg, FugProps );

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (TK) * log( FugProps[0] );
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (Pbar);
    // back correction
    tps.gibbs_energy -= R_CONSTANT * (TK) * log(Fug/Pbar);

    return tps;
}

}
