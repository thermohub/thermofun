#ifndef THERMOMODELSSOLVENT_H
#define THERMOMODELSSOLVENT_H

#include <memory>

// TCorrPT includes
//#include "Substance.h"
//#include "ThermoProperties.h"

namespace TCorrPT {

// forward declarations
struct Substance;
struct PropertiesSolvent;
struct ThermoPropertiesSubstance;

class ThermoModelsSolvent
{
public:
    ThermoModelsSolvent();


};


class WaterHGK
{
public:
    /// Construct a default AqueousSpecies instance
    WaterHGK();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit WaterHGK(const Substance& substance);

    /// Calculate the thermodynamic properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param reaction The name of the reaction
    auto propertiesSolvent (double T, double P) -> PropertiesSolvent;

    auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class WaterHGKreaktoro
{
public:
    /// Construct a default AqueousSpecies instance
    WaterHGKreaktoro();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit WaterHGKreaktoro(const Substance& substance);

    /// Calculate the thermodynamic properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param reaction The name of the reaction
    auto propertiesSolvent (double T, double P) -> PropertiesSolvent;

    auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace TCorrPT

#endif // THERMOMODELSSOLVENT_H
