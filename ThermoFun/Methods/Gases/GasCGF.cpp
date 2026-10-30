// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "Methods/Gases/GasCGF.h"
#include "Methods/Gases/s_solmod_.h"
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/Constants.h>

namespace ThermoFun {

auto thermoPropertiesGasCGF(real TK, real Pbar, const GasInputs& subst, ThermoPropertiesSubstanceAD tps, bool cancel_fugacity) -> ThermoPropertiesSubstanceAD
{
    real FugProps[6];
    if (Pbar.val() == 0.0)
        Pbar += 1e-5;
    solmod::TCGFcalc myCGF( 1, Pbar, TK );
    double TClow = subst.lower_T;
    real CPg[12] = {}; // CGcalcFugPure reads 12 coefficients
    for (unsigned int i = 0; i < 12 && i < subst.parameters.size(); i++)
    {
        CPg[i] = subst.parameters[i];
    }

    myCGF.CGcalcFugPure( (TClow/*+273.15*/), CPg, FugProps );

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (TK) * log( FugProps[0] );
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (Pbar);
    // back correction
    // the standard state has no fugacity coefficient: remove RT ln(phi) added above, unless the caller keeps it
    if (cancel_fugacity)
        tps.gibbs_energy -= R_CONSTANT * (TK) * log(Fug/Pbar);

    return tps;
}

}
