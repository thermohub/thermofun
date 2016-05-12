#include "ThermoModelsSolvent.h"

//#include "Substance.h"
#include "Solvent/WaterHGKgems.h"
#include "Solvent/WaterHGKreaktoro.h"

namespace TCorrPT {

ThermoModelsSolvent::ThermoModelsSolvent()
{

}

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
//    STATES state;
    WaterHGKgems water_hgk;
    water_hgk.waterHGK (T, P);
    PropertiesSolvent properties_solvent_PT = water_hgk.propertiesSolvent();

    return properties_solvent_PT;
}

auto WaterHGK::thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance
{
//    STATES state;
    WaterHGKgems water_hgk;
    water_hgk.waterHGK (T, P);
    ThermoPropertiesSubstance thermo_properties_substance_PT = water_hgk.thermoPropertiesSubstance();

    return thermo_properties_substance_PT;
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
////    STATES state;
//    WaterHGKgems water_hgk;
//    water_hgk.waterHGK (T, P);
//    PropertiesSolvent properties_solvent_PT = water_hgk.propertiesSolvent();

//    return properties_solvent_PT;
}

auto WaterHGKreaktoro::thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance tps;
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    Reaktoro::WaterThermoState wt = Reaktoro::waterThermoStateHGK(t,p);

    tps = thermoPropertiesWaterHKF(t, p, wt);

    return tps;
}

} // namespace TCorrPT

