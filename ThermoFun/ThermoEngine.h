#ifndef THERMO_H
#define THERMO_H

#include <string>
#include <memory>
#include <map>
#include <vector>

namespace ThermoFun {

// Forward declarations
class Database;
class Solvent;
class Substance;
class Element;
struct ThermoPreferences;
struct ThermoPropertiesSubstance;
struct ThermoPropertiesReaction;
struct ElectroPropertiesSolvent;
struct PropertiesSolvent;

/**
 * @brief The Thermo class mainly calculates the thermodynamic properties of the substances
 * from the internal database. It also calculates the electro-chemical and other physico-chemical
 * properties of the solvent
 */
class ThermoEngine
{
    friend class Interface;
public:

    /// Construct a Thermo instance with given dataset file
    explicit ThermoEngine(const std::string filename);

    /// Construct a Thermo instance with given Database instance
    ThermoEngine(const Database& database);

    /// Construct a copy of an ThermoEngine instance
    ThermoEngine(const ThermoEngine& other);

    /// Sets the symbol of the solvent which is used to calculate properties using the thermo instance
    auto setSolventSymbol(const std::string solvent_symbol) -> void;

    /// Returns the symbol of the solvent which is used to calculate properties using the thermo instance
    auto solventSymbol( ) const -> std::string;

    /// Returns the instance of the database present inside thermo
    auto database() -> const Database;

    /**
     * @brief appendData append records to the database from a file
     * @param filename path to the file with recors
     */
    auto appendData(std::string filename) -> void;

    /**
     * @brief appendData append records to the database from a vector of JSON strings
     * @param jsonRecords vector of records in JSON string format
     */
    auto appendData(std::vector<std::string> jsonRecords) -> void;

    // Substance
    /// Calculate the thermodynamic properties of a substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param substance The symbol of the substance
    auto thermoPropertiesSubstance (double T, double &P, std::string substance) -> ThermoPropertiesSubstance;

    /// Calculate the electro-chemical properties of a substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param substance The symbol of the substance
    auto electroPropertiesSolvent(double T, double &P, std::string solvent) -> ElectroPropertiesSolvent;

    /// Calculate the physical properties of a substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param solvent The symbol of the solvent
    auto propertiesSolvent(double T, double &P, std::string solvent) -> PropertiesSolvent;

    // Reaction
    /// Calculate the thermodynamic properties of a reaction.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param reaction The symbol of the reaction
    auto thermoPropertiesReaction (double T, double &P, std::string reaction) -> ThermoPropertiesReaction;

    /// Calculate the thermodynamic properties of a reaction from the substances participating in the reaction.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param reaction The symbol of the reaction
    auto thermoPropertiesReactionFromReactants (double T, double &P, std::string symbol) -> ThermoPropertiesReaction;

    /// Pareses a given substance formula present in the database
    /// @param formula
    /// @return map of Elements and coefficients
    auto parseSubstanceFormula(std::string formula) -> std::map<Element, double>;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif // THERMO_H
