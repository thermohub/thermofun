// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "Substances/Gases/GasSTP.h"
#include "Substances/Gases/s_solmod_.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "ThermoParameters.h"

namespace ThermoFun {

auto thermoPropertiesGasSTP(real TK, real Pbar, Substance subst, ThermoPropertiesSubstanceAD tps) -> ThermoPropertiesSubstanceAD
{
    real FugProps[6];
    char Eos_Code;
    if (Pbar.val() == 0.0)
        Pbar += 1e-5;

    if (subst.formula() == "CO2") Eos_Code = 'C';
    if (subst.formula() == "H2O") Eos_Code = 'V';

    solmod::TSTPcalc mySTP( 1, Pbar, TK, Eos_Code );  // modified 05.11.2010 (TW)
    double TClow = lowerTemperatureBound(subst, "STP Sterner-Pitzer fluid model");
    real CPg[7];
    for (unsigned int i = 0; i < 7; i++)
    {
        CPg[i] = subst.thermoParameters().critical_parameters[i];
    }

    mySTP.STPCalcFugPure( (TClow/*+273.15*/), CPg, FugProps );

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
