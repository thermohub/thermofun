#ifndef THERMOMODELSSOLVENT_H
#define THERMOMODELSSOLVENT_H

#include <memory>

namespace TCorrPT {

// forward declarations
struct Substance;
struct PropertiesSolvent;
struct ThermoPropertiesSubstance;
struct ElectroPropertiesSolvent;

class WaterHGK
{
public:
    /// Construct a default AqueousSpecies instance
    WaterHGK();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit WaterHGK(const Substance& substance);

    /// Return the physical properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto propertiesSolvent (double T, double P) -> PropertiesSolvent;

    /// Return the themrodynamic properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance;

    /// Return the electro-chemical properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto electroPropertiesSolvent(double T, double P) -> ElectroPropertiesSolvent;

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

    /// Return the physical properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto propertiesSolvent (double T, double P) -> PropertiesSolvent;

    /// Return the themrodynamic properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance;

    /// Return the electro-chemical properties of the solvent.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto electroPropertiesSolvent(double T, double P) -> ElectroPropertiesSolvent;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace TCorrPT

#endif // THERMOMODELSSOLVENT_H
