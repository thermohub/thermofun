#include "Database.h"

// C++ includes
#include <map>
#include <set>
#include <string>
#include <vector>

// TCorrPT includes
#include "Common/Exception.h"

namespace TCorrPT {

namespace {
using SubstancesMap = std::map<std::string, Substance>;
using ReactionsMap = std::map<std::string, Reaction>;

}

auto errorNonExistent(std::string type, std::string name, int line) -> void
{
    Exception exception;
    exception.error << "Cannot get an instance of the " << type << " `" << name << "` in the database.";
    exception.reason << "There is no such " << type << " in the database.";
    exception.line = line;
    RaiseError(exception);
}


struct Database::Impl
{
    /// The set of all aqueous species in the database
    SubstancesMap substances_map;

    /// The set of all gaseous species in the database
    ReactionsMap reactions_map;

    Impl()
    {}

    Impl(std::string filename)
    {
//        // Create the XML document
//        xml_document doc;

//        // Load the xml database file
//        auto result = doc.load_file(filename.c_str());

//        // Check if result is not ok, and then try a built-in database with same name
//        if(!result)
//        {
//            // Search for a built-in database
//            std::string builtin = database(filename);

//            // If not empty, use the built-in database to create the xml doc
//            if(!builtin.empty()) result = doc.load(builtin.c_str());
//        }

//        // Ensure either a database file path was correctly given, or a built-in database
//        if(!result)
//        {
//            std::string names;
//            for(auto const& s : databases()) names += s + " ";
//            RuntimeError("Could not initialize the Database instance with given database name `" + filename + "`.",
//                "This name either points to a non-existent database file, or it is not one of the "
//                "built-in database files in Reaktoro. The built-in databases are: " + names + ".");
//        }

//        // Parse the xml document
//        parse(doc, filename);
    }

    template<typename Key, typename Value>
    auto collectValues(const std::map<Key, Value>& map) -> std::vector<Value>
    {
        std::vector<Value> collection;
        collection.reserve(map.size());
        for(const auto& pair : map)
            collection.push_back(pair.second);
        return collection;
    }

    auto addSubstance(const Substance& substance) -> void
    {
        substances_map.insert({substance.name(), substance});
    }

    auto addReaction(const Reaction& reaction) -> void
    {
        reactions_map.insert({reaction.name(), reaction});
    }

    auto getSubstances() -> std::vector<Substance>
    {
        return collectValues(substances_map);
    }

    auto getReactions() -> std::vector<Reaction>
    {
        return collectValues(reactions_map);
    }

    auto getSubstance(std::string name) -> Substance&
    {
        if(substances_map.count(name) == 0)
            errorNonExistent("substance", name, __LINE__);

        return substances_map.find(name)->second;
    }

    auto getReaction(std::string name) -> Reaction&
    {
        if(reactions_map.count(name) == 0)
            errorNonExistent("reaction", name, __LINE__);

        return reactions_map.at(name);
    }

    auto containsSubstance(std::string name) const -> bool
    {
        return substances_map.count(name) != 0;
    }

    auto containsReaction(std::string name) const -> bool
    {
        return reactions_map.count(name) != 0;
    }

};

Database::Database()
: pimpl(new Impl())
{}

Database::Database(std::string filename)
: pimpl(new Impl(filename))
{}

auto Database::addSubstance(const Substance& substance) -> void
{
    Substance subst;
    pimpl->addSubstance(substance);
}

auto Database::addReaction(const Reaction& reaction) -> void
{
    pimpl->addReaction(reaction);
}

auto Database::getSubstances() -> std::vector<Substance>
{
    return pimpl->getSubstances();
}

auto Database::getReactions() -> std::vector<Reaction>
{
    return pimpl->getReactions();
}

auto Database::getSubstance(std::string name) const -> const Substance&
{
    return pimpl->getSubstance(name);
}

auto Database::getReaction(std::string name) const -> const Reaction&
{
    return pimpl->getReaction(name);
}

auto Database::containsSubstance(std::string name) const -> bool
{
    return pimpl->containsSubstance(name);
}

auto Database::containsReaction(std::string name) const -> bool
{
    return pimpl->containsReaction(name);
}

} // namespace TCorrPT

