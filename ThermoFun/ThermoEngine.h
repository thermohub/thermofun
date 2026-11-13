// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#ifndef THERMO_H
#define THERMO_H
#include <ThermoFun/Engine/Consistency.h>
#include <ThermoFun/Engine/DetailedResult.h>
#include <ThermoFun/Engine/Sensitivity.h>
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
    size_t max_cache_size = 100000; // results kept in all the caches together (about 0.7 kB each); 0: no limit
    int solvent_state = 0; // not read by the calculations (kept for the old API): the state of water comes from its record (H2O(g) is vapour) or from the state argument of propertiesSolvent and electroPropertiesSolvent
    std::string solvent_symbol = "H2O@"; // the solvent of the aqueous species; if the database has no such record, the engine uses the water solvent it has (chosen when the engine is made or a setting changes, not changed by a calculation)
    bool fallback_to_reference_properties = false; // leave properties as is given in substance record database, if no functions to calculate them are defined or there are missing data in the calculation chain, value undefined only if not present in the database record
    bool apply_pressure_correction_to_gas_props = true; // a gas whose record prescribes a method for the pressure (a fluid equation of state, IdealGas): G has RT ln(P/Pr), and S has -R ln(P/Pr). False: the state at Pr. The same on every path: alone, as a reactant, and in a substance defined by a reaction
    bool cancel_fugacity_coefficient_in_gas_props = false; // fluid equations of state (CORK, PRSV, CG, SRK, PR78, STP): true removes RT ln(phi) from G; the residual terms of H and S stay. With both options at their defaults G(T, P) = G(T, Pr) + RT ln(P/Pr) + RT ln(phi), so dG/dP = V. Without the pressure term and with this false, steam from the HGK equation has RT ln(P/1 bar) removed from G
    bool round_to_uncertainty = false; // round the values of the calculated properties to the decimals of their uncertainties (err) with the NEA TDB rules (the derivatives are not rounded); only for properties with an error > 0
    int uncertainty_significant_digits = 2; // significant digits of the (rounded up) uncertainties, used if round_to_uncertainty is true (NEA TDB tables give one or two)
    model::PropertySource property_source = model::PropertySource::Record; // where the properties of a record come from: Record (what the record says), Direct (its methods), Reaction (the defining reaction of a substance), Reactants (the reactants of a reaction). A source that a record cannot use is ignored for it; setPropertySource replaces it for one symbol
    std::map<std::string, std::string> method_implementation; // method id -> implementation, for example {"HKF88": "reaktoro"}; it replaces the implementation of the records
    bool propagate_parameter_errors = false; // also propagate the uncertainties of the parameters of the methods, with those of the reference values and the covariances of the records (u^2 = J S J^T). Without it, only the uncertainties of the reference values propagate. Costs two extra calculations per uncertain value

    /// G(T, Pr) for a gas: no RT ln(P/Pr) and no RT ln(phi) in G. For a code that adds these terms itself.
    /// The same as apply_pressure_correction_to_gas_props = false and cancel_fugacity_coefficient_in_gas_props = true,
    /// which were the defaults before version 1.0.
    auto useStandardStateOfGases() -> EnginePreferences&
    {
        apply_pressure_correction_to_gas_props = false;
        cancel_fugacity_coefficient_in_gas_props = true;
        return *this;
    }

    bool operator==(const EnginePreferences&) const = default;
};

struct EngineConventions {
    model::EnergyReference energy_reference = model::EnergyReference::BensonHelgeson;
    model::StandardPressure standard_pressure = model::StandardPressure::OneBar;
    model::ConcentrationScale concentration_scale = model::ConcentrationScale::Molality;
    model::WaterReference water_reference = model::WaterReference::Geochemical;

    // The old names, checked. A word set here replaces energy_reference or water_reference at the next call of the engine,
    // and the field is then empty: read the words with apparentPropertiesWord() and waterPropertiesWord().
    std::string apparent_properties;   // e.g. "BENSON_HELGESON", "BERMAN_BROWN"
    std::string water_properties;      // e.g. "GEOCHEMICAL", "STEAM_TABLES"

    /// The old word of energy_reference, for example "BENSON_HELGESON" (review of v2, Q9)
    auto apparentPropertiesWord() const -> std::string;
    /// The old word of water_reference, for example "GEOCHEMICAL"
    auto waterPropertiesWord() const -> std::string;

    bool operator==(const EngineConventions&) const = default;
};



/// How a record is calculated: the steps of a substance or a reaction, down to the records with their own methods.
struct CalculationPath
{
    std::string kind;                     ///< "substance" or "reaction"
    std::string symbol;
    std::string source;                   ///< "methods", "reaction", "reactants", "reference", "water" or "hydrogen_ion"
    std::vector<std::string> methods;     ///< the methods the record has
    double coefficient = 1.0;             ///< in the reaction above, if there is one
    std::string role;                     ///< "", "defining_reaction", "reactant" or "solvent"
    std::vector<CalculationPath> steps;   ///< what it needs
    std::string problem;                  ///< why it cannot be calculated, if so
};

/**
 * @brief The ThermoEngine class calculates the thermodynamic properties of the substances
 * from the internal database. It also calculates the electro-chemical and other physico-chemical
 * properties of the solvent
 *
 * Threads: one engine for each thread. A calculation writes the results and settings of the engine, so an engine must not
 * be shared by threads. A copy (constructor or assignment) has its own results and settings, and shares the records.
 */
/// Defined since 1.0: the volumes of the results (volume, reaction_volume) are in m3/mol; before, in J/bar
#define THERMOFUN_VOLUME_SI 1

class ThermoEngine
{
    friend class Interface;
    friend class ParameterHandle;

public:
    /**
     * @brief Construct a new Thermo Engine object
     *
     * @param filename name/path of the file or a string containing a ThermoDataSet in JSON format
     */
    explicit ThermoEngine(const std::string filename);

    /// Construct a Thermo instance with given Database instance
    ThermoEngine(const Database &database);

    /// Construct a copy of an ThermoEngine instance: its own settings and caches, the same records (shared, copy-on-write)
    ThermoEngine(const ThermoEngine &other);

    /// Copy assignment: a copy, as the copy constructor makes (before, the two engines shared their settings and caches)
    auto operator=(const ThermoEngine& other) -> ThermoEngine&;
    ThermoEngine(ThermoEngine&&) noexcept = default;
    auto operator=(ThermoEngine&&) noexcept -> ThermoEngine& = default;

    /// Where the properties of one substance or reaction come from. It replaces the preference property_source for that symbol.
    /// A substance and its defining reaction can have the same symbol, and each has its own choice:
    ///   Reaction: the substance comes from its defining reaction. The reaction with this symbol does not change.
    ///   Reactants: the reaction comes from its reactants. The substance with this symbol does not change.
    ///   Direct: both come from their own methods.
    ///   Record: removes both choices.
    /// So the substance from its methods and the reaction from its reactants is Direct, then Reactants.
    auto setPropertySource(const std::string& symbol, model::PropertySource source) -> void;

    /// The same with a word: "record", "direct", "reaction" or "reactants". An unknown word raises an error with the words allowed.
    auto setPropertySource(const std::string& symbol, const std::string& source) -> void;

    /// The source of a symbol: its own, else the preference of the engine.
    auto propertySource(const std::string& symbol) const -> model::PropertySource;

    /// Sets the symbol of the solvent which is used to calculate properties using the thermo instance
    auto setSolventSymbol(const std::string solvent_symbol) -> void;

    /// Sets the preferences used by ThermoEngine
    auto setPreferences(const EnginePreferences preferences) -> void;

    /// Sets the conventions used by ThermoEngine
    auto setConventions(const EngineConventions conventions) -> void;

    /// Returns the symbol of the solvent which is used to calculate properties using the thermo instance
    auto solventSymbol() -> std::string&;

    /// The saturation pressure of water at T (K), in Pa: what P = 0 means in a calculation. From the water record of the
    /// engine, else from the Wagner-Pruss curve. An error at or above the critical temperature of water.
    auto saturationPressure(double T) const -> double;

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

    /// What each method of a substance changes, at each (T[i], P[i]) in K and Pa, in SI. T or P can have one value: it goes
    /// with each value of the other. A substance calculated without methods of its own (water, from its defining reaction)
    /// has no methods: its total is all in `other`.
    auto contributions(const std::string& substance, const std::vector<double>& T, const std::vector<double>& P) const -> std::vector<MethodContributions>;

    /// The result of a calculation, and how it came: the source, the methods in order and the conversions applied.
    /// How a substance, or a reaction if there is no such substance, is calculated with the sources of this engine. It calculates nothing.
    auto calculationPath(const std::string& symbol) const -> CalculationPath;

    /// @param P The pressure value (in units of Pa); 0 means the saturation pressure, which is written back
    auto thermoPropertiesSubstanceDetailed(double T, double &P, const std::string& substance) const -> DetailedPropertiesSubstance;

    /// What the results depend on: the version of ThermoFun, the format and dataset of the loaded data, the SHA-256 of all records,
    /// and the implementations that the preferences ask for.
    auto snapshot() const -> CalculationSnapshot;

    /// The exact derivatives of the properties of a substance by the given parameters, through its defining reaction and the other species of that reaction.
    /// Results: G, H, S, Cp, V, U, A. A pressure of 0 is the saturation pressure, written to P. The results are in the canonical convention, and a change of
    /// convention does not change them.
    auto parameterSensitivities(double T, double& P, const std::string& symbol, const std::vector<ParameterRef>& parameters) const -> SensitivityResult;

    /// The same for a reaction. Results: logK, lnK, dG, dH, dS, dCp, dV.
    auto reactionParameterSensitivities(double T, double& P, const std::string& symbol, const std::vector<ParameterRef>& parameters) const -> SensitivityResult;

    /// A handle on one parameter of a record: see ParameterRef. An unknown parameter is an error.
    auto parameter(const std::string& symbol, const std::string& method, const std::string& parameter) -> ParameterHandle;

    /// The handles of all the values that can be fitted, of the substance and of the reaction with this symbol.
    auto parameters(const std::string& symbol) -> std::vector<ParameterHandle>;

    /// A database with the current values of the handles.
    auto commitParameters() const -> Database;

    /// Gives these parameters a covariance, in SI, row after row: their uncertainties, and one covariance block for each record. The covariance of parameters of
    /// two different records has no place in the format: it is dropped, and the function returns the number of the values dropped.
    auto setParameterCovariance(const std::vector<ParameterRef>& parameters, const std::vector<double>& covariance) -> std::size_t;

    /// The number of results kept in the caches.
    auto numberOfCachedResults() const -> std::size_t;

    /// The records that can be calculated in two ways, and the difference between the ways at each pair (T in K, P in Pa):
    /// a substance from its methods and from its defining reaction, a reaction from its methods and from its reactants.
    /// Information for the owners of the data.
    auto consistencyReport(const std::vector<std::pair<double, double>>& TP, const ConsistencyOptions& options = {}) const -> std::vector<ConsistencyItem>;

    /// Parses a given substance formula present in the database
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
