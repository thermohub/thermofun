#ifndef DATABASE_H
#define DATABASE_H

// C++ includes
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace ChemicalFun
{
class ElementKey;
struct ElementValues;
}

namespace ThermoFun {

// Forward declarations
class Substance;
class Reaction;
class Element;

using ElementsMap   = std::map<std::string, Element>;
using SubstancesMap = std::map<std::string, Substance>;
using ReactionsMap  = std::map<std::string, Reaction>;


/**
 * @brief The Database class stores maps of elements, substances and reactions. A database instance can be used to create a ThermoEngine instance
 * which can be further used to calculate the standard thermodynamic properties of substances and reactions at T and P
 */
class Database
{
public:
    /// Construct default database instance
    Database();

    /**
     * @brief Construct a new Database object
     *
     * @param filename name/path of the file or a string containing a ThermoDataSet in JSON format
     */
    explicit Database(std::string filename);

    /**
     * @brief Database constructs a database instance from a vector of records in json format
     * Records with the same symbol will be overwritten!
     * @param jsonRecords vector of records in JSON string format
     * @param _label, optional, (element, substance, reactions),
     * used when the vector of records are of one type and do not contain themselves the key "_label"
     */
    Database(std::vector<std::string> jsonRecords, std::string _label);

    /// Assign a Database instance to this instance
    auto operator=(Database other) -> Database&;

    /// Construct a copy of an Database instance
    Database(const Database& other);

    /**
     * @brief appendData append records to the database from a file.
     * Records with the same symbol will be overwritten!
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

    /// Add an Element instance in the database. If the element with the symbol exists the record will not be added.
    auto addElement(const Element& element) -> void;

    /// Sets an Element in the database. If the element with the symbol exists the record will be overwritten.
    auto setElement(const Element& element) -> void;

    /// Add an Element instance in the database from a JSON string. If the element with the symbol exists the record will not be added.
    auto addElement(const std::string& jsonElement) -> void;

    /// Sets an Element in the database from a JSON string. If the element with the symbol exists the record will be overwritten.
    auto setElement(const std::string& jsonElement) -> void;

    /// Add a map of Elements in the database. If the element with the symbol exists the record will not be added.
    auto addMapElements(const ElementsMap& elements) -> void;

    /// Add a map of Elements in the database. If the element with the symbol exists the record will be overwritten.
    auto setMapElements(const ElementsMap& elements) -> void;

    /// Add a Substance instance in the database. If the substance with the symbol exists the record will not be added.
    auto addSubstance(const Substance& substance) -> void;

    /// Add a Substance instance in the database from a JSON string. If the substance with the symbol exists the record will not be added.
    auto addSubstance(const std::string& jsonSubstance) -> void;

    /// Sets a substance in the database. If the substance with the symbol exists the record will be overwritten.
    auto setSubstance(const Substance& substance) -> void;

    /// Sets a substance in the database  from a JSON string. If the substance with the symbol exists the record will be overwritten.
    auto setSubstance(const std::string& jsonSubstance) -> void;

    /// Add a map of Substances in the database. If the substance with the symbol exists the record will not be added.
    auto addMapSubstances(const SubstancesMap& substances) -> void;

    /// Set a map of Substances in the database. If the substance with the symbol exists the record will be overwritten.
    auto setMapSubstances(const SubstancesMap& substances) -> void;

    /// Add a Reaction instance in the database. If the reaction with the symbol exists the record will not be added.
    auto addReaction(const Reaction& reaction) -> void;

    /// Set a reaction in the database. If the reaction with the symbol exists the record will be overwritten.
    auto setReaction(const Reaction& reaction) -> void;

    /// Add a Reaction instance in the database from a JSON string. If the reaction with the symbol exists the record will not be added.
    auto addReaction(const std::string& jsonReaction) -> void;

    /// Set a reaction in the database from a JSON string. If the reaction with the symbol exists the record will be overwritten.
    auto setReaction(const std::string& jsonReaction) -> void;

    /// Calculates the reaction record parameters, based on the defined method and available data
//    auto calcParametersReactions( ) -> void;

    /// Add a map of Reactions in the database.  If the reaction with the symbol exists the record will not be added.
    auto addMapReactions(const ReactionsMap& reactions) -> void;

    /// Sets a map of Reactions in the database.  If the reaction with the symbol exists the record will be overwritten.
    auto setMapReactions(const ReactionsMap& reactions) -> void;

    /// Return all elements in the database
    auto getElements() const -> std::vector<Element>;

    /// Return all substances in the database
    auto getSubstances() const -> std::vector<Substance>;

    /// Return all reactions in the database
    auto getReactions() const -> std::vector<Reaction>;

    /// Return list of all elements in the database
    auto getElementsList() const -> std::vector<std::string>;

    /// Return list of all substances in the database
    auto getSubstancesList() const -> std::vector<std::string>;

    /// Return list og all reactions in the database
    auto getReactionsList() const -> std::vector<std::string>;

    /// Returns the map of elements in the database
    auto mapElements() const -> const ElementsMap&;

    /// Returns the map of substances in the database
    auto mapSubstances() const -> const SubstancesMap&;

    /// Returns the map of reactions in the database
    auto mapReactions() const -> const ReactionsMap&;

    /// Returns the number of elements in the databse
    auto numberOfElements() const -> size_t;

    /// Returns the number of substances in the databse
    auto numberOfSubstances() const -> size_t;

    /// Returns the number of reactions in the database
    auto numberOfReactions() const -> size_t;

    /// Return a element in the database
    auto getElement(std::string symbol) const -> const Element&;

    /// Return a substance in the database
    auto getSubstance(std::string symbol) const -> const Substance&;

    /// Return a reactions in the database
    auto getReaction(std::string symbol) const -> const Reaction&;

    /// Reference to the element in the database
    auto element(std::string symbol) -> Element&;

    /// Reference to the substance in the database
    auto substance(std::string symbol) -> Substance&;

    /// Reference to the reaction in the database
    auto reaction(std::string symbol) -> Reaction&;

    /// Check if the database contains a given element
    /// @param symbol The name of the element
    auto containsElement(std::string symbol) const -> bool;

    /// Check if the database contains a given substance
    /// @param symbol The name of the substance
    auto containsSubstance(std::string symbol) const -> bool;

    /// Check if the database contains a given reaction
    /// @param symbol The name of the reaction
    auto containsReaction(std::string symbol) const -> bool;

    /// Pareses a given substance formula present in the database
    /// @param formula
    /// @return map of Elements and coefficients
    auto parseSubstanceFormula(std::string formula) const -> std::map<Element, double>;

    /// Pareses a given substance formula present in the database
    /// @param formula
    /// @return elemental entropy
    auto elementalEntropyFormula(std::string formula) const -> double;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif // DATABASE_H
