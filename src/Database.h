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

namespace TCorrPT {

// Forward declarations
class Substance;
class Reaction;
//class bson;

class Database
{
public:
    /// Construct default database instance
    Database();

    /// Construct a database instance by parsing a "json", "yam", "xml" file
    /// containg the exported substances and reactions
    // bsonio library should be used here
    explicit Database(std::string filename);

//    Database(vector<void> *bsonSubstances);

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
    /// @param substance The name of the substance
    auto containsSubstance(std::string symbol) const -> bool;

    /// Check if the database contains a given reaction
    /// @param substance The name of the reaction
    auto containsReaction(std::string symbol) const -> bool;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace TCorrPT

#endif // DATABASE_H
