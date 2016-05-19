//#include "ThermoProperties.h"
#include "WaterHGKreaktoro.h"
#include "Reaktoro/WaterConstants.hpp"

namespace TCorrPT {

auto thermoPropertiesWaterHGKreaktoro(Reaktoro::Temperature T, const Reaktoro::WaterThermoState& wt) -> ThermoPropertiesSubstance
{
    // Auxiliary data from Helgeson and Kirkham (1974), on page 1098
    const auto Ttr =  273.16;                   // unit: K
    const auto Str =  15.1320 * cal_to_J; // unit: J/(mol*K)
    const auto Gtr = -56290.0 * cal_to_J; // unit: J/mol
    const auto Htr = -68767.0 * cal_to_J; // unit: J/mol
    const auto Utr = -67887.0 * cal_to_J; // unit: J/mol
    const auto Atr = -55415.0 * cal_to_J; // unit: J/mol

    const auto Sw = Reaktoro::waterMolarMass * wt.entropy;         // unit: J/(mol*K)
    const auto Hw = Reaktoro::waterMolarMass * wt.enthalpy;        // unit: J/mol
    const auto Uw = Reaktoro::waterMolarMass * wt.internal_energy; // unit: J/mol

    // Calculate the standard molal thermodynamic properties of the aqueous species
    const auto S  = Sw + Str;
    const auto H  = Hw + Htr;
    const auto U  = Uw + Utr;
    const auto G  = Hw - T * (Sw + Str) + Ttr * Str + Gtr;
    const auto A  = Uw - T * (Sw + Str) + Ttr * Str + Atr;
    const auto V  = wt.volume * Reaktoro::waterMolarMass * 100000; // unit J/bar
    const auto Cp = wt.cp * Reaktoro::waterMolarMass;
    const auto Cv = wt.cv * Reaktoro::waterMolarMass;

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

auto electroPropertiesWaterJNreaktoro(const Reaktoro::WaterElectroState& wts) -> ElectroPropertiesSolvent
{
    ElectroPropertiesSolvent eps;

    eps.bornN = wts.bornN;
    eps.bornQ = wts.bornQ;
    eps.bornU = wts.bornU;
    eps.bornX = wts.bornX;
    eps.bornY = wts.bornY;
    eps.bornZ = wts.bornZ;

    eps.epsilon   = wts.epsilon;
    eps.epsilonP  = wts.epsilonP;
    eps.epsilonPP = wts.epsilonPP;
    eps.epsilonT  = wts.epsilonT;
    eps.epsilonTP = wts.epsilonTP;
    eps.epsilonTT = wts.epsilonTT;

    return eps;
}

auto propertiesWaterHGKreaktoro(const Reaktoro::WaterThermoState& wt) -> PropertiesSolvent
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

    return state;
}

auto saturatedWaterVaporPressureHGK(double t) -> double
{
    double  pl, psHGK, v, w, b, q, z;
    int i=-1;
    double a[8] ={ -.78889166e1,  .25514255e1, -.6716169e1,  .33239495e2,
                   -.10538479e3,  .17435319e3, -.14839348e3, .48631602e2};
    if (t <= 314.0e0)
    {
        pl    = 6.3573118e0 - 8858.843e0 / t + 607.56335e0 * pow(t,-0.6e0);
        psHGK = 0.1e0 * exp(pl);
    }
    else
    {
        v = t / 647.25e0;
        w = fabs(1.0e0 - v);
        b = 0.0e0;
        while (++i <= 7)
        {
            z  = i + 1;
            b += a[i] * pow(w,((z + 1.0e0) / 2.0e0));
        }
        q = b / v;
        psHGK = 22.093e0 * exp(q);
    }
    return(psHGK*1e1);
}

} // end namespace TCorrPT
