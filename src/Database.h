#ifndef DATABASE_H
#define DATABASE_H

// C++ includes
#include <memory>
#include <string>
#include <vector>
#include <map>

#include "ejdb/bson.h"

namespace ThermoFun {

// Forward declarations
class Substance;
class Reaction;
class Element;

using ElementsMap   = std::map<std::string, Element>;
using SubstancesMap = std::map<std::string, Substance>;
using ReactionsMap  = std::map<std::string, Reaction>;

/**
 * @brief The Database class stores maps of substances and reactions. A database instance can be used to create a themro instance
 * which can be further used to calculate the standard themrodynamic properties of substances and reactions at T and P
 */
class Database
{
public:
    /// Construct default database instance
    Database();

    /// Construct a database instance by parsing a "json", "yam", "xml" file
    /// containg the exported substances and reactions
    /*explicit*/ Database(std::string filename);

    /**
     * @brief Database constructs a database instace from a vector of substances in bson format
     * @param bsonSubstances vector of substances in bson format
     * see BSONIO
     */
    Database(std::vector<bson> bsonSubstances);

    /// Assign a Database instance to this instance
    auto operator=(Database other) -> Database&;

    /// Construct a copy of an Database instance
    Database(const Database& other);

    /// Add an Element instance in the database.
    auto addElement(const Element& element) -> void;

    /// Sets a Element in the database. If substance exists the record will be overwriten
    auto setElement(const Element& element) -> void;

    /// Add a map of Elements in the database.
    auto addMapElements(const ElementsMap& elements) -> void;

    /// Add an Substance instance in the database.
    auto addSubstance(const Substance& substance) -> void;

    /// Sets a substance in the database. If substance exists the record will be overwriten
    auto setSubstance(const Substance& substance) -> void;

    /// Add a map of Substances in the database.
    auto addMapSubstances(const SubstancesMap& substances) -> void;

    /// Add an Reaction instance in the database.
    auto addReaction(const Reaction& reaction) -> void;

    /// Sets a reaction in the database. If reaction exists the record will be overwriten
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
    auto numberOfElements() -> int;

    /// Returns the number of substances in the databse
    auto numberOfSubstances() -> int;

    /// Returns the number of reactions in the database
    auto numberOfReactions() -> int;

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
