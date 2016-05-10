#ifndef THERMOMODELSSOLVENT_H
#define THERMOMODELSSOLVENT_H

// TCorrPT includes
#include "Substance.h"
//#include "ThermoProperties.h"

namespace TCorrPT {

class ThermoModelsSolvent
{
public:
    ThermoModelsSolvent();


};


/// integration of empirical heat capacity equation Cp=f(T);
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
    auto thermoPropertiesSolvent (double T, double P) -> ThermoPropertiesSolvent;

    auto thermoPropertiesSubstance(double T, double P) -> ThermoPropertiesSubstance;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace TCorrPT

#endif // THERMOMODELSSOLVENT_H
