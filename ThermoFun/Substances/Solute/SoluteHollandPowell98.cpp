#include "Substances/Solute/SoluteHollandPowell98.h"

#include "Common/Exception.h"
#include "Substance.h"
#include "ThermoParameters.h"
#include "ThermoProperties.h"

namespace ThermoFun {

auto thermoPropertiesAqSoluteHP98(Reaktoro_::Temperature TK, Reaktoro_::Pressure Pbar, Substance subst, const PropertiesSolvent &wpr,  const PropertiesSolvent& wp) -> ThermoPropertiesSubstance
{
    auto T = TK;
    double Tprime = 0.0;
    ThermoPropertiesSubstance tps;
    auto tpsr = subst.thermoReferenceProperties();
    auto T298 = subst.referenceT();
    auto G298 = tpsr.gibbs_energy;
    auto H298 = tpsr.enthalpy;
    auto S298 = tpsr.entropy;
    auto V298 = tpsr.volume; // J/bar
    auto Cp298 = tpsr.heat_capacity_cp;

//    // props given in HP98 page 314
//    ALPw298 = 0.0002593; // 1/K
//    BETw298 = 0.00004523; // 1/bar
//    dALPdTw298 = 0.0000095714; // 1/K^2
//    RHOw298 = 0.997; // g/cm^3
    auto ALPw298 = wpr.Alpha;
    auto BETw298 = wpr.Beta;
    auto dALPdTw298 = wpr.dAldT;
    auto RHOw298 = wpr.density;

    const auto HP98param = subst.thermoParameters().solute_holland_powell98_coeff;

    // EOS coefficients
    auto b = HP98param[0];

    auto RHOw = wp.density;
    auto ALPw = wp.Alpha;
    auto BETw = wp.Beta;
    auto dALPdTw = wp.dAldT;

    if (T <= 500)
        Tprime = T.val;
    else
        Tprime = 500;

    auto G_bb = H298 - T*S298 + Pbar*V298 + b*( T298*T - pow(T298,2)/2 - pow(T,2)/2 ) + ( Cp298 - T298*b )/( T298*dALPdTw298 )*( ALPw298*(T - T298) - BETw298*Pbar + (T/Tprime)*log(RHOw/RHOw298) );
    auto G = G298 - (T - T298)*S298 + Pbar*V298 + b*( T298*T - pow(T298,2)/2 - pow(T,2)/2 ) + ( Cp298 - T298*b )/( T298*dALPdTw298 )*( ALPw298*(T - T298) - BETw298*Pbar + (T/Tprime)*log(RHOw/RHOw298) );
    auto S = S298 - b*(T298 - T) - (Cp298 - T298*b)/(T298*dALPdTw298)*(ALPw298 - log(RHOw/RHOw298)/Tprime - (T/Tprime)*ALPw);
    auto So = S298 - b*(T298 - T) - ((Cp298 - T298*b)*(ALPw298 + log(RHOw/RHOw298)/Tprime))/(T298*dALPdTw298);
    auto V = V298 + (Cp298 - T298*b)/(T298*dALPdTw298)*( -BETw298 + T/Tprime*BETw );
    auto Cp = b/T - (Cp298 - T298*b)/(T298*dALPdTw298)*( dALPdTw - 2*ALPw/Tprime - T/Tprime*dALPdTw );
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
