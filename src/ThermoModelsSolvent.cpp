#include "ThermoModelsSolvent.h"
#include "Solvent/WaterHGK.h"

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
    ThermoPropertiesSolvent   thermo_properties_solvent_PT = WaterHGKProperties (T, P);

    return thermo_properties_solvent_PT;
}

} // namespace TCorrPT

