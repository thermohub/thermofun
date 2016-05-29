#include "ThermoModelsSolvent.h"

//#include "Substance.h"
#include "Solvent/WaterHGK-JNgems.h"
#include "Solvent/WaterHGKreaktoro.h"
#include "Solvent/WaterWP95reaktoro.h"
#include "Solvent/Reaktoro/WaterUtils.hpp"

namespace TCorrPT {

//=======================================================================================================
// Calculate the properties of water using the Haar-Gallagher-Kell (1984) equation of state
// References:
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
auto WaterHGK::propertiesSolvent(double T, double P, int state) -> PropertiesSolvent
{
    WaterHGKgems water_hgk;

    water_hgk.calculateWaterHGKgems(T, P);

    return water_hgk.propertiesWaterHGKgems(state);
}

auto WaterHGK::thermoPropertiesSubstance(double T, double P, int state) -> ThermoPropertiesSubstance
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
auto WaterHGKreaktoro::propertiesSolvent(double T, double P) -> PropertiesSolvent
{
//    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    if (P==0) p = Reaktoro::Pressure(Reaktoro::waterSaturatedPressureWagnerPruss(t).val);
    Reaktoro::WaterThermoState wt = Reaktoro::waterThermoStateHGK(t, p);

    return propertiesWaterHGKreaktoro(wt);
}

auto WaterHGKreaktoro::thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance
{
//    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    if (P==0) p = Reaktoro::Pressure(Reaktoro::waterSaturatedPressureWagnerPruss(t).val);
    Reaktoro::WaterThermoState wt = Reaktoro::waterThermoStateHGK(t, p);

    return thermoPropertiesWaterHGKreaktoro(t, wt);
}

//=======================================================================================================
// Calculate the properties of water using the Haar-Gallagher-Kell (1984) equation of state as
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
auto WaterWP95reaktoro::propertiesSolvent(double T, double P) -> PropertiesSolvent
{
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    if (P==0) p = Reaktoro::Pressure(Reaktoro::waterSaturatedPressureWagnerPruss(t).val);

    Reaktoro::WaterThermoState wt = Reaktoro::waterThermoStateWagnerPruss(t, p);

    return propertiesWaterWP95reaktoro(wt);
}

auto WaterWP95reaktoro::thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    if (P==0) p = Reaktoro::Pressure(Reaktoro::waterSaturatedPressureWagnerPruss(t).val);

    Reaktoro::WaterThermoState wt = Reaktoro::waterThermoStateWagnerPruss(t, p);

    return thermoPropertiesWaterWP95reaktoro(t, wt);
}

} // namespace TCorrPT

