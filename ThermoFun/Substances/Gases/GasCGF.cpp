#include "Substances/Gases/GasCGF.h"
#include "Substances/Gases/s_solmod_.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesGasCGF(real TK, real Pbar, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD
{
    real FugProps[6];
    if (Pbar.val() == 0.0)
        Pbar += 1e-5;
    solmod::TCGFcalc myCGF( 1, Pbar, TK );
    double TClow = lowerTemperatureBound(subst, "CG Churakov-Gottschalk fluid model");
    real CPg[12] = {}; // CGcalcFugPure reads 12 coefficients
    const auto& critical_parameters = subst.thermoParameters().critical_parameters;
    for (unsigned int i = 0; i < 12 && i < critical_parameters.size(); i++)
    {
        CPg[i] = critical_parameters[i];
    }

    myCGF.CGcalcFugPure( (TClow/*+273.15*/), CPg, FugProps );

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
