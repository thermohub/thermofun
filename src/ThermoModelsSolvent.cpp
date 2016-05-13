#include "ThermoModelsSolvent.h"

//#include "Substance.h"
#include "Solvent/WaterHGKgems.h"
#include "Solvent/WaterHGKreaktoro.h"

namespace TCorrPT {

//ThermoModelsSolvent::ThermoModelsSolvent()
//{

//}

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
auto WaterHGK::propertiesSolvent(double T, double P) -> PropertiesSolvent
{
    WaterHGKgems water_hgk;

    water_hgk.calculateWaterHGKgems(T, P);

    return water_hgk.propertiesWaterHGKgems();
}

auto WaterHGK::thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance
{
    WaterHGKgems water_hgk;

    water_hgk.calculateWaterHGKgems(T, P);

    return  water_hgk.thermoPropertiesWaterHGKgems();
}

auto WaterHGK::electroPropertiesSolvent(double T, double P) -> ElectroPropertiesSolvent
{
    WaterHGKgems water_hgk;

    water_hgk.calculateWaterHGKgems(T, P);

    return  water_hgk.electroPropertiesWaterHGKgems();
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
    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    Reaktoro::WaterThermoState wt = Reaktoro::waterThermoStateHGK(t,p);

    return propertiesWaterHGKreaktoro(t, p, wt);
}

auto WaterHGKreaktoro::thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance
{
    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    Reaktoro::WaterThermoState wt = Reaktoro::waterThermoStateHGK(t,p);

    return thermoPropertiesWaterHGKreaktoro(t, p, wt);
}

} // namespace TCorrPT

