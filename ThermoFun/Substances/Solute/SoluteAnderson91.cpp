#include "Substances/Solute/SoluteAnderson91.h"

#include "Common/Exception.h"
#include "Substance.h"
#include "ThermoParameters.h"
#include "ThermoProperties.h"

namespace ThermoFun {

auto thermoPropertiesAqSoluteAN91(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Substance subst, const PropertiesSolvent &wpr,  const PropertiesSolvent& wp) -> ThermoPropertiesSubstance
{
    auto T = TK;
    ThermoPropertiesSubstance tps;
    auto tpsr = subst.thermoReferenceProperties();
    double T298 = subst.referenceT();
    double G298 = tpsr.gibbs_energy.val;
    double H298 = tpsr.enthalpy.val;
    double S298 = tpsr.entropy.val;
    double V298 = tpsr.volume.val; // J/bar
    double Cp298 = tpsr.heat_capacity_cp.val;

    double ALPw298 = wpr.Alpha.val;
    double BETw298 = wpr.Beta.val*1e5; // 1/bar
    double dALPdTw298 = wpr.dAldT.val;
    double RHOw298 = wpr.density.val/1000; // g/cm3

    auto RHOw = wp.density/1000; // g/cm3
    auto dRHOdT = wp.densityT/1000;
    auto ALPw = wp.Alpha;
    auto BETw = wp.Beta*1e5; // 1/bar
    auto dALPdTw = wp.dAldT;

    auto G_bb = H298 - T*S298 + Cp298/(T298*dALPdTw298)*(ALPw298*(T - T298) + log(RHOw/RHOw298));
    auto G = G298 - S298*(T-T298) + Cp298/(T298*dALPdTw298)*(ALPw298*(T - T298) + log(RHOw/RHOw298));
    auto S = S298 - (Cp298*(ALPw298 - ALPw))/(T298*dALPdTw298);
    Reaktoro_::ThermoScalar V(V298); //-(Cp298*ALPw)/(T298*dALPdTw298); // the model does not include a volume term!
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
