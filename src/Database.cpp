#include "Database.h"

// C++ includes
#include <map>
#include <set>
#include <string>
#include <vector>

// ThermoFun includes
#include "Common/Exception.h"
#include "ReadFiles.h"

// ThermoFun includes
#include "Substance.h"
#include "Reaction.h"

//#include "bsonio/v_json.h"
//#include "bsonio/nejdb.h"

namespace ThermoFun {

namespace {

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

    char type_ = bsonio::FileTypes::Undef_;

    Impl()
    {}

    Impl(std::string filename)
    {
        bsonio::FJson file (filename);
        type_ = file.Type();

        switch( type_ )
        {
          case bsonio::FileTypes::Json_:
              parseJson( filename );
               break;
          case bsonio::FileTypes::Yaml_:
//               loadYaml( bobj );
               break;
          case bsonio::FileTypes::XML_:
//               loadXml( bobj );
               break;
        }
    }

    Impl(vector<bson> bsonSubstances)
    {
        string kbuf;
        flog.open(parsinglogfile, ios::trunc); flog.close();

        for (int i=0; i<bsonSubstances.size(); i++)
        {

            bsonio::bson_to_key( bsonSubstances[i].data, label, kbuf );

            if (kbuf == "substance")
            {
                Substance substance = parseSubstance(bsonSubstances[i].data);
                substances_map[substance.symbol()] = substance;
            } else
            if (kbuf == "reaction")
            {
                    //                      Reaction reaction = parseReaction(bso);
                    //                      reactions_map[reaction.symbol()] = reaction;
            } else
            {
                Exception exception;
                exception.error << "Unknown JSON type " << kbuf << " ";
                exception.reason << "The JSON object needs to be a substance or reaction.";
                exception.line = __LINE__;
                RaiseError(exception);
            }
        }
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
        substances_map.insert({substance.symbol(), substance});
    }

    auto addMapSubstances(const SubstancesMap& substances) -> void
    {
        substances_map = substances;
    }

    auto addReaction(const Reaction& reaction) -> void
    {
        reactions_map.insert({reaction.symbol(), reaction});
    }

    auto addMapReactions(const ReactionsMap& reactions) -> void
    {
        reactions_map = reactions;
    }

    auto getSubstances() -> std::vector<Substance>
    {
        return collectValues(substances_map);
    }

    auto getReactions() -> std::vector<Reaction>
    {
        return collectValues(reactions_map);
    }

    auto numberOfSubstances() -> int
    {
        return collectValues(substances_map).size();
    }

    auto numberOfReactions() -> int
    {
        return collectValues(reactions_map).size();
    }

    auto getSubstance(std::string symbol) -> Substance&
    {
        if(substances_map.count(symbol) == 0)
            errorNonExistent("substance", symbol, __LINE__);

        return substances_map.find(symbol)->second;
    }

    auto getReaction(std::string symbol) -> Reaction&
    {
        if(reactions_map.count(symbol) == 0)
            errorNonExistent("reaction", symbol, __LINE__);

        return reactions_map.at(symbol);
    }

    auto containsSubstance(std::string symbol) const -> bool
    {
        return substances_map.count(symbol) != 0;
    }

    auto containsReaction(std::string symbol) const -> bool
    {
        return reactions_map.count(symbol) != 0;
    }

    /// Parses the JSON file and puts the data into the internal data structure
    /// @param filename name of the file (in the working directory)
    auto parseJson(std::string filename) -> void
    {
        string kbuf, objStr; bsonio::ParserJson parserJson;
        bson bso; char b;
        bso.data = 0;
        // Reading work structure from json text file
        fstream f(filename, ios::in);
        flog.open(parsinglogfile, ios::trunc); flog.close();

        if (!f.good())
        {
            Exception exception;
            exception.error << "File read error " << filename << " ";
            exception.reason << "The file could not be read. ";
            exception.line = __LINE__;
            RaiseError(exception);
        }

        try
        {

            while( !f.eof() )
            {
                f.get(b);
                if( b == bsonio::jsBeginObject )
                {
                    b= ' ';
                    objStr =  parserJson.readObjectText(f);
                    bson_init(&bso);
                    parserJson.parseObject( &bso );
                    bson_finish(&bso);

                    bsonio::bson_to_key( bso.data, label, kbuf );

                    if (kbuf == "substance")
                    {
                        Substance substance = parseSubstance(bso.data);
                        substances_map[substance.symbol()] = substance;
                    } else
                    if (kbuf == "reaction")
                    {
                          Reaction reaction = parseReaction(bso.data);
                          reactions_map[reaction.symbol()] = reaction;
                    } else
                    {
                        Exception exception;
                        exception.error << "Unknown JSON type " << kbuf << " ";
                        exception.reason << "The JSON object needs to be a substance or reaction, file " << filename << ".";
                        exception.line = __LINE__;
                        RaiseError(exception);
                    }
                }
            }
        }
        catch (bsonio::bsonio_exeption e)
        {
            Exception exception;
            exception.error << e.title_;
            exception.reason << e.mess_;
            exception.line = __LINE__;
            RaiseError(exception);
        }
    }
};

Database::Database()
: pimpl(new Impl())
{}

Database::Database(std::string filename)
: pimpl(new Impl(filename))
{}

Database::Database(vector<bson> bsonSubstances)
: pimpl(new Impl(bsonSubstances))
{}

auto Database::setAqSubstanceSolventSymbol(std::string substance_symbol, std::string solvent_symbol) -> void
{
    pimpl->substances_map.at(substance_symbol).setSolventSymbol(solvent_symbol);
}

auto Database::setAllAqSubstanceSolventSymbol(std::string solvent_symbol) -> void
{
    typedef SubstancesMap::iterator it_type;
    for(it_type it = pimpl->substances_map.begin(); it != pimpl->substances_map.end(); it++) {
        if (it->second.substanceClass() == SubstanceClass::type::AQSOLUTE)
        {
            it->second.setSolventSymbol(solvent_symbol);
        }
    }
}

auto Database::addSubstance(const Substance& substance) -> void
{
    pimpl->addSubstance(substance);
}

auto Database::addMapSubstances(const SubstancesMap& substances) -> void
{
    pimpl->addMapSubstances(substances);
}

auto Database::addReaction(const Reaction& reaction) -> void
{
    pimpl->addReaction(reaction);
}

auto Database::addMapReactions(const ReactionsMap& reactions) -> void
{
    pimpl->addMapReactions(reactions);
}

auto Database::getSubstance(std::string symbol) const -> const Substance&
{
    return pimpl->getSubstance(symbol);
}

auto Database::getReaction(std::string symbol) const -> const Reaction&
{
    return pimpl->getReaction(symbol);
}

auto Database::getSubstances() -> std::vector<Substance>
{
    return pimpl->getSubstances();
}

auto Database::getReactions() -> std::vector<Reaction>
{
    return pimpl->getReactions();
}

auto Database::numberOfSubstances() -> int
{
    return pimpl->numberOfSubstances();
}

auto Database::numberOfReactions() -> int
{
    return pimpl->numberOfReactions();
}
auto Database::containsSubstance(std::string symbol) const -> bool
{
    return pimpl->containsSubstance(symbol);
}

auto Database::containsReaction(std::string symbol) const -> bool
{
    return pimpl->containsReaction(symbol);
}

} // namespace ThermoFun

