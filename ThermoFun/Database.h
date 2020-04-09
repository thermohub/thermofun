#ifndef DATABASE_H
#define DATABASE_H

// C++ includes
#include <memory>
#include <string>
#include <vector>
#include <map>

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
     * @brief Database constructs a database instace from a vector of records in json format
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
     * @param _label, oprional, (element, substance, reactions),
     * used when the vector of records are of one type and do not contain themselves the key "_label"
     */
    auto appendData(std::vector<std::string> jsonRecords, std::string _label) -> void;

    /// Add an Element instance in the database.
    auto addElement(const Element& element) -> void;

    /// Sets an Element in the database.
    auto setElement(const Element& element) -> void;

    /// Add a map of Elements in the database. If the element exists the record will be overwritten
    auto addMapElements(const ElementsMap& elements) -> void;

    /// Add an Substance instance in the database. If the substance exists the record will be overwritten
    auto addSubstance(const Substance& substance) -> void;

    /// Sets a substance in the database.
    auto setSubstance(const Substance& substance) -> void;

    /// Add a map of Substances in the database.
    auto addMapSubstances(const SubstancesMap& substances) -> void;

    /// Add an Reaction instance in the database. If the reaction exists the record will be overwritten
    auto addReaction(const Reaction& reaction) -> void;

    /// Sets a reaction in the database.
    auto setReaction(const Reaction& reaction) -> void;

    /// Calculates the reaction record parameters, based on the defined method and available data
//    auto calcParametersReactions( ) -> void;

    /// Add a map pf Reactions in the database.
    auto addMapReactions(const ReactionsMap& reactions) -> void;

    /// Return all elements in the database
    auto getElements() -> std::vector<Element>;

    /// Return all substances in the database
    auto getSubstances() -> std::vector<Substance>;

    /// Return all reactions in the database
    auto getReactions() -> std::vector<Reaction>;

    /// Returns the map of elements in the database
    auto mapElements() const -> const ElementsMap&;

    /// Returns the map of substances in the database
    auto mapSubstances() const -> const SubstancesMap&;

    /// Returns the map of reactions in the database
    auto mapReactions() const -> const ReactionsMap&;

    /// Returns the number of elements in the databse
    auto numberOfElements() -> size_t;

    /// Returns the number of substances in the databse
    auto numberOfSubstances() -> size_t;

    /// Returns the number of reactions in the database
    auto numberOfReactions() -> size_t;

    /// Return a element in the database
    auto getElement(std::string symbol) const -> const Element&;

    /// Return a substance in the database
    auto getSubstance(std::string symbol) const -> const Substance&;

    /// Return a reactions in the database
    auto getReaction(std::string symbol) const -> const Reaction&;

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
    auto parseSubstanceFormula(std::string formula) -> std::map<Element, double>;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif // DATABASE_H
