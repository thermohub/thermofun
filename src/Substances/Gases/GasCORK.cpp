#include "Substances/Gases/GasCORK.h"
#include "Substances/Gases/s_solmod_.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesGasCORK(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Substance subst, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    double FugProps[6];
    char Eos_Code;

    if (subst.formula() == "CO2") Eos_Code = 'C';
    if (subst.formula() == "H2O") Eos_Code = 'V';

    solmod::TCORKcalc myCORK( 1, Pbar.val, (TK.val), Eos_Code );  // modified 05.11.2010 (TW)
    double TClow = subst.thermoParameters().temperature_intervals[0][0];
    double * CPg = new double[7];
    for (unsigned int i = 0; i < 7; i++)
    {
        CPg[i] = subst.thermoParameters().critical_parameters[i];
    }

    myCORK.CORKCalcFugPure( (TClow/*+273.15*/), (CPg), FugProps );

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (TK) * log( FugProps[0] ); // from ideal gas at 1 bar and givent T to pure real gas/fluid at T and P
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (Pbar); //FugProps[0] - fugacity coefficient
    tps.gibbs_energy -= R_CONSTANT * (TK) * log(Fug/Pbar);

    subst.checkCalcMethodBounds("CORK compensated-Redlich-Kwong fluid model", TK.val, Pbar.val*bar_to_Pa, tps);

    return tps;
}
}
