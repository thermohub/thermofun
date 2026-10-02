#include "Substances/Solute/SoluteHollandPowell98.h"

#include "Common/Exception.h"
#include "Substance.h"
#include "ThermoParameters.h"
#include "ThermoProperties.h"

namespace ThermoFun {

auto thermoPropertiesAqSoluteHP98(real TK, real Pbar, Substance subst, const PropertiesSolventAD&wpr,  const PropertiesSolventAD& wp) -> ThermoPropertiesSubstanceAD
{
    auto T = TK;
    ThermoPropertiesSubstanceAD tps;
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
    double ALPw298 = wpr.Alpha.val();
    double BETw298 = wpr.Beta.val()*1e5; // 1/bar
    double dALPdTw298 = wpr.dAldT.val();
    double RHOw298 = wpr.density.val()/1000; // g/cm3

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

    // T' = T below 500 K (it varies with T, so it carries the derivative), constant 500 K above
    real Tprime = (T <= 500) ? T : real(500.0);

    // k * (alpha298 (T - T298) - beta298 P + u ln(rho/rho298)) is the density term of G, u = T/T' (1 up to 500 K)
    const auto k = ( Cp298 - T298*b )/( T298*dALPdTw298 );
    const auto L = log(RHOw/RHOw298);
    const auto u = T/Tprime;
    const double du = (T <= 500) ? 0.0 : 1.0/500.0; // du/dT
    auto G = G298 - (T - T298)*S298 + Pbar*V298 + b*( T298*T - pow(T298,2)/2 - pow(T,2)/2 ) + k*( ALPw298*(T - T298) - BETw298*Pbar + u*L );
    // S = -dG/dT, V = dG/dP, Cp = T dS/dT and H = G + T S are exact derivatives of G (d ln(rho)/dT = -alpha)
    auto S = S298 - b*(T298 - T) - k*( ALPw298 + du*L - u*ALPw );
    auto V = V298 + k*( -BETw298 + u*BETw );
    auto Cp = T*( b + k*( 2.0*du*ALPw + u*dALPdTw ) );
    auto H = G + T*S;

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
