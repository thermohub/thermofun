#include "ThermoModelsSolvent.h"
#include "ThermoProperties.h"
#include "Substance.h"

//#include "Substance.h"
#include "Substances/Solvent/WaterHGK-JNgems.h"
#include "Substances/Solvent/WaterHGKreaktoro.h"
#include "Substances/Solvent/WaterWP95reaktoro.h"
#include "Substances/Solvent/WaterZhangDuan2005.h"
#include "Substances/Solvent/Reaktoro/WaterUtils.hpp"
#include "Substances/Solvent/Reaktoro/WaterThermoState.hpp"
#include "Substances/Solvent/Reaktoro/WaterThermoStateUtils.hpp"
#include "Common/Exception.h"

namespace ThermoFun {

auto checkModelValidity(double T, double P, double Tmax, double /*Tmin*/, double Pmax, double Pmin, std::string model) -> void
{
    // Check if given temperature is within the allowed range
    if(T < 0 /*Tmin*/ || T > Tmax)
    {
        Exception exception;
        exception.error << "Out of T bound in model " << model;
        exception.reason << "The provided temperature, " << T << " K,"  << "is either negative "
              "or greater than the maximum allowed, " << Tmax << " K.";
        RaiseError(exception);
    }

    // Check if given pressure is within the allowed range
    if( P > Pmax)
    {
        Exception exception;
        exception.error << "Out of P bound in model " << model;
        exception.reason << "The provided pressure, " << P << " Pa,"  << "is greater than the maximum allowed, " << Pmax << " Pa.";
        RaiseError(exception);
    }

    // Check if given pressure is within the allowed range
    if( P < Pmin)
    {
        Exception exception;
        exception.error << "Out of P bound in model " << model;
        exception.reason << "The provided pressure, " << P << " Pa,"  << "is lower than the minimum allowed, " << Pmin << " Pa.";
        RaiseError(exception);
    }
}

//=======================================================================================================
// Calculate the properties of water using the Haar-Gallagher-Kell (1984) equation of state
// References: HAAR L., GALLAGHER J. S., and KELL G. S. Steam Tables, Thermodynamic and Transport Properties
// and Computer Programs for Vapor and Liquid States of Wafer in SI Unites. 1984, Hemisphere Publishing Co.
// Added: DM 08.05.2016
//=======================================================================================================
struct WaterHGK::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterHGK::WaterHGK(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterHGK::propertiesSolvent(double T, double &P, int state, std::string triple) -> PropertiesSolvent
{
    WaterTripleProperties wtr = waterTripleData.at(triple);

    double Pout = P;
    auto ps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        WaterHGKgems water_hgk;
        real t = pass.T - C_to_K;
        real p = pass.P / bar_to_Pa;
        water_hgk.calculateWaterHGKgems(t, p, wtr);
        Pout = p.val() * bar_to_Pa;
        return water_hgk.propertiesWaterHGKgems(state);
    });
    P = Pout;

    // The GEMS model gives densityT = -alpha rho, densityP = beta rho, densityTT = rho (alpha^2 - dalpha/dT) and (analytical,
    // from the third derivative of P with respect to the density in the LVS and HGK regions) densityPP = -rho beta^2 Gamma, with
    // Gamma = rho P_rhorho/P_rho. The mixed second derivative is the exact derivative of the analytical alpha with respect to P:
    // densityTP = d(densityT)/dP (= d(densityP)/dT), and the mixed third derivative is d(densityTT)/dP.
    if (Reaktoro_::kWithAutodiff)
    {
        ps.densityTP.val = ps.densityT.ddp;
        ps.densityTP.ddt = ps.densityTT.ddp;
        ps.densityTP.sta = ps.densityT.sta;
    }
    else
        ps.densityTP.sta = {Reaktoro_::Status::notdefined, "densityTP is the derivative of densityT: not calculated (built without autodiff)"};

    return ps;
}

auto WaterHGK::thermoPropertiesSubstance(double T, double &P, int state, std::string triple) -> ThermoPropertiesSubstance
{
    WaterTripleProperties wtr = waterTripleData.at(triple);

    double Pout = P;
    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        WaterHGKgems water_hgk;
        real t = pass.T - C_to_K;
        real p = pass.P / bar_to_Pa;
        water_hgk.calculateWaterHGKgems(t, p, wtr);
        Pout = p.val() * bar_to_Pa;
        return water_hgk.thermoPropertiesWaterHGKgems(state);
    });
    P = Pout;

    return tps;
}

//=======================================================================================================
// Calculate the properties of water using the Haar-Gallagher-Kell (1984) equation of state as
// implemented in Reaktoro
// References:
// Added: DM 12.05.2016
//=======================================================================================================

struct WaterHGKreaktoro::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterHGKreaktoro::WaterHGKreaktoro(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterHGKreaktoro::propertiesSolvent(double T, double &P, int state) -> PropertiesSolvent
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T)).val();

    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return propertiesWaterHGKreaktoro(waterThermoStateHGK(pass.T, pass.P, state));
    });
}

auto WaterHGKreaktoro::thermoPropertiesSubstance(double T, double &P, int state, std::string tripple) -> ThermoPropertiesSubstance
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T)).val();

    WaterTripleProperties wtr = waterTripleData.at(tripple);

    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesWaterHGKreaktoro(pass.T, waterThermoStateHGK(pass.T, pass.P, state), wtr);
    });
}

//=======================================================================================================
// Calculate the properties of water using the Wagner and Pruss (1995) equation of state as
// implemented in Reaktoro
// References: Wagner, W., Prub, A. The IAPWS formulation 1995 for the thermodynamic properties of ordinary
// water substance for general and scientific use. J. Phys. Chem. Ref. Data, 2002 31(2):387–535.
// Added: DM 12.05.2016
//=======================================================================================================

struct WaterWP95reaktoro::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterWP95reaktoro::WaterWP95reaktoro(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterWP95reaktoro::propertiesSolvent(double T, double &P, int state) -> PropertiesSolvent
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T)).val();

    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return propertiesWaterWP95reaktoro(waterThermoStateWagnerPruss(pass.T, pass.P, state));
    });
}

auto WaterWP95reaktoro::thermoPropertiesSubstance(double T, double &P, int state, std::string tripple) -> ThermoPropertiesSubstance
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T)).val();

    WaterTripleProperties wtr = waterTripleData.at(tripple);

    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesWaterWP95reaktoro(pass.T, waterThermoStateWagnerPruss(pass.T, pass.P, state), wtr);
    });
}

//=======================================================================================================
// Calculate the properties of water using the Zhang and Duan (2005) equation of state
// References:
// Added: DM 21.07.2016
//=======================================================================================================

struct WaterZhangDuan2005::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterZhangDuan2005::WaterZhangDuan2005(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterZhangDuan2005::propertiesSolvent(double T, double P, int /*state*/) -> PropertiesSolvent
{
    checkModelValidity(T, P, 2273.15, 273.15, 3e10, 1e8, "Zhang and Duan (2005) H2O model.");

    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return propertiesWaterZhangDuan2005(pass);
    });
}

auto WaterZhangDuan2005::thermoPropertiesSubstance(double T, double P, int /*state*/) -> ThermoPropertiesSubstance
{
    checkModelValidity(T, P, 2273.15, 273.15, 3e10, 1e8, "Zhang and Duan (2005) H2O model.");

    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesWaterZhangDuan2005(pass.T, pass.P / bar_to_Pa); // pressure in bar
    });

    // only the volume is calculated by this model
    const Reaktoro_::StatusMessage undefined = {Reaktoro_::Status::notdefined, ""};
    for (auto* x : {&tps.gibbs_energy, &tps.helmholtz_energy, &tps.internal_energy, &tps.enthalpy,
                    &tps.entropy, &tps.heat_capacity_cp, &tps.heat_capacity_cv})
        x->sta = undefined;
    return tps;
}

} // namespace ThermoFun

