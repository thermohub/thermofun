#ifndef SOLUTEHKFREAKTORO
#define SOLUTEHKFREAKTORO

#include "Solvent/Reaktoro/WaterThermoStateUtils.hpp"

namespace TCorrPT {

// Forward declarations
//class AqueousSpecies;
struct ElectroPropertiesSubstance;
//struct WaterThermoState;

/// A type used to describe the function g of the HKF model and its partial temperature and pressure derivatives
struct FunctionG
{
    /// The function g at temperature T and pressure P
    Reaktoro::ThermoScalar g;

    /// The first-order partial derivative of function g with respect to temperature
    Reaktoro::ThermoScalar gT;

    /// The first-order partial derivative of function g with respect to pressure
    Reaktoro::ThermoScalar gP;

    /// The second-order partial derivative of function g with respect to temperature
    Reaktoro::ThermoScalar gTT;

    /// The second-order partial derivative of function g with respect to temperature and pressure
    Reaktoro::ThermoScalar gTP;

    /// The second-order partial derivative of function g with respect to pressure
    Reaktoro::ThermoScalar gPP;
};

/// Calculate the function g of the HKF model.
auto functionG(Reaktoro::Temperature T, Reaktoro::Pressure P, const PropertiesSolvent &wts) -> FunctionG;

/// Calculate the electrostatic state of the aqueous species using the g-function state.
auto speciesElectroStateHKF(const FunctionG& g, Substance species) -> ElectroPropertiesSubstance;

/// Calculate the electrostatic state of the aqueous species using the HKF model.
//auto speciesElectroStateHKF(Reaktoro::Temperature T, Reaktoro::Pressure P/*, const AqueousSpecies& species*/) -> ElectroPropertiesSubstance;

auto speciesThermoStateSoluteHKF(Reaktoro::Temperature T, Reaktoro::Pressure P, Substance species, const ElectroPropertiesSubstance& aes, const ElectroPropertiesSolvent& wes) -> ThermoPropertiesSubstance;

}

#endif // SOLUTEHKFREAKTORO

