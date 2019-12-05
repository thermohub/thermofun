#include "Database.h"

// ThermoFun includes
#include "Common/Exception.h"
#include "Common/formuladata.h"
#include "Common/ParseJsonToData.h"
#include "Substance.h"
#include "Reaction.h"
#include "Element.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ThermoFun {

[[ noreturn ]] auto errorNonExistent(std::string type, std::string name, int line) -> void
{
    Exception exception;
    exception.error << "Cannot get an instance of the " << type << " `" << name << "` in the database.";
    exception.reason << "There is no such " << type << " in the database.";
    exception.line = line;
    RaiseError(exception)
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
        fromFile( filename );
        if (elements_map.size()>0)
            ChemicalFormula::setDBElements( elements_map );
    }

    Impl(vector<string> jsons)
    {
        fromJSONs(jsons);
        if (elements_map.size()>0)
            ChemicalFormula::setDBElements( elements_map );
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
        checkIfSymbolExists(elements_map, "element", element.symbol());
        elements_map.insert({element.symbol(), element});
    }

    auto setElement(const Element& element) -> void
    {
        checkIfSymbolExists(elements_map, "element", element.symbol());
        elements_map[element.symbol()] = element;
    }

    auto addSubstance(const Substance& substance) -> void
    {
        checkIfSymbolExists(substances_map, "substance", substance.symbol());
        substances_map.insert({substance.symbol(), substance});
    }

    auto setSubstance(const Substance& substance) -> void
    {
        checkIfSymbolExists(substances_map, "substance", substance.symbol());
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
        checkIfSymbolExists(reactions_map, "reaction", reaction.symbol());
        reactions_map.insert({reaction.symbol(), reaction});
    }

    auto setReaction(const Reaction& reaction) -> void
    {
        checkIfSymbolExists(reactions_map, "reaction", reaction.symbol());
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

    auto numberOfElements() -> size_t
    {
        return collectValues(elements_map).size();
    }

    auto numberOfSubstances() -> size_t
    {
        return collectValues(substances_map).size();
    }

    auto numberOfReactions() -> size_t
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

    template<class T>
    auto checkIfSymbolExists(std::map<std::string, T> map_, std::string record_type, std::string symbol) -> void
    {
        auto it = map_.find(symbol);
        if (it != map_.end())
            cout << "The "<< record_type <<" with the symbol " << symbol
                 << " is already in the database. Overwriting ..." << endl
                 << "To add it to the database assing it a different symbol." << endl;
    }

    auto addRecord(json j) -> void
    {
        auto properties = j;
        std::string _label;

        if (j.contains("properties"))
            if (!j["properties"].is_null())
                properties = j["properties"];
        if (j.contains("_label"))
            if (!j["_label"].is_null())
                _label = j["_label"].get<std::string>();

        auto props = properties.dump();

        if (_label == "substance")
        {
            Substance substance = parseSubstance(props);
            substance.setJsonString(props);
            checkIfSymbolExists(substances_map, "substance", substance.symbol());
            substances_map[substance.symbol()] = substance;
        } else
        if (_label == "reaction")
        {
           Reaction reaction = parseReaction(props);
           reaction.setJsonString(props);
           checkIfSymbolExists(reactions_map, "reaction", reaction.symbol());
           reactions_map[reaction.symbol()] = reaction;
        } else
        if (_label == "element")
        {
            Element element = parseElement(props);
            element.setJsonString(props);
            checkIfSymbolExists(elements_map, "element", element.symbol());
            elements_map[element.symbol()] = element;
        } else
        {
            Exception exception;
            exception.error << "Unknown JSON type " << _label << " ";
            exception.reason << "The JSON object needs to be an element, a substance or a reaction. ";
            exception.line = __LINE__;
            RaiseError(exception)
        }
    }

    /// Parses the JSON file and puts the data into the internal data structure
    /// @param filename name of the file (in the working directory)
    auto fromFile(std::string filename) -> void
    {
        try {
            std::ifstream ifs(filename);
            if (!ifs.good())
                funError("File reading error", std::string("Database file "+ filename +" not found!"), __LINE__, __FILE__);
            json j = json::parse(ifs);

            for(auto it = j.begin(); it != j.end(); ++it)
            {
                addRecord(it.value());
            }
        }     catch (json::exception &ex)
        {
            // output exception information
            std::cout << "message: " << ex.what() << '\n'
                      << "exception id: " << ex.id << std::endl;
        }

    }

    auto fromJSONs(vector<string> jsons) -> void
    {
        try {
//            flog.open(parsinglogfile, ios::trunc); flog.close();
            for (size_t i=0; i<jsons.size(); i++)
            {
                json j = json::parse(jsons[i]);
                addRecord(j);
            }
        }      catch (json::exception &ex)
        {
            // output exception information
            std::cout << "message: " << ex.what() << '\n'
                      << "exception id: " << ex.id << std::endl;
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

Database::Database(const Database& other)
: pimpl(new Impl(*other.pimpl))
{}

auto Database::operator=(Database other) -> Database&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

auto Database::appendData(std::string filename) -> void
{
    pimpl->fromFile(filename);
}

auto Database::appendData(vector<string> jsonRecords) -> void
{
    pimpl->fromJSONs(jsonRecords);
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

auto Database::numberOfElements() -> size_t
{
    return pimpl->numberOfElements();
}

auto Database::numberOfSubstances() -> size_t
{
    return pimpl->numberOfSubstances();
}

auto Database::numberOfReactions() -> size_t
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

    for (auto element : formula.getElements_map())
    {
        Element e = elementKeyToElement(element.first);
        map[e] = element.second;
    }

    return map;
}


} // namespace ThermoFun

