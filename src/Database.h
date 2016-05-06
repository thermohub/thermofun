#ifndef DATABASE_H
#define DATABASE_H

// C++ includes
#include <memory>
#include <string>
#include <vector>

// TCorrPT includes
#include "Substance.h"
#include "Reaction.h"

namespace TCorrPT {

// Forward declarations
class Substance;
class Reaction;

class Database
{
public:
    /// Construct default database instance
    Database();

    /// Construct a database instance by parsing a "json", "yam", "xml" file
    /// containg the exported substances and reactions
    // bsonio library should be used here
    explicit Database(std::string filename);

    /// Add an Substance instance in the database.
    auto addSubstance(const Substance& substance) -> void;

    /// Add an Reaction instance in the database.
    auto addReaction(const Reaction& reaction) -> void;

    /// Return all substance in the database
    auto getSubstances() -> std::vector<Substance>;

    /// Return all reactions in the database
    auto getReactions() -> std::vector<Reaction>;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

} // namespace TCorrPT

#endif // DATABASE_H
