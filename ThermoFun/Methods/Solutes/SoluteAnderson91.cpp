// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "Methods/Solutes/SoluteAnderson91.h"

#include "Common/Exception.h"
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/Constants.h>

namespace ThermoFun {

auto thermoPropertiesAqSoluteAN91(real TK, real Pbar, const SoluteReference& reference, const PropertiesSolventAD&wpr,  const PropertiesSolventAD& wp) -> ThermoPropertiesSubstanceAD
{
    auto T = TK;
    ThermoPropertiesSubstanceAD tps;
    double T298 = reference.T;
    double G298 = reference.G;
    double H298 = reference.H;
    double S298 = reference.S;
    double V298 = reference.V; // J/bar
    double Cp298 = reference.Cp;

    double ALPw298 = wpr.Alpha.val();
    double BETw298 = wpr.Beta.val()*1e5; // 1/bar
    double dALPdTw298 = wpr.dAldT.val();
    double RHOw298 = wpr.density.val()/1000; // g/cm3

    auto RHOw = wp.density/1000; // g/cm3
    auto dRHOdT = wp.densityT/1000;
    auto ALPw = wp.Alpha;
    auto BETw = wp.Beta*1e5; // 1/bar
    auto dALPdTw = wp.dAldT;

    auto G_bb = H298 - T*S298 + Cp298/(T298*dALPdTw298)*(ALPw298*(T - T298) + log(RHOw/RHOw298));
    auto G = G298 - S298*(T-T298) + Cp298/(T298*dALPdTw298)*(ALPw298*(T - T298) + log(RHOw/RHOw298));
    auto S = S298 - (Cp298*(ALPw298 - ALPw))/(T298*dALPdTw298);
    real V(V298); //-(Cp298*ALPw)/(T298*dALPdTw298); // the model does not include a volume term!
    auto Cp = T*(Cp298*dALPdTw)/(T298*dALPdTw298);
    auto H = G_bb + T*S298;

    tps.gibbs_energy     = G;
    tps.volume           = V;
    tps.entropy          = S;
    tps.heat_capacity_cp = Cp;
    tps.enthalpy         = H;
    tps.internal_energy  = tps.enthalpy - Pbar*tps.volume;
    tps.helmholtz_energy = tps.internal_energy - T*tps.entropy;

    return tps;
}
}
