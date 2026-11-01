// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "WaterElectroBradleyPitzer79.h"
#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/ThermoProperties.h>

namespace ThermoFun {

/// The dielectric constant of water of Bradley and Pitzer (1979): D = D1000 + C ln((B + P)/(B + 1000)), with T in K and P in bar, and its
/// analytical derivatives. The constants: U1 to U9.
auto electroPropertiesWaterBradleyPitzer79(const Reaktoro_::Pass& pass, const std::vector<double>& constants) -> ElectroPropertiesSolventAD
{
    const double U1 = constants[0], U2 = constants[1], U3 = constants[2], U4 = constants[3], U5 = constants[4], U6 = constants[5], U7 = constants[6], U8 = constants[7], U9 = constants[8];
    const real T = pass.T;
    const real P = pass.P / bar_to_Pa;

    const real D1000 = U1*exp(U2*T + U3*T*T);
    const real rate = U2 + 2.0*U3*T;
    const real D1000T = D1000*rate;
    const real D1000TT = D1000*(rate*rate + 2.0*U3);
    const real C = U4 + U5/(U6 + T);
    const real CT = -U5/((U6 + T)*(U6 + T));
    const real CTT = 2.0*U5/((U6 + T)*(U6 + T)*(U6 + T));
    const real B = U7 + U8/T + U9*T;
    const real BT = -U8/(T*T) + U9;
    const real BTT = 2.0*U8/(T*T*T);

    const real invP = 1.0/(B + P), inv1000 = 1.0/(B + 1000.0);
    const real L = log((B + P)/(B + 1000.0));
    const real difference = invP - inv1000;

    const real eps = D1000 + C*L;
    const real epsT = D1000T + CT*L + C*BT*difference;
    const real epsP = C*invP;
    const real epsTT = D1000TT + CTT*L + 2.0*CT*BT*difference + C*BTT*difference - C*BT*BT*(invP*invP - inv1000*inv1000);
    const real epsTP = CT*invP - C*BT*invP*invP;
    const real epsPP = -C*invP*invP;

    const real epsilon2 = eps*eps;
    ElectroPropertiesSolventAD wep;
    // the pressure derivatives are per bar
    wep.epsilon   = eps;
    wep.epsilonT  = epsT;
    wep.epsilonP  = epsP;
    wep.epsilonTT = epsTT;
    wep.epsilonTP = epsTP;
    wep.epsilonPP = epsPP;
    wep.bornZ = -1.0/eps;
    wep.bornY = epsT/epsilon2;
    wep.bornQ = epsP/epsilon2/bar_to_Pa; // from 1/bar to 1/Pa
    wep.bornX = epsTT/epsilon2 - 2.0*wep.bornY*wep.bornY*eps;
    return wep;
}

}
