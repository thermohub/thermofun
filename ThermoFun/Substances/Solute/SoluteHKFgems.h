#ifndef SOLUTEHKFGEMS
#define SOLUTEHKFGEMS
#include <ThermoFun/ThermoProperties.h>


namespace ThermoFun {

struct ElectroPropertiesSubstance;
struct FunctionG;
class Substance;

/// Returns the thermodynamic properties of a substance using the HKF EOS
/// @ref Tanger and Helgeson (1988)
/// @param T temparature (K)
/// @param P pressure (Pa)
/// @param species aqueous species instance
/// @param aes electro-chemical properties of the substance
/// @param wes electro-chemical properties of the solvent
auto thermoPropertiesAqSoluteHKFgems(real TC, real Pbar, Substance species, const ElectroPropertiesSubstance& aes, const ElectroPropertiesSolventAD& wes, const PropertiesSolventAD& wp) -> ThermoPropertiesSubstanceAD;

/// Returns the G function and its derivatives (gShock2 in gems)
/// @ref Shock et al. (1991)
/// @param T temparature (K)
/// @param P pressure (Pa)
/// @param ps solvent properties (i.e. density, alpha, beta, epsilon, etc.)
auto gShok2(real TC, real Pbar, const PropertiesSolventAD&ps ) -> FunctionG;

/// Returns the electro-chemical properties of a substance (omeg92 in GEMS)
/// @ref Johnson et al. (1991)
/// @param g structure holding the g function values and its derivatives
/// @param species instance of the species
auto omeg92(FunctionG g, Substance species) -> ElectroPropertiesSubstance;

}


#endif // SOLUTEHKFGEMS

