#ifndef THERMO_H
#define THERMO_H

// TCorrPT includes
#include "Database.h"
#include "ThermoModelsSubstance.h"
#include "ThermoModelsSolvent.h"
#include "ElectroModelsSolvent.h"

namespace TCorrPT {

struct ThermoPreferences
{
    Substance workSubstance;
    MethodGenEoS_Thrift::type method_genEOS;
    MethodCorrT_Thrift::type  method_T;
    MethodCorrP_Thrift::type  method_P;

    unsigned solventState = 0;

    bool isHydrogen = false;
    bool isH2Ovapor = false;
    bool isH2OSolvent = false;
    bool isGasFluid = false;

    string H2OSolventSymbol;
};

/**
 * @brief The Thermo class mainly calculates the themrodynamic properties of the substances
 * from the internal database. It also calculates the electro-chemical and other phisico-chemical
 * properites of the solvent
 */
class Thermo
{
public:
    /// Construct a Thermo instance with given Database instance
    explicit Thermo(const Database& database);

    // Substance
    /// Calculate the thermodynamic properties of a substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param substance The name of the substance
    auto thermoPropertiesSubstance (double T, double &P, std::string substance) -> ThermoPropertiesSubstance;

    /// Calculate the electro-chemical properties of a substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param substance The name of the substance
    auto electroPropertiesSolvent(double T, double &P, std::string substance) -> ElectroPropertiesSolvent;

    /// Calculate the physical properties of a substance.
    /// @param T The temperature value (in units of C)
    /// @param P The pressure value (in units of bar)
    /// @param solvent The name of the solvent
    auto propertiesSolvent(double T, double &P, std::string solvent) -> PropertiesSolvent;

//    /// Calculate the apparent standard molar Gibbs free energy of a substance (in units of J/mol).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param substance The name of the substance
//    auto standardPartialMolarGibbsEnergy(double T, double P, std::string substance) const -> double;

//    /// Calculate the apparent standard molar Helmholtz free energy of a substance (in units of J/mol).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param substance The name of the substance
//    auto standardPartialMolarHelmholtzEnergy(double T, double P, std::string substance) const -> double;

//    /// Calculate the apparent standard molar internal energy of a substance (in units of J/mol).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param substance The name of the substance
//    auto standardPartialMolarInternalEnergy(double T, double P, std::string substance) const -> double;

//    /// Calculate the apparent standard molar enthalpy of a substance (in units of J/mol).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param substance The name of the substance
//    auto standardPartialMolarEnthalpy(double T, double P, std::string substance) const -> double;

//    /// Calculate the standard molar entropies of a substance (in units of J/K).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param substance The name of the substance
//    auto standardPartialMolarEntropy(double T, double P, std::string substance) const -> double;

//    /// Calculate the standard molar volumes of a substance (in units of m3/mol).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param substance The name of the substance
//    auto standardPartialMolarVolume(double T, double P, std::string substance) const -> double;

//    /// Calculate the standard molar isobaric heat capacity of a substance (in units of J/(mol*K)).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param substance The name of the substance
//    auto standardPartialMolarHeatCapacityConstP(double T, double P, std::string substance) const -> double;

//    /// Calculate the standard molar isochoric heat capacity of a substance (in units of J/(mol*K)).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param substance The name of the substance
//    auto standardPartialMolarHeatCapacityConstV(double T, double P, std::string substance) const -> double;

//    // Reaction
//    /// Calculate the ln equilibrium constant of a reaction.
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param reaction The reaction equation
//    auto lnEquilibriumConstant(double T, double P, std::string reaction) -> double;

//    /// Calculate the log equilibrium constant of a reaction.
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param reaction The reaction equation
//    auto logEquilibriumConstant(double T, double P, std::string reaction) -> double;

//    /// Calculate the apparent standard molar Gibbs free energy of a reaction (in units of J/mol).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param reaction The name of the reaction
//    auto standardPartialMolarGibbsEnergyOfReaction(double T, double P, std::string reaction) const -> double;

//    /// Calculate the apparent standard molar Helmholtz free energy of a reaction (in units of J/mol).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param reaction The name of the reaction
//    auto standardPartialMolarHelmholtzEnergyOfReaction(double T, double P, std::string reaction) const -> double;

//    /// Calculate the apparent standard molar internal energy of a reaction (in units of J/mol).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param reaction The name of the reaction
//    auto standardPartialMolarInternalEnergyOfReaction(double T, double P, std::string reaction) const -> double;

//    /// Calculate the apparent standard molar enthalpy of a reaction (in units of J/mol).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param reaction The name of the reaction
//    auto standardPartialMolarEnthalpyOfReaction(double T, double P, std::string reaction) const -> double;

//    /// Calculate the standard molar entropies of a reaction (in units of J/K).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param reaction The name of the reaction
//    auto standardPartialMolarEntropyOfReaction(double T, double P, std::string reaction) const -> double;

//    /// Calculate the standard molar volumes of a reaction (in units of m3/mol).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param reaction The name of the reaction
//    auto standardPartialMolarVolumeOfReaction(double T, double P, std::string reaction) const -> double;

//    /// Calculate the standard molar isobaric heat capacity of a reaction (in units of J/(mol*K)).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param reaction The name of the reaction
//    auto standardPartialMolarHeatCapacityOfReactionConstP(double T, double P, std::string reaction) const -> double;

//    /// Calculate the standard molar isochoric heat capacity of a reaction (in units of J/(mol*K)).
//    /// @param T The temperature value (in units of K)
//    /// @param P The pressure value (in units of Pa)
//    /// @param reaction The name of the reaction
//    auto standardPartialMolarHeatCapacityOfReactionConstV(double T, double P, std::string reaction) const -> double;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;

    auto getThermoPreferences(std::string substance) -> ThermoPreferences;

};

} // namespace TCorrPT

#endif // THERMO_H
