#ifndef THERMOCALCULATIONS_H
#define THERMOCALCULATIONS_H

#include <string>

namespace ThermoFun {

// Forward declarations
struct Database;
struct Substance;
struct Reaction;
struct ThermoPropertiesSubstance;
struct ThermoPropertiesReaction;
struct ElectroPropertiesSolvent;
struct PropertiesSolvent;

// Substance
/// Calculate the thermodynamic properties of a substance.
/// @param T The temperature value (in units of C)
/// @param P The pressure value (in units of bar)
/// @param substance The symbol of the substance
auto calcThermoPropertiesSubstance (double T, double &P, Substance substance, Substance solvent) -> ThermoPropertiesSubstance;

/// Calculate the electro-chemical properties of a substance.
/// @param T The temperature value (in units of C)
/// @param P The pressure value (in units of bar)
/// @param substance The symbol of the substance
auto calcElectroPropertiesSolvent(double T, double &P, Substance solvent) -> ElectroPropertiesSolvent;

/// Calculate the physical properties of a substance.
/// @param T The temperature value (in units of C)
/// @param P The pressure value (in units of bar)
/// @param solvent The symbol of the solvent
auto calcPropertiesSolvent(double T, double &P, Substance solvent) -> PropertiesSolvent;

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

//    /// Calculate the standard molar volume of a substance (in units of m3/mol).
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

// Reaction
/// Calculate the thermodynamic properties of a reaction.
/// @param T The temperature value (in units of C)
/// @param P The pressure value (in units of bar)
/// @param reaction The symbol of the reaction
auto thermoPropertiesReaction (double T, double &P, Reaction reaction, Substance solvent) -> ThermoPropertiesReaction;

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



} // namespace ThermoFun

#endif // THERMOCALCULATIONS_H
