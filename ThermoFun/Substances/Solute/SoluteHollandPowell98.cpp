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
    double T298 = subst.referenceT();
    double G298 = tpsr.gibbs_energy.val;
    double H298 = tpsr.enthalpy.val;
    double S298 = tpsr.entropy.val;
    double V298 = tpsr.volume.val; // J/bar
    double Cp298 = tpsr.heat_capacity_cp.val;

//    // props given in HP98 page 314
//    ALPw298 = 0.0002593; // 1/K
//    BETw298 = 0.00004523; // 1/bar
//    dALPdTw298 = 0.0000095714; // 1/K^2
//    RHOw298 = 0.997; // g/cm^3
    double ALPw298 = wpr.Alpha.val;
    double BETw298 = wpr.Beta.val*1e5; // 1/bar
    double dALPdTw298 = wpr.dAldT.val;
    double RHOw298 = wpr.density.val/1000; // g/cm3

    const auto HP98param = subst.thermoParameters().solute_holland_powell98_coeff;

    // EOS coefficients
    if (HP98param.size()<1)
    {
        Exception exception;
        exception.error << "Error in Holland and Powell aq solute model";
        exception.reason << "There are no model parameters given for "<<subst.symbol() << ".";
        exception.file = __FILE__;
        exception.line = __LINE__;
        RaiseError(exception);
    }


    auto b = HP98param[0];
    auto RHOw = wp.density/1000; // g/cm3
    auto ALPw = wp.Alpha;
    auto BETw = wp.Beta*1e5; // 1/bar
    auto dALPdTw = wp.dAldT;

    if (T <= 500)
        Tprime = T.val;
    else
        Tprime = 500;

    auto G_bb = H298 - T*S298 + Pbar*V298 + b*( T298*T - pow(T298,2)/2 - pow(T,2)/2 ) + ( Cp298 - T298*b )/( T298*dALPdTw298 )*( ALPw298*(T - T298) - BETw298*Pbar + (T/Tprime)*log(RHOw/RHOw298) );
    auto G = G298 - (T - T298)*S298 + Pbar*V298 + b*( T298*T - pow(T298,2)/2 - pow(T,2)/2 ) + ( Cp298 - T298*b )/( T298*dALPdTw298 )*( ALPw298*(T - T298) - BETw298*Pbar + (T/Tprime)*log(RHOw/RHOw298) );
    auto S = S298 - b*(T298 - T) - (Cp298 - T298*b)/(T298*dALPdTw298)*(ALPw298 - log(RHOw/RHOw298)/Tprime - (T/Tprime)*ALPw);
    auto V = V298 + (Cp298 - T298*b)/(T298*dALPdTw298)*( -BETw298 + T/Tprime*BETw );
    auto Cp = T*(b - ((T298*b - Cp298)*(T*dALPdTw))/(T298*dALPdTw298*Tprime));
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
