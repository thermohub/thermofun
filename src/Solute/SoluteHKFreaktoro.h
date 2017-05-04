#ifndef SOLUTEHKFREAKTORO
#define SOLUTEHKFREAKTORO

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace ThermoFun {

// Forward declarations
struct ElectroPropertiesSubstance;

/// Returns the G function and its derivatives
/// @ref Shock et al. (1991)
/// @param T temparature (K)
/// @param P pressure (Pa)
/// @param ps solvent properties (i.e. density, alpha, beta, epsilon, etc.)
auto functionG(Reaktoro_::Temperature T, Reaktoro_::Pressure P, const PropertiesSolvent &ps) -> FunctionG;

/// Calculate the electrostatic state of the aqueous species using the g-function state.
/// @param g structure holding the g function values and its derivatives
/// @param species instance of the species
auto speciesElectroStateHKF(const FunctionG& g, Substance species) -> ElectroPropertiesSubstance;

/// Returns the thermodynamic properties of a substance using the HKF EOS
/// @ref Tanger and Helgeson (1988)
/// @param T temparature (K)
/// @param P pressure (Pa)
/// @param species aqueous species instance
/// @param aes electro-chemical properties of the substance
/// @param wes electro-chemical properties of the solvent
auto thermoPropertiesAqSoluteHKFreaktoro(Reaktoro_::Temperature T, Reaktoro_::Pressure P, Substance species, const ElectroPropertiesSubstance& aes, const ElectroPropertiesSolvent& wes, const PropertiesSolvent &wp) -> ThermoPropertiesSubstance;

//auto checkTemperatureValidityHKF(Reaktoro::Temperature T, Reaktoro::Pressure P, Substance species) -> void;
}

#endif // SOLUTEHKFREAKTORO

