#include "Database.h"

// C++ includes
#include <map>
#include <set>
#include <string>
#include <vector>

// TCorrPT includes
#include "Common/Exception.h"
#include "ReadFiles.h"

//#include "bsonio/v_json.h"
#include "bsonio/nejdb.h"

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

/*
        bson InputBson;
        bson bso;
        bso.data = 0;

        file.LoadBson( &InputBson );
        string key = InputBson.data;


        string kbuf;

        vector<string> values;
        string field = "properties.eos_hkf_coeffs.values.0";

        bsonio::bson_to_key( InputBson.data, field.c_str(), kbuf );

        bsonio::strip( kbuf );

        std::string className = "VertexSubstance";

        SchemaNode* data;

        ThriftSchema schema;

        data->_schema = &schema;

        data->_schema->addSchemaFile("substance.schema.json");
        data->_schema->addSchemaFile("graphdb.schema.json");
        data->_schema->addSchemaFile("prop.schema.json");

        SchemaNode* data2 = data->newSchemaStruct( className, bso.data );

        char b;

        // Reading work structure from json text file
        fstream f(filename, ios::in);
        bsonio::bsonioErrIf( !f.good() , filename, "Fileread error...");

        bsonio::ParserJson parserJson;
        string objStr;
        string value;

//        impex::FormatStructDataFile fformatdata;

//        readDataFromBsonSchema(data->_schema, &InputBson, "VertexSubstance",  &fformatdata );

        while( !f.eof() )
           {
              f.get(b);
              if( b == bsonio::jsBeginObject )
              {
                b= ' ';
                objStr =  parserJson.readObjectText(f);
                //std::cout << objStr.c_str() << endl;
                bson_init(&bso);
                parserJson.parseObject( &bso );
                bson_finish(&bso);

                bsonio::bson_to_key( bso.data, field.c_str(), kbuf );
                data2->setStruct(bso.data);

                data2->field("properties.eos_hkf_coeffs.values.0")->getValue( value  );

                int size = data2->field("properties.eos_hkf_coeffs.values")->getSizeArray();
                vector<string> vvalue;

                data2->field("properties.eos_hkf_coeffs.values")->getArray( vvalue  );

//                cout << value << endl;

               }
            }

*/
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

    auto numberOfSubstances() -> int
    {
        return collectValues(substances_map).size();
    }

    auto numberOfReactions() -> int
    {
        return collectValues(reactions_map).size();
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

    /// Parses the JSON file and puts the data into the internal data structure
    /// @param filename name of the file (in the working directory)
    auto parseJson(std::string filename) -> void
    {
        string kbuf, objStr; bsonio::ParserJson parserJson;
        bson bso; char b;
        bso.data = 0;
        // Reading work structure from json text file
        fstream f(filename, ios::in);

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
                        Substance substance = parseSubstance(bso);
                        substances_map[substance.symbol()] = substance;
                    } else
                    if (kbuf == "reaction")
                    {
    //                      Reaction reaction = parseReaction(bso);
    //                      reactions_map[reaction.name()] = reaction;
                    } else
                    {
                        Exception exception;
                        exception.error << "Unknown JSON type " << kbuf << " ";
                        exception.reason << "The JSON object needs to be a substance, file " << filename << ".";
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

auto Database::addSubstance(const Substance& substance) -> void
{
    Substance subst;
    pimpl->addSubstance(substance);
}

auto Database::addReaction(const Reaction& reaction) -> void
{
    pimpl->addReaction(reaction);
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

} // namespace TCorrPT

