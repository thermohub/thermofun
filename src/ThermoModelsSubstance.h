#ifndef THERMOMODELSUBSTANCE_H
#define THERMOMODELSUBSTANCE_H

// TCorrPT includes
#include "Substance.h"
//#include "ThermoProperties.h"

namespace TCorrPT {

// Forward declarations
//class Database;

class ThermoModelsSubstance
{
public:
    /// Construct a ThermoModelsSubstance instance with given Sunstance instance
    explicit ThermoModelsSubstance(const Substance& substance);

    /// Calculate the thermodynamic properties.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param reaction The name of the reaction
    auto thermoProperties (double T, double P) -> ThermoPropertiesSubstance;

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

    /// Calculate the thermodynamic properties.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param reaction The name of the reaction
    auto thermoProperties (double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};



} // namespace TCorrPT

#endif // THERMOMODELSUBSTANCE_H
