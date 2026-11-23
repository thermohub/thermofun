#ifndef SOLUTEHKFGEMS
#define SOLUTEHKFGEMS
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/Solutes/SoluteInputs.h>


namespace ThermoFun {

struct ElectroPropertiesSubstance;
struct FunctionG;

/// Returns the thermodynamic properties of a substance using the HKF EOS
/// @ref Tanger and Helgeson (1988)
/// @param T temparature (K)
/// @param P pressure (Pa)
/// @param species aqueous species instance
/// @param aes electro-chemical properties of the substance
/// @param wes electro-chemical properties of the solvent
/// @param born if given, gets the Born terms of the totals
auto thermoPropertiesAqSoluteHKFgems(real TC, real Pbar, const HKFInputs& species, const ElectroPropertiesSubstance& aes, const ElectroPropertiesSolventAD& wes, const PropertiesSolventAD& wp, HKFBornTerms* born = nullptr) -> ThermoPropertiesSubstanceAD;

/// Returns the G function and its derivatives (gShock2 in gems)
/// @ref Shock et al. (1991)
/// @param T temparature (K)
/// @param P pressure (Pa)
/// @param ps solvent properties (i.e. density, alpha, beta, epsilon, etc.)
auto gShok2(real TC, real Pbar, const PropertiesSolventAD&ps, const HKFModelParameters& constants) -> FunctionG;

/// Returns the electro-chemical properties of a substance (omeg92 in GEMS)
/// @ref Johnson et al. (1991)
/// @param g structure holding the g function values and its derivatives
/// @param species instance of the species
auto omeg92(FunctionG g, const HKFInputs& species) -> ElectroPropertiesSubstance;

}


#endif // SOLUTEHKFGEMS

