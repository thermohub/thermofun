#ifndef DATABASE_H
#define DATABASE_H

// C++ includes
#include <memory>
#include <string>
#include <vector>

// TCorrPT includes
#include "Substance.h"
#include "Reaction.h"

//#include "ReadFiles.h"
//#include "bson.h"
//#include "bsonio/v_json.h"
#include "ejdb/bson.h"

namespace TCorrPT {

// Forward declarations
//class Substance;
//class Reaction;
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
    explicit Database(std::string filename);

    /**
     * @brief Database constructs a database instace from a vector of substances in bson format
     * @param bsonSubstances vector of substances in bson format
     * see BSONIO
     */
    Database(vector<bson> bsonSubstances);

    /// Add an Substance instance in the database.
    auto addSubstance(const Substance& substance) -> void;

    /// Add an Reaction instance in the database.
    auto addReaction(const Reaction& reaction) -> void;

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

    /**
     * @brief setAqSubstanceSolventSymbol sets the solvent symbol that will be use for calculating the solute porperties
     * @param substance_symbol for which substance
     * @param solvent_symbol
     */
    auto setAqSubstanceSolventSymbol(std::string substance_symbol, std::string solvent_symbol) -> void;

    /**
     * @brief setAllAqSubstanceSolventSymbol sets a solvent symbol to all solutes
     * @param solvent_symbol
     */
    auto setAllAqSubstanceSolventSymbol(std::string solvent_symbol) -> void;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace TCorrPT

#endif // DATABASE_H
