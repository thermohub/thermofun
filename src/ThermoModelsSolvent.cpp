#include "ThermoModelsSolvent.h"
#include "Solvent/WaterHGKgems.h"

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
auto WaterHGK::thermoPropertiesSolvent(double T, double P) -> ThermoPropertiesSolvent
{
//    STATES state;
    WaterHGKgems water_hgk;
    water_hgk.waterHGK (T, P);
    ThermoPropertiesSolvent thermo_properties_solvent_PT = water_hgk.propertiesSolvent();

    return thermo_properties_solvent_PT;
}

auto WaterHGK::thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance
{
//    STATES state;
    WaterHGKgems water_hgk;
    water_hgk.waterHGK (T, P);
    ThermoPropertiesSubstance thermo_properties_substance_PT = water_hgk.propertiesSubstance();

    return thermo_properties_substance_PT;
}

} // namespace TCorrPT

