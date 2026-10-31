// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef THERMO_H
#define THERMO_H
#include <ThermoFun/Engine/Consistency.h>
#include <ThermoFun/Engine/DetailedResult.h>
#include <ThermoFun/Engine/Snapshot.h>
#include <ThermoFun/Model/Vocabulary.h>
#include <ThermoFun/ThermoProperties.h>

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

struct EnginePreferences
{
    bool enable_memoize = true;
    size_t max_cache_size = 1e6; // if 0 unlimited cache
    int solvent_state = 0; // 0: liquid; 1: vapor
    std::string solvent_symbol = "H2O@"; // default
    bool fallback_to_reference_properties = false; // leave properties as is given in substance record database, if no functions to calculate them are defined or there are missing data in the calculation chain, value undefined only if not present in the database record
    bool apply_pressure_correction_to_gas_props = false; // making the pressure/fugacity correction in the G0, H0, ... properties of gases
    bool cancel_fugacity_coefficient_in_gas_props = true; // fluid equations of state (CORK, PRSV, CG, SRK, PR78, STP): true removes RT ln(phi) from G (G(T, 1 bar), standard state); false keeps it, so G = G(T, 1 bar) + RT ln(phi), and steam from the HGK equation has RT ln(P/1 bar) removed from G in the same way
    bool round_to_uncertainty = false; // round the values of the calculated properties to the decimals of their uncertainties (err) with the NEA TDB rules (the derivatives are not rounded); only for properties with an error > 0
    int uncertainty_significant_digits = 2; // significant digits of the (rounded up) uncertainties, used if round_to_uncertainty is true (NEA TDB tables give one or two)
    v2::PropertySource property_source = v2::PropertySource::Record; // where the properties of a record come from: Record (what the record says), Direct (its methods), Reaction (the defining reaction of a substance), Reactants (the reactants of a reaction). A source that a record cannot use is ignored for it; setPropertySource replaces it for one symbol
    std::map<std::string, std::string> method_implementation; // method id -> implementation, for example {"HKF88": "reaktoro"}; it replaces the implementation of the records
    bool propagate_parameter_errors = false; // propagate the errors of the reference properties and of the coefficients of the models (given in the database as errors) to the calculated properties; costs two extra calculations per parameter with an error

    bool operator==(const EnginePreferences&) const = default;
};

struct EngineConventions {
    v2::EnergyReference energy_reference = v2::EnergyReference::BensonHelgeson;
    v2::StandardPressure standard_pressure = v2::StandardPressure::OneBar;
    v2::ConcentrationScale concentration_scale = v2::ConcentrationScale::Molality;
    v2::WaterReference water_reference = v2::WaterReference::Geochemical;

    // The old names, checked. A word set here replaces energy_reference or water_reference at the next call of the engine.
    std::string apparent_properties;   // e.g. "BENSON_HELGESON", "BERMAN_BROWN"
    std::string water_properties;      // e.g. "GEOCHEMICAL", "STEAM_TABLES"

    bool operator==(const EngineConventions&) const = default;
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

    /// Where the properties of one substance or reaction come from. It replaces the preference property_source for that
    /// symbol; Record removes this choice.
    auto setPropertySource(const std::string& symbol, v2::PropertySource source) -> void;

    /// The same with a word: "record", "direct", "reaction" or "reactants". An unknown word raises an error with the words allowed.
    auto setPropertySource(const std::string& symbol, const std::string& source) -> void;

    /// The source of a symbol: its own, else the preference of the engine.
    auto propertySource(const std::string& symbol) const -> v2::PropertySource;

    /// Sets the symbol of the solvent which is used to calculate properties using the thermo instance
    auto setSolventSymbol(const std::string solvent_symbol) -> void;

    /// Sets the preferences used by ThermoEngine
    auto setPreferences(const EnginePreferences preferences) -> void;

    /// Sets the conventions used by ThermoEngine
    auto setConventions(const EngineConventions conventions) -> void;

    /// Returns the symbol of the solvent which is used to calculate properties using the thermo instance
    auto solventSymbol() -> std::string&;

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

    using ExplainStep = ThermoFun::ExplainStep;

    /// The methods that calculate a substance at T (K) and P (Pa), in the order they run, with the state after each one.
    /// Empty if no such methods run: water, a substance defined by a reaction, a record without a base method.
    auto explain(double T, double P, std::string substance) const -> std::vector<ExplainStep>;

    /// The result of a calculation, and how it came: the source, the methods in order and the conversions applied.
    /// @param P The pressure value (in units of Pa); 0 means the saturation pressure, which is written back
    auto thermoPropertiesSubstanceDetailed(double T, double &P, const std::string& substance) const -> DetailedPropertiesSubstance;

    /// What the results depend on: the version of ThermoFun, the format and dataset of the loaded data, the SHA-256 of all records,
    /// and the implementations that the preferences ask for.
    auto snapshot() const -> CalculationSnapshot;

    /// The records that can be calculated in two ways, and the difference between the ways at each pair (T in K, P in Pa):
    /// a substance from its methods and from its defining reaction, a reaction from its methods and from its reactants.
    /// Information for the owners of the data.
    auto consistencyReport(const std::vector<std::pair<double, double>>& TP, const ConsistencyOptions& options = {}) const -> std::vector<ConsistencyItem>;

    /// Pareses a given substance formula present in the database
    /// @param formula
    /// @return map of Elements and coefficients
    auto parseSubstanceFormula(std::string formula) const -> std::map<Element, double>;

    /// Return conventions used in the engine
    /// Settings for calculating and outputting properties of a engine 
    /// A change through the reference applies from the next calculation (the cached results are cleared then).
    EngineConventions& conventions();
    const EngineConventions& conventions() const;

    /// Retrieves a copy of the curent preferences
    /// A change through the reference applies from the next calculation (the cached results are cleared then).
    EnginePreferences& preferences();
    const EnginePreferences& preferences() const;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif // THERMO_H
