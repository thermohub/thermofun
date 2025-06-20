#include "WaterWP95reaktoro.h"
#include "Reaktoro/WaterConstants.hpp"
#include "GlobalVariables.h"
#include "Reaktoro/WaterThermoState.hpp"
#include "ThermoProperties.h"

namespace ThermoFun {

auto thermoPropertiesWaterWP95reaktoro(Reaktoro_::Temperature T, const WaterThermoState& wt, const WaterTripleProperties& wtr) -> ThermoPropertiesSubstance
{
    // Auxiliary data from Helgeson and Kirkham (1974), on page 1098
    const auto Ttr =  273.16;                   // unit: K
//    const auto Str =  15.1320 * cal_to_J; // unit: J/(mol*K)
//    const auto Gtr = -56290.0 * cal_to_J; // unit: J/mol
//    const auto Htr = -68767.0 * cal_to_J; // unit: J/mol
//    const auto Utr = -67887.0 * cal_to_J; // unit: J/mol
//    const auto Atr = -55415.0 * cal_to_J; // unit: J/mol

//    const auto Str =  15.1374 * cal_to_J; // unit: J/(mol*K)
//    const auto Gtr = -56279.90 * cal_to_J; // unit: J/mol
//    const auto Htr = -68766.76 * cal_to_J; // unit: J/mol
//    const auto Utr = -67888.19 * cal_to_J; // unit: J/mol
//    const auto Atr = -55416.18 * cal_to_J; // unit: J/mol

    const auto Sw = waterMolarMass * wt.entropy;         // unit: J/(mol*K)
    const auto Hw = waterMolarMass * wt.enthalpy;        // unit: J/mol
    const auto Uw = waterMolarMass * wt.internal_energy; // unit: J/mol

    // Calculate the standard molal thermodynamic properties of the aqueous species
    const auto S  = Sw + wtr.Str;
    const auto H  = Hw + wtr.Htr;
    const auto U  = Uw + wtr.Utr;
    const auto G  = Hw - T * (Sw + wtr.Str) + Ttr * wtr.Str + wtr.Gtr;
    const auto A  = Uw - T * (Sw + wtr.Str) + Ttr * wtr.Str + wtr.Atr;
    const auto V  = wt.volume * waterMolarMass * 100000; // unit J/bar
    const auto Cp = wt.cp * waterMolarMass;
    const auto Cv = wt.cv * waterMolarMass;

    ThermoPropertiesSubstance state;
    state.entropy          = S;
    state.enthalpy         = H;
    state.internal_energy  = U;
    state.gibbs_energy     = G;
    state.helmholtz_energy = A;
    state.volume           = V;
    state.heat_capacity_cp = Cp;
    state.heat_capacity_cv = Cv;

//#ifdef OutputSTEAM_CONVENTION
//    OutputSteamConventionH2OProp("H2OWP95reaktoro.csv", wt);
//#endif

    return state;
}

auto propertiesWaterWP95reaktoro(const WaterThermoState& wt) -> PropertiesSolvent
{
    PropertiesSolvent state;

    state.density    = wt.density;
    state.densityP   = wt.densityP;
    state.densityPP  = wt.densityPP;
    state.densityT   = wt.densityT;
    state.densityTP  = wt.densityTP;
    state.densityTT  = wt.densityTT;
    state.pressure   = wt.pressure;
    state.pressureD  = wt.pressureD;
    state.pressureDD = wt.pressureDD;
    state.pressureT  = wt.pressureT;
    state.pressureTD = wt.pressureTD;
    state.pressureTT = wt.pressureTT;

    state.Alpha      = -wt.densityT/wt.density;
    state.Beta       = wt.densityP/wt.density;
    state.dAldT      = -wt.densityTT/wt.density + state.Alpha*state.Alpha;
//    const auto alphaP = -wt.densityTP/wt.density - alpha*beta;
//    const auto betaP  =  wt.densityPP/wt.density - beta*beta;

//    auto t = Reaktoro::Temperature( wt.temperature.val );
//    waterIdealGas(t, state);

    return state;
}


}
