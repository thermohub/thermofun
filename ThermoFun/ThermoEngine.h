#ifndef THERMO_H
#define THERMO_H

#include <string>
#include <memory>
#include <map>
#include <vector>

namespace ThermoFun
{

// Forward declarations
class Database;
class Solvent;
class Substance;
class Reaction;
class Element;
struct WorkPreferences;
struct ThermoPropertiesSubstance;
struct ThermoPropertiesReaction;
struct ElectroPropertiesSolvent;
struct PropertiesSolvent;

struct EnginePreferences
{
    bool enable_memoize = true;
    size_t max_cache_size = 1e6; // if 0 unlimited cache
    int solventState = 0; // 0: liquid; 1: vapor
    std::string waterTripleProperties = "Helgeson_Kirkham_1974"; // NEA_HGK or NEA_IAPWS // use NEA_ to be consistent with thier standard themrodynamic properties of H2O ar 25 C and 1 bar.
    std::string solventSymbol = "H2O@"; // default
    bool fallback_to_reference_properties = true; // leave properties as is given in substance record database, if no functions to calculate them are defined or there are missing data in the calculation chain
    bool apply_pressure_correction_to_gas_props = false; // making the pressure/fuagcity correction in the G0, H0, ... properties of gases
};


/**
 * @brief The ThermoEngine class calculates the thermodynamic properties of the substances
 * from the internal database. It also calculates the electro-chemical and other physico-chemical
 * properties of the solvent
 */
class ThermoEngine
{
    friend class Interface;

public:
    /**
     * @brief Construct a new Thermo Engine object
     *
     * @param filename name/path of the file or a string containing a ThermoDataSet in JSON format
     */
    explicit ThermoEngine(const std::string filename);

    /// Construct a Thermo instance with given Database instance
    ThermoEngine(const Database &database);

    /// Construct a copy of an ThermoEngine instance
    ThermoEngine(const ThermoEngine &other);

    /// Sets the symbol of the solvent which is used to calculate properties using the thermo instance
    auto setSolventSymbol(const std::string solvent_symbol) -> void;

    /// Sets the preferences used by ThermoEngine
    auto setPreferences(const EnginePreferences preferences) -> void;

    /// Retireves a copy of the curent prefrences
    auto preferences() -> EnginePreferences&;

    /// Returns the symbol of the solvent which is used to calculate properties using the thermo instance
    auto solventSymbol() -> std::string&;

    /**
     * @brief setThermoPreferences
     * @param prefs
     */
    auto setThermoPreferences(const WorkPreferences prefs) -> void;

    /// Returns the instance of the database present inside thermo
    auto database() const -> const Database &;

    /**
     * @brief appendData append records to the database from a file
     * @param filename name/path of the file or a string containing a ThermoDataSet in JSON format
     */
    auto appendData(std::string filename) -> void;

    /**
     * @brief appendData append records to the database from a vector of JSON strings
     * Records with the same symbol will be overwritten!
     * @param jsonRecords vector of records in JSON string format
     * @param _label, optional, (element, substance, reactions),
     * used when the vector of records are of one type and do not contain themselves the key "_label"
     */
    auto appendData(std::vector<std::string> jsonRecords, std::string _label) -> void;

    // Substance
    /// Calculate the thermodynamic properties of a substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param substance The symbol of the substance
    auto thermoPropertiesSubstance(double T, double &P, std::string substance) const -> ThermoPropertiesSubstance;

    /// Calculate the thermodynamic properties of a substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param substance substance object
    auto thermoPropertiesSubstance(double T, double &P, const Substance& substance) const -> ThermoPropertiesSubstance;

    /// Calculate the electro-chemical properties of a substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param solvent The symbol of the substance solvent
    /// @param state The solvent state, 0 liquid, 1 gas/vapor,  default -1 determined by the record key "aggregate_state" in the database "4": "AS_AQUEOUS" or "O": "AS_GAS"
    auto electroPropertiesSolvent(double T, double &P, std::string solvent, int state=-1) const -> ElectroPropertiesSolvent;

    /// Calculate the electro-chemical properties of a substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param solvent substance solvent object
    /// @param state The solvent state, 0 liquid, 1 gas/vapor,  default -1 determined by the record key "aggregate_state" in the database "4": "AS_AQUEOUS" or "O": "AS_GAS"
    auto electroPropertiesSolvent(double T, double &P, const Substance& solvent, int state=-1) const -> ElectroPropertiesSolvent;

    /// Calculate the physical properties of a substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param solvent The symbol of the substance solvent
    /// @param state The solvent state, 0 liquid, 1 gas/vapor,  default -1 determined by the record key "aggregate_state" in the database "4": "AS_AQUEOUS" or "O": "AS_GAS"
    auto propertiesSolvent(double T, double &P, std::string solvent, int state=-1) const -> PropertiesSolvent;

    /// Calculate the physical properties of a substance.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param solvent substance solvent object
    /// @param state The solvent state, 0 liquid, 1 gas/vapor,  default -1 determined by the record key "aggregate_state" in the database "4": "AS_AQUEOUS" or "O": "AS_GAS"
    auto propertiesSolvent(double T, double &P, const Substance& solvent, int state=-1) const -> PropertiesSolvent;

    // Reaction
    /// Calculate the thermodynamic properties of a reaction.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param reaction The symbol of the reaction
    auto thermoPropertiesReaction(double T, double &P, std::string reaction) const -> ThermoPropertiesReaction;

    /// Calculate the thermodynamic properties of a reaction.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param reaction the reaction object
    auto thermoPropertiesReaction(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction;

    /// Calculate the thermodynamic properties of a reaction from the substances participating in the reaction.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param reaction The symbol of the reaction
    auto thermoPropertiesReactionFromReactants(double T, double &P, std::string reaction) const -> ThermoPropertiesReaction;

    /// Calculate the thermodynamic properties of a reaction from the substances participating in the reaction.
    /// @param T The temperature value (in units of K)
    /// @param P The pressure value (in units of Pa)
    /// @param reaction the reaction object
    auto thermoPropertiesReactionFromReactants(double T, double &P, const Reaction& reaction) const -> ThermoPropertiesReaction;

    /// Pareses a given substance formula present in the database
    /// @param formula
    /// @return map of Elements and coefficients
    auto parseSubstanceFormula(std::string formula) const -> std::map<Element, double>;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif // THERMO_H
