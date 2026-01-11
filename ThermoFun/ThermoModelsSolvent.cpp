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
    WaterHGKgems water_hgk; T -= C_to_K; P /= bar_to_Pa;

    WaterTripleProperties wtr = waterTripleData.at(triple);
    water_hgk.calculateWaterHGKgems(T, P, wtr); P *= bar_to_Pa;

    return water_hgk.propertiesWaterHGKgems(state);
}

auto WaterHGK::thermoPropertiesSubstance(double T, double &P, int state, std::string triple) -> ThermoPropertiesSubstance
{
    WaterHGKgems water_hgk; T -= C_to_K; P /= bar_to_Pa;

    WaterTripleProperties wtr = waterTripleData.at(triple);

    water_hgk.calculateWaterHGKgems(T, P, wtr); P *= bar_to_Pa;

    return  water_hgk.thermoPropertiesWaterHGKgems(state);
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
    auto t = Reaktoro_::Temperature(T);
    auto p = Reaktoro_::Pressure(P);

    if (P==0) p = Reaktoro_::Pressure(waterSaturatedPressureWagnerPruss(t).val);
    WaterThermoState wt = waterThermoStateHGK(t, p, state); P = p.val;

    return propertiesWaterHGKreaktoro(wt);
}

auto WaterHGKreaktoro::thermoPropertiesSubstance(double T, double &P, int state, std::string tripple) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T);
    auto p = Reaktoro_::Pressure(P);

    if (P==0) p = Reaktoro_::Pressure(waterSaturatedPressureWagnerPruss(t).val);
    WaterThermoState wt = waterThermoStateHGK(t, p, state); P = p.val;

    WaterTripleProperties wtr = waterTripleData.at(tripple);

    return thermoPropertiesWaterHGKreaktoro(t, wt, wtr);
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
    auto t = Reaktoro_::Temperature(T);
    auto p = Reaktoro_::Pressure(P);

    if (P==0) p = Reaktoro_::Pressure(waterSaturatedPressureWagnerPruss(t).val);

    WaterThermoState wt = waterThermoStateWagnerPruss(t, p, state); P = p.val;

    return propertiesWaterWP95reaktoro(wt);
}

auto WaterWP95reaktoro::thermoPropertiesSubstance(double T, double &P, int state, std::string tripple) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T);
    auto p = Reaktoro_::Pressure(P);

    if (P==0) p = Reaktoro_::Pressure(waterSaturatedPressureWagnerPruss(t).val);

    WaterThermoState wt = waterThermoStateWagnerPruss(t, p, state); P = p.val;

    WaterTripleProperties wtr = waterTripleData.at(tripple);

    return thermoPropertiesWaterWP95reaktoro(t, wt, wtr);
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
    auto t = Reaktoro_::Temperature(T);
    auto p = Reaktoro_::Pressure(P); p /= bar_to_Pa;

    checkModelValidity(T, P, 2273.15, 273.15, 3e10, 1e8, "Zhang and Duan (2005) H2O model.");

//    if (P==0) p = Reaktoro_::Pressure(Reaktoro_::waterSaturatedPressureWagnerPruss(t).val);

//    Reaktoro_::WaterThermoState wt = Reaktoro_::waterThermoStateWagnerPruss(t, p, state);

    return propertiesWaterZhangDuan2005(t,p);
}

auto WaterZhangDuan2005::thermoPropertiesSubstance(double T, double P, int /*state*/) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T);
    auto p = Reaktoro_::Pressure(P); p /= bar_to_Pa; // bar

    checkModelValidity(T, P, 2273.15, 273.15, 3e10, 1e8, "Zhang and Duan (2005) H2O model.");

//    if (P==0) p = Reaktoro_::Pressure(Reaktoro_::waterSaturatedPressureWagnerPruss(t).val);

//    Reaktoro_::WaterThermoState wt = Reaktoro_::waterThermoStateWagnerPruss(t, p, state);

    return thermoPropertiesWaterZhangDuan2005(t, p);
}

} // namespace ThermoFun

