#ifndef SOLUTEHKFREAKTORO
#define SOLUTEHKFREAKTORO

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

// Forward declarations
//class AqueousSpecies;
struct ElectroPropertiesSubstance;
//struct WaterThermoState;

/// Calculate the function g of the HKF model.
auto functionG(Reaktoro::Temperature T, Reaktoro::Pressure P, const PropertiesSolvent &wts) -> FunctionG;

/// Calculate the electrostatic state of the aqueous species using the g-function state.
auto speciesElectroStateHKF(const FunctionG& g, Substance species) -> ElectroPropertiesSubstance;

/// Calculate the electrostatic state of the aqueous species using the HKF model.
//auto speciesElectroStateHKF(Reaktoro::Temperature T, Reaktoro::Pressure P/*, const AqueousSpecies& species*/) -> ElectroPropertiesSubstance;

auto speciesThermoStateSoluteHKF(Reaktoro::Temperature T, Reaktoro::Pressure P, Substance species, const ElectroPropertiesSubstance& aes, const ElectroPropertiesSolvent& wes) -> ThermoPropertiesSubstance;

auto checkTemperatureValidityHKF(Reaktoro::Temperature T, Reaktoro::Pressure P, Substance species) -> void;

}

#endif // SOLUTEHKFREAKTORO

