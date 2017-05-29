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

using SubstancesMap = std::map<std::string, Substance>;
using ReactionsMap = std::map<std::string, Reaction>;

//struct bson;

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

    /// Add an Substance instance in the database.
    auto addSubstance(const Substance& substance) -> void;

    /// Add a map of Substances in the database.
    auto addMapSubstances(const SubstancesMap& substances) -> void;

    /// Add an Reaction instance in the database.
    auto addReaction(const Reaction& reaction) -> void;

    /// Add a map pf Reactions in the database.
    auto addMapReactions(const ReactionsMap& reactions) -> void;

    /// Return all substance in the database
    auto getSubstances() -> std::vector<Substance>;

    /// Return all reactions in the database
    auto getReactions() -> std::vector<Reaction>;

    /// Returns the number of substances in the databse
    auto numberOfSubstances() -> int;

    /// Returns the number of reactions in the database
    auto numberOfReactions() -> int;

    /// Return a substance in the database
    auto getSubstance(std::string symbol) const -> const Substance&;

    /// Return a reactions in the database
    auto getReaction(std::string symbol) const -> const Reaction&;

    /// Check if the database contains a given substance
    /// @param symbol The name of the substance
    auto containsSubstance(std::string symbol) const -> bool;

    /// Check if the database contains a given reaction
    /// @param symbol The name of the reaction
    auto containsReaction(std::string symbol) const -> bool;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif // DATABASE_H
