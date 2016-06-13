#ifndef THERMOMODELSUBSTANCE_H
#define THERMOMODELSUBSTANCE_H

// TCorrPT includes
#include "Substance.h"

namespace TCorrPT {

class ThermoModelsSubstance
{
public:
    /// Construct a ThermoModelsSubstance instance with given Sunstance instance
    explicit ThermoModelsSubstance(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class WaterIdealGasWoolley
{
public:
    /// Construct a default AqueousSpecies instance
    WaterIdealGasWoolley();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit WaterIdealGasWoolley(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

class SoluteHKFgems
{
public:
    /// Construct a default AqueousSpecies instance
    SoluteHKFgems();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit SoluteHKFgems(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};


class SoluteHKFreaktoro
{
public:
    /// Construct a default AqueousSpecies instance
    SoluteHKFreaktoro();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit SoluteHKFreaktoro(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};


/// integration of empirical heat capacity equation Cp=f(T);
class EmpiricalCpIntegration
{
public:
    /// Construct a default AqueousSpecies instance
    EmpiricalCpIntegration();

    /// Construct an AqueousSpecies instance from a Species instance
    explicit EmpiricalCpIntegration(const Substance& substance);

    /// Returns the thermodynamic properties of the substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    auto thermoProperties (double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace TCorrPT

#endif // THERMOMODELSUBSTANCE_H
