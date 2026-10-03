#include "Substances/Gases/GasPRSV.h"
#include "Substances/Gases/s_solmod_.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesGasPRSV(real TK, real Pbar, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD
{
    double FugProps[6];
    if (Pbar.val() == 0.0)
        Pbar += 1e-5;
    solmod::TPRSVcalc myPRSV( 1, (Pbar.val()), (TK.val()) );
    double TClow = lowerTemperatureBound(subst, "PRSV Peng-Robinson-Stryjek-Vera fluid model");
    double * CPg = new double[7];
    for (unsigned int i = 0; i < 7; i++)
    {
        CPg[i] = subst.thermoParameters().critical_parameters[i];
    }

    myPRSV.PRSVCalcFugPure( (TClow/*+273.15*/), (CPg), FugProps );

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (TK) * log( FugProps[0] );
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (Pbar);
    tps.gibbs_energy -= R_CONSTANT * (TK) * log(Fug/Pbar);

    return tps;
}

}
