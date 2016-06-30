#ifndef SOLUTEHKFGEMS
#define SOLUTEHKFGEMS

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

/// Returns the thermodynamic properties of a substance using the HKF EOS
/// @ref Tanger and Helgeson (1988)
/// @param T temparature (K)
/// @param P pressure (Pa)
/// @param species aqueous species instance
/// @param aes electro-chemical properties of the substance
/// @param wes electro-chemical properties of the solvent
auto thermoPropertiesAqSoluteHKFgems(Reaktoro::Temperature T, Reaktoro::Pressure P, Substance species, const ElectroPropertiesSubstance& aes, const ElectroPropertiesSolvent& wes) -> ThermoPropertiesSubstance;

/// Returns the G function and its derivatives (gShock2 in gems)
/// @ref Shock et al. (1991)
/// @param T temparature (K)
/// @param P pressure (Pa)
/// @param ps solvent properties (i.e. density, alpha, beta, epsilon, etc.)
auto gShok2(Reaktoro::Temperature T, Reaktoro::Pressure P, const PropertiesSolvent &ps ) -> FunctionG;

/// Returns the electro-chemical properties of a substance (omeg92 in GEMS)
/// @ref Johnson et al. (1991)
/// @param g structure holding the g function values and its derivatives
/// @param species instance of the species
auto omeg92(FunctionG g, Substance species) -> ElectroPropertiesSubstance;

}


#endif // SOLUTEHKFGEMS

