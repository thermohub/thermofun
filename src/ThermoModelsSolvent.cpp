#include "ThermoModelsSolvent.h"

//#include "Substance.h"
#include "Solvent/WaterHGK-JNgems.h"
#include "Solvent/WaterHGKreaktoro.h"
#include "Solvent/WaterWP95reaktoro.h"
#include "Solvent/WaterZhangDuan2005.h"
#include "Solvent/Reaktoro/WaterUtils.hpp"

#include "Common/Exception.h"

namespace TCorrPT {

//=======================================================================================================
// Calculate the properties of water using the Haar-Gallagher-Kell (1984) equation of state
// References:
// Added: DM 08.05.2016
//=======================================================================================================

auto checkModelValidity(double T, double P, double Tmax, double Tmin, double Pmax, double Pmin, string model) -> void
{
    // Check if given temperature is within the allowed range
    if(T < 0 /*Tmin*/ || T > Tmax)
    {
        Exception exception;
        exception.error << "Out of T bound in model " << model;
        exception.reason << "The provided temperature, " << T << " C,"  << "is either negative "
              "or greater than the maximum allowed, " << Tmax << " C.";
        RaiseError(exception);
    }

    // Check if given pressure is within the allowed range
    if( P > Pmax)
    {
        Exception exception;
        exception.error << "Out of P bound in model " << model;
        exception.reason << "The provided pressure, " << P << " bar,"  << "is greater than the maximum allowed, " << Pmax << " bar.";
        RaiseError(exception);
    }

    // Check if given pressure is within the allowed range
    if( P < Pmin)
    {
        Exception exception;
        exception.error << "Out of P bound in model " << model;
        exception.reason << "The provided pressure, " << P << " bar,"  << "is lower than the minimum allowed, " << Pmin << " bar.";
        RaiseError(exception);
    }
}



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
auto WaterHGK::propertiesSolvent(double T, double &P, int state) -> PropertiesSolvent
{
    WaterHGKgems water_hgk;

    water_hgk.calculateWaterHGKgems(T, P);

    return water_hgk.propertiesWaterHGKgems(state);
}

auto WaterHGK::thermoPropertiesSubstance(double T, double &P, int state) -> ThermoPropertiesSubstance
{
    WaterHGKgems water_hgk;

    water_hgk.calculateWaterHGKgems(T, P);

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
//    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

    if (P==0) p = Reaktoro_::Pressure(Reaktoro_::waterSaturatedPressureWagnerPruss(t).val);
    Reaktoro_::WaterThermoState wt = Reaktoro_::waterThermoStateHGK(t, p, state);

    P = p.val / bar_to_Pa;

    return propertiesWaterHGKreaktoro(wt);
}

auto WaterHGKreaktoro::thermoPropertiesSubstance(double T, double &P, int state) -> ThermoPropertiesSubstance
{
//    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

    if (P==0) p = Reaktoro_::Pressure(Reaktoro_::waterSaturatedPressureWagnerPruss(t).val);
    Reaktoro_::WaterThermoState wt = Reaktoro_::waterThermoStateHGK(t, p, state);

    P = p.val / bar_to_Pa;

    return thermoPropertiesWaterHGKreaktoro(t, wt);
}

//=======================================================================================================
// Calculate the properties of water using the Wagner and Pruss (1995) equation of state as
// implemented in Reaktoro
// References:
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
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

    if (P==0) p = Reaktoro_::Pressure(Reaktoro_::waterSaturatedPressureWagnerPruss(t).val);

    Reaktoro_::WaterThermoState wt = Reaktoro_::waterThermoStateWagnerPruss(t, p, state);

    P = p.val / bar_to_Pa;

    return propertiesWaterWP95reaktoro(wt);
}

auto WaterWP95reaktoro::thermoPropertiesSubstance(double T, double &P, int state) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

    if (P==0) p = Reaktoro_::Pressure(Reaktoro_::waterSaturatedPressureWagnerPruss(t).val);

    Reaktoro_::WaterThermoState wt = Reaktoro_::waterThermoStateWagnerPruss(t, p, state);

    P = p.val / bar_to_Pa;

    return thermoPropertiesWaterWP95reaktoro(t, wt);
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
auto WaterZhangDuan2005::propertiesSolvent(double T, double &P, int state) -> PropertiesSolvent
{
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P /* * bar_to_Pa*/);

    checkModelValidity(T, P, 2000, 0, 300000, 1000, "Zhang and Duan (2005) H2O model.");

//    if (P==0) p = Reaktoro_::Pressure(Reaktoro_::waterSaturatedPressureWagnerPruss(t).val);

//    Reaktoro_::WaterThermoState wt = Reaktoro_::waterThermoStateWagnerPruss(t, p, state);

//    P = p.val / bar_to_Pa;

    return propertiesWaterZhangDuan2005(t,p);
}

auto WaterZhangDuan2005::thermoPropertiesSubstance(double T, double &P, int state) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P /*/ 10 *//* * bar_to_Pa*/); // bar

//    if (P==0) p = Reaktoro_::Pressure(Reaktoro_::waterSaturatedPressureWagnerPruss(t).val);

//    Reaktoro_::WaterThermoState wt = Reaktoro_::waterThermoStateWagnerPruss(t, p, state);

//    P = p.val / bar_to_Pa;

    return thermoPropertiesWaterZhangDuan2005(t, p);
}

} // namespace TCorrPT

