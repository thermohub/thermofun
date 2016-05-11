//#include "ThermoProperties.h"
#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

////
#include "Solvent/Reaktoro/WaterThermoState.hpp"

namespace TCorrPT {


auto propertiesSolvent(double T, double P) -> PropertiesSolvent
{
    PropertiesSolvent ps;

    auto t = Reaktoro::Temperature(T);
    auto p = Reaktoro::Pressure(P);

    Reaktoro::WaterThermoState waterThermoState = Reaktoro::waterThermoStateHGK(t,p);

    return ps;
}

auto thermoPropertiesSolvent( double T, double P) -> ThermoPropertiesSolvent
{
    ThermoPropertiesSolvent tps;


    return tps;
}

auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance
{

}

}
