//#include "ThermoProperties.h"
#include "WaterHGKreaktoro.h"

namespace TCorrPT {


//auto propertiesSolvent(double T, double P) -> PropertiesSolvent
//{
//    PropertiesSolvent ps;

//    auto t = Reaktoro::Temperature(T);
//    auto p = Reaktoro::Pressure(P);

//    Reaktoro::WaterThermoState waterThermoState = Reaktoro::waterThermoStateHGK(t,p);



//    return ps;
//}

//auto thermoPropertiesSolvent( double T, double P) -> ThermoPropertiesSolvent
//{
//    ThermoPropertiesSolvent tps;


//    return tps;
//}

//auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance
//{
//    ThermoPropertiesSubstance tps;
//    auto t = Reaktoro::Temperature(T);
//    auto p = Reaktoro::Pressure(P);

//    Reaktoro::WaterThermoState wt = Reaktoro::waterThermoStateHGK(t,p);

//    tps = thermoPropertiesWaterHKF(T, P, wt);

//    return tps;
//}


auto thermoPropertiesWaterHKF(Reaktoro::Temperature T, Reaktoro::Pressure P, const Reaktoro::WaterThermoState& wt) -> ThermoPropertiesSubstance
{
    // Auxiliary data from Helgeson and Kirkham (1974), on page 1098
    const auto Ttr =  273.16;                   // unit: K
    const auto Str =  15.1320 * cal_to_J; // unit: J/(mol*K)
    const auto Gtr = -56290.0 * cal_to_J; // unit: J/mol
    const auto Htr = -15971.0 * cal_to_J; // unit: J/mol
    const auto Utr = -15766.0 * cal_to_J; // unit: J/mol
    const auto Atr = -55415.0 * cal_to_J; // unit: J/mol

    const auto Sw = waterMolarMass * wt.entropy;         // unit: J/(mol*K)
    const auto Hw = waterMolarMass * wt.enthalpy;        // unit: J/mol
    const auto Uw = waterMolarMass * wt.internal_energy; // unit: J/mol

    // Calculate the standard molal thermodynamic properties of the aqueous species
    const auto S  = Sw + Str;
    const auto H  = Hw + Htr;
    const auto U  = Uw + Utr;
    const auto G  = Hw - T * (Sw + Str) + Ttr * Str + Gtr;
    const auto A  = Uw - T * (Sw + Str) + Ttr * Str + Atr;
    const auto V  = wt.volume * waterMolarMass;
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

    return state;
}
}
