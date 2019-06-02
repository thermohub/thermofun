#include "Database.h"

// ThermoFun includes
#include "Common/Exception.h"
//#include "Common/ParseBsonTraversalData.h"
#include "DBClient/formuladata.h"
#include "Substance.h"
#include "Reaction.h"
#include "Element.h"
#include "DBClient/DatabaseClient.h"
//#include "DBClient/ThermoSetData.h"

// jsonio includes
#include "jsonio/json2file.h"

#include "nlohmann/json.hpp"
#include "Common/ParseJsonToData.h"


using json = nlohmann::json;

namespace ThermoFun {

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
    /// The map of all aqueous species in the database
    SubstancesMap substances_map;

    /// The map of all reactions in the database
    ReactionsMap reactions_map;

    /// The map of all elements in the database
    ElementsMap elements_map;

    //char type_ = jsonio::FileTypes::Undef_;

    Impl()
    {}

    Impl(std::string filename)
    {
        //jsonio::FJson file (filename);
        //type_ = file.Type();
        parseJson( filename );
        if (elements_map.size()>0)
            ChemicalFormula::setDBElements( elements_map );
    }

    Impl (DatabaseClient &dbc, const string &ThermoDataSetSymbol)
    {
        auto recordList = dbc.recordsFromThermoDataSet(ThermoDataSetSymbol);
//        auto db = databaseFromRecordList(dbc, recordList);
//        substances_map = db.mapSubstances();
//        reactions_map  = db.mapReactions();
    }

    Impl(vector<string> jsons)
    {
        string kbuf;
//        flog.open(parsinglogfile, ios::trunc); flog.close();

        for (int i=0; i<jsons.size(); i++)
        {
            auto domdata = jsonio::unpackJson( jsons[i] );
            domdata->findValue(label, kbuf);
            //bsonio::bson_to_key( bsons[i].data, label, kbuf );

            if (kbuf == "substance")
            {
//                Substance substance = parseSubstance(domdata.get());
//                substances_map[substance.symbol()] = substance;
            } else
            if (kbuf == "reaction")
            {
                    //                      Reaction reaction = parseReaction(bso);
                    //                      reactions_map[reaction.symbol()] = reaction;
            } else
            if (kbuf == "element")
            {
//                Element element = parseElement(domdata.get());
//                elements_map[element.symbol()] = element;
            } else
            {
                Exception exception;
                exception.error << "Unknown JSON type " << kbuf << " ";
                exception.reason << "The JSON object needs to be a substance or reaction.";
                exception.line = __LINE__;
                RaiseError(exception);
            }
            if (elements_map.size()>0)
                ChemicalFormula::setDBElements( elements_map );
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

    auto addElement(const Element& element) -> void
    {
        elements_map.insert({element.symbol(), element});
    }

    auto setElement(const Element& element) -> void
    {
        elements_map[element.symbol()] = element;
    }

    auto addSubstance(const Substance& substance) -> void
    {
        substances_map.insert({substance.symbol(), substance});
    }

    auto setSubstance(const Substance& substance) -> void
    {
        substances_map[substance.symbol()] = substance;
    }

    auto addMapElements(const ElementsMap& elements) -> void
    {
        elements_map = elements;
    }

    auto addMapSubstances(const SubstancesMap& substances) -> void
    {
        substances_map = substances;
    }

    auto addReaction(const Reaction& reaction) -> void
    {
        reactions_map.insert({reaction.symbol(), reaction});
    }

    auto setReaction(const Reaction& reaction) -> void
    {
        reactions_map[reaction.symbol()] = reaction;
    }

    auto addMapReactions(const ReactionsMap& reactions) -> void
    {
        reactions_map = reactions;
    }

    auto getElements() -> std::vector<Element>
    {
        return collectValues(elements_map);
    }

    auto getSubstances() -> std::vector<Substance>
    {
        return collectValues(substances_map);
    }

    auto getReactions() -> std::vector<Reaction>
    {
        return collectValues(reactions_map);
    }

//    auto calcParametersReactions( ) -> void
//    {
//        for (auto &reaction : reactions_map)
//            reaction.second.calcParameters();
//    }

    auto numberOfElements() -> int
    {
        return collectValues(elements_map).size();
    }

    auto numberOfSubstances() -> int
    {
        return collectValues(substances_map).size();
    }

    auto numberOfReactions() -> int
    {
        return collectValues(reactions_map).size();
    }

    auto getElement(std::string symbol) -> Element&
    {
        if(elements_map.count(symbol) == 0)
            errorNonExistent("element", symbol, __LINE__);

        return elements_map.find(symbol)->second;
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

    auto mapElements() -> ElementsMap&
    {
        return elements_map;
    }

    auto mapSubstances() -> SubstancesMap&
    {
        return substances_map;
    }

    auto mapReactions() -> ReactionsMap&
    {
        return reactions_map;
    }

    auto containsElement(std::string symbol) const -> bool
    {
        return elements_map.count(symbol) != 0;
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
        string kbuf;
        flog.open(parsinglogfile, ios::trunc);
        flog.close();

        try
        {
            std::ifstream ifs(filename);
            json j = json::parse(ifs);

            for(auto it = j.begin(); it != j.end(); ++it)
            {
                auto properties = it.value()["properties"];
                auto _label = it.value()["_label"].get<std::string>();

                if (_label == "substance")
                {
                    Substance substance = parseSubstance(properties.dump());
                    substances_map[substance.symbol()] = substance;
                } else
                    if (_label == "reaction")
                    {
                        Reaction reaction = parseReaction(properties.dump());
                        reactions_map[reaction.symbol()] = reaction;
                    } else
                        if (_label == "element")
                        {
                            Element element = parseElement(properties.dump());
                            elements_map[element.symbol()] = element;
                        }
                        else
                        {
                            Exception exception;
                            exception.error << "Unknown JSON type " << _label << " ";
                            exception.reason << "The JSON object needs to be a substance or reaction, file " << filename << ".";
                            exception.line = __LINE__;
                            RaiseError(exception);
                        }

            }
        }
        catch (jsonio::jsonio_exception e)
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

Database::Database(vector<string> jsonSubstances)
: pimpl(new Impl(jsonSubstances))
{}

Database::Database(DatabaseClient &dbc, const std::string &thermoDataSetSymbol)
: pimpl(new Impl(dbc, thermoDataSetSymbol))
{}

Database::Database(const Database& other)
: pimpl(new Impl(*other.pimpl))
{}

auto Database::operator=(Database other) -> Database&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

auto Database::addElement(const Element& element) -> void
{
    pimpl->addElement(element);
}

auto Database::setElement(const Element& element) -> void
{
    pimpl->setElement(element);
}

auto Database::addMapElements(const ElementsMap& elements) -> void
{
    pimpl->addMapElements(elements);
}

auto Database::addSubstance(const Substance& substance) -> void
{
    pimpl->addSubstance(substance);
}

auto Database::setSubstance(const Substance& substance) -> void
{
    pimpl->setSubstance(substance);
}

auto Database::addMapSubstances(const SubstancesMap& substances) -> void
{
    pimpl->addMapSubstances(substances);
}

auto Database::addReaction(const Reaction& reaction) -> void
{
    pimpl->addReaction(reaction);
}

auto Database::setReaction(const Reaction& reaction) -> void
{
    pimpl->setReaction(reaction);
}

auto Database::addMapReactions(const ReactionsMap& reactions) -> void
{
    pimpl->addMapReactions(reactions);
}

auto Database::getElement(std::string symbol) const -> const Element&
{
    return pimpl->getElement(symbol);
}

auto Database::getSubstance(std::string symbol) const -> const Substance&
{
    return pimpl->getSubstance(symbol);
}

auto Database::getReaction(std::string symbol) const -> const Reaction&
{
    return pimpl->getReaction(symbol);
}

//auto Database::calcParametersReactions( ) -> void
//{
//    return pimpl->calcParametersReactions( );
//}

auto Database::mapElements() const -> const ElementsMap&
{
    return pimpl->mapElements();
}

auto Database::mapSubstances() const -> const SubstancesMap&
{
    return pimpl->mapSubstances();
}

auto Database::mapReactions() const -> const ReactionsMap&
{
    return pimpl->mapReactions();
}

auto Database::getElements() -> std::vector<Element>
{
    return pimpl->getElements();
}

auto Database::getSubstances() -> std::vector<Substance>
{
    return pimpl->getSubstances();
}

auto Database::getReactions() -> std::vector<Reaction>
{
    return pimpl->getReactions();
}

auto Database::numberOfElements() -> int
{
    return pimpl->numberOfElements();
}

auto Database::numberOfSubstances() -> int
{
    return pimpl->numberOfSubstances();
}

auto Database::numberOfReactions() -> int
{
    return pimpl->numberOfReactions();
}

auto Database::containsElement(std::string symbol) const -> bool
{
    return pimpl->containsElement(symbol);
}

auto Database::containsSubstance(std::string symbol) const -> bool
{
    return pimpl->containsSubstance(symbol);
}

auto Database::containsReaction(std::string symbol) const -> bool
{
    return pimpl->containsReaction(symbol);
}

auto Database::parseSubstanceFormula(std::string formula_) -> std::map<Element, double>
{
    std::set<ElementKey> elements;
    std::map<Element, double> map;
    FormulaToken formula("");

    formula.setFormula(formula_);
//    elements.insert(formula.getElements().begin(), formula.getElements().end());

    for (auto element : formula.getElements_map())
    {
        Element e = elementKeyToElement(element.first);
        map[e] = element.second;
    }

    return map;
}


} // namespace ThermoFun

