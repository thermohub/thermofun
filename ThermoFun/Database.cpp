#include "Database.h"

// ThermoFun includes
#include "ChemicalFun/FormulaParser/ChemicalData.h"
#include "Common/Exception.h"
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

    /// Downloaded elements data for formula parser
    ChemicalFun::DBElements all_elements;
    //char type_ = jsonio::FileTypes::Undef_;

    Impl()
    {}

    Impl(std::string filename)
    {
        //jsonio::FJson file (filename);
        //type_ = file.Type();
        fromFile(filename);
        setDBElements(elements_map);
    }

    Impl(std::vector<std::string> jsons, std::string _label)
    {
        fromJSONs(jsons, _label);
        setDBElements(elements_map);
    }

    template<typename Key, typename Value>
    auto collectValues(const std::map<Key, Value>& map) const -> std::vector<Value>
    {
        std::vector<Value> collection;
        collection.reserve(map.size());
        for(const auto& pair : map)
            collection.push_back(pair.second);
        return collection;
    }

    auto setDBElement(Element& element) -> void
    {
        ChemicalFun::ElementValues eldata;
        auto elkey = element.toElementKey(eldata);
        all_elements.addElement(elkey, eldata);
    }

    auto setDBElements(ElementsMap elements) -> void
    {
        thfun_logger->debug("Database::setDBElements() elements {}", elements.size());
        for (auto& e : elements) {
            setDBElement(e.second);
        }
    }

    auto elementKeyToElement(ChemicalFun::ElementKey elementKey) -> Element
    {
        auto itrdb = all_elements.elements().find(elementKey);
        if (itrdb == all_elements.elements().end())
            funError("Invalid symbol", elementKey.Symbol(), __LINE__, __FILE__);
        return Element(elementKey, itrdb->second);
    }

    auto addElement(const Element& element) -> void
    {
        elements_map.insert({element.symbol(), element});
        setDBElement(elements_map[element.symbol()]);
    }

    auto setElement(const Element& element) -> void
    {
        checkIfSymbolExists(elements_map, "element", element.symbol());
        elements_map[element.symbol()] = element;
        setDBElement(elements_map[element.symbol()]);
    }

    auto addSubstance(const Substance& substance) -> void
    {
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
        setDBElements(elements_map);
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
        checkIfSymbolExists(reactions_map, "reaction", reaction.symbol());
        reactions_map[reaction.symbol()] = reaction;
    }

    auto addMapReactions(const ReactionsMap& reactions) -> void
    {
        reactions_map = reactions;
    }

    auto getElements() const -> std::vector<Element>
    {
        return collectValues(elements_map);
    }

    auto getSubstances() const -> std::vector<Substance>
    {
        return collectValues(substances_map);
    }

    auto getReactions() const -> std::vector<Reaction>
    {
        return collectValues(reactions_map);
    }

    auto numberOfElements() const -> size_t
    {
        return elements_map.size();
    }

    auto numberOfSubstances() const -> size_t
    {
        return substances_map.size();
    }

    auto numberOfReactions() const -> size_t
    {
        return reactions_map.size();
    }

    auto getElement(std::string symbol) -> const Element&
    {
        if(!containsElement(symbol)) {
            errorNonExistent("element", symbol, __LINE__);
        }
        return elements_map[symbol];
    }

    auto getSubstance(std::string symbol) -> const Substance&
    {
        if(!containsSubstance(symbol)) {
            errorNonExistent("substance", symbol, __LINE__);
        }
        return substances_map[symbol];
    }

    auto getReaction(std::string symbol) -> const Reaction&
    {
        if(!containsReaction(symbol)) {
            errorNonExistent("reaction", symbol, __LINE__);
        }
        return reactions_map[symbol];
    }

    auto element(std::string symbol) -> Element&
    {
        if(!containsElement(symbol)) { // try restore data from defaults
            auto el_key =ChemicalFun::ElementKey(symbol,0);
            Element empty_element;
            if( all_elements.elements().find(el_key) != all_elements.elements().end()) {
                empty_element = elementKeyToElement(el_key);
            }
            else {
                empty_element.setSymbol(symbol);
                empty_element.setName(symbol);
            }
            elements_map[symbol] = empty_element;
        }
        return elements_map[symbol];
    }

    auto substance(std::string symbol) -> Substance&
    {
        if(!containsSubstance(symbol)) {
            Substance empty_subst;
            empty_subst.setSymbol(symbol);
            empty_subst.setName(symbol);
            // ... set other default data
            substances_map[symbol]=empty_subst;
        }
        return substances_map[symbol];
    }

    auto reaction(std::string symbol) -> Reaction&
    {
        if(!containsReaction(symbol))
        {
            Reaction empty_react;
            empty_react.setSymbol(symbol);
            empty_react.setName(symbol);
            // ... set other default data
            reactions_map[symbol]=empty_react;
        }
        return reactions_map[symbol];
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
        if (it != map_.end()) {
            thfun_logger->warn("The {} with symbol {} is already in the database. Overwriting ... \n"
                               "To add it to the database as a separate record assign it a different symbol.", record_type, symbol);
       }
    }

    auto addRecord(json j, std::string _label) -> void
    {
        auto props= j;

        if (j.contains("properties"))
            if (!j["properties"].is_null())
                props = j["properties"];
        if (j.contains("_label"))
            if (!j["_label"].is_null())
                _label = j["_label"].get<std::string>();

        props = props.dump();

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

    auto addRecords(json j, std::string _label = "unknown label") -> void
    {
        for(auto it = j.begin(); it != j.end(); ++it)
        {
            addRecord(it.value(), _label);
        }
    }

    /// Parses the JSON file (or string) and puts the data into the internal data structure
    /// @param filename name of the file (in the working directory)
    auto fromFile(std::string filename) -> void
    {
        try {

            json j;

            try
            {
                // trying to see if the string is a valid json
                j = json::parse(filename);
            }
            catch (json::parse_error &e)
            {
                std::ifstream ifs(filename);
                if (!ifs.good())
                    funError("File reading error", std::string("Database file " + filename + " not found!"), __LINE__, __FILE__);
                j = json::parse(ifs);
            }

            if (j.contains("elements"))
                addRecords(j["elements"], "element");
            if (j.contains("substances"))
                addRecords(j["substances"], "substance");
            if (j.contains("reactions"))
                addRecords(j["reactions"], "reaction");

            // if the file just contains an array of records containing key "_label" on on the type of record
            if (j.is_array())
                addRecords(j);
        }     catch (json::exception &ex)
        {
            thfun_logger->error(" exception id:  {} message: {}", ex.id, ex.what());
        }

    }

    auto fromJSONs(std::vector<std::string> jsons, std::string _label) -> void
    {
        try {

            if (jsons.size()>0) // bugfix for unknown json parse crash, DM 06.12.2019
                json j = json::parse(jsons[0]);
            for (size_t i=0; i<jsons.size(); i++)
            {
                json j = json::parse(jsons[i]);
                addRecord(j, _label);
            }
        }      catch (json::exception &ex)
        {
            thfun_logger->error(" exception id:  {} message: {}", ex.id, ex.what());
        }
    }
};

Database::Database()
: pimpl(new Impl())
{}

Database::Database(std::string filename)
: pimpl(new Impl(filename))
{}

Database::Database(std::vector<std::string> jsonRecords, std::string _label="unknown label")
: pimpl(new Impl(jsonRecords, _label))
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
    auto elements_number = pimpl->mapElements().size();
    pimpl->fromFile(filename);
    if (elements_number != pimpl->mapElements().size())
        pimpl->setDBElements(pimpl->mapElements());
}

auto Database::appendData(std::vector<std::string> jsonRecords, std::string _label = "unknown label") -> void
{
    auto elements_number = pimpl->mapElements().size();
    pimpl->fromJSONs(jsonRecords, _label);
    if (elements_number != pimpl->mapElements().size())
        pimpl->setDBElements(pimpl->mapElements());
}

auto Database::addElement(const Element& element) -> void
{
    pimpl->addElement(element);
}

auto Database::setElement(const Element& element) -> void
{
    pimpl->setElement(element);
}

auto Database::addElement(const std::string& jsonElement) -> void
{
    pimpl->addElement(Element(jsonElement));
}

auto Database::setElement(const std::string& jsonElement) -> void
{
    pimpl->setElement(Element(jsonElement));
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

auto Database::addSubstance(const std::string &jsonSubstance) -> void
{
    pimpl->addSubstance(Substance(jsonSubstance));
}

auto Database::setSubstance(const std::string &jsonSubstance) -> void
{
    pimpl->setSubstance(Substance(jsonSubstance));
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

auto Database::addReaction(const std::string &jsonReaction) -> void
{
    pimpl->addReaction(Reaction(jsonReaction));
}

auto Database::setReaction(const std::string &jsonReaction) -> void
{
    pimpl->setReaction(Reaction(jsonReaction));
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

auto Database::element(std::string symbol) -> Element&
{
    return pimpl->element(symbol);
}

auto Database::substance(std::string symbol) -> Substance&
{
    return pimpl->substance(symbol);
}

auto Database::reaction(std::string symbol) -> Reaction&
{
    return pimpl->reaction(symbol);
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

auto Database::getElements() const -> std::vector<Element>
{
    return pimpl->getElements();
}

auto Database::getSubstances() const -> std::vector<Substance>
{
    return pimpl->getSubstances();
}

auto Database::getReactions() const -> std::vector<Reaction>
{
    return pimpl->getReactions();
}

// Return all elements in the database
auto Database::getElementsList() const -> std::vector<std::string>
{
   std::vector<std::string> list;
   for(const auto& item: pimpl->elements_map) {
      list.push_back(item.first);
   }
   return list;
}

// Return all substances in the database
auto Database::getSubstancesList() const -> std::vector<std::string>
{
   std::vector<std::string> list;
   for(const auto& item: pimpl->substances_map) {
      list.push_back(item.first);
   }
   return list;
}

// Return all reactions in the database
auto Database::getReactionsList() const -> std::vector<std::string>
{
   std::vector<std::string> list;
   for(const auto& item: pimpl->reactions_map) {
      list.push_back(item.first);
   }
   return list;
}



auto Database::numberOfElements() const -> size_t
{
    return pimpl->numberOfElements();
}

auto Database::numberOfSubstances() const -> size_t
{
    return pimpl->numberOfSubstances();
}

auto Database::numberOfReactions() const -> size_t
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

auto Database::parseSubstanceFormula(std::string formula_) const -> std::map<Element, double>
{
    std::map<Element, double> map;
    ChemicalFun::FormulaToken formula(formula_);
    // ??? Do we need props, do not save
    //auto props = formula.properties(pimpl->all_elements.elements());

    for (const auto& element : formula.getStoichCoefficients())
    {
        Element e = pimpl->elementKeyToElement(element.first);
        map[e] = element.second;
    }

    return map;
}

auto Database::elementalEntropyFormula(std::string formula) const -> double
{
    ChemicalFun::FormulaProperties prop = pimpl->all_elements.formulasProperties(formula);
    return prop.elemental_entropy;
}

auto Database::setSubstancesChargeFromFromula() -> void
{
    for (auto& s: pimpl->substances_map)
    {
//        auto js = s.second.jsonString();
//        if (js != "")
//        {
//            json j = json::parse(js);
//            if (!j.contains("formula_charge"))
//            {
//                 ChemicalFun::FormulaToken formula(s.second.formula());
//                 auto p = formula.properties(pimpl->all_elements.elements());
//                 s.second.setCharge(p.charge);
//            } else if (j["formula_charge"].is_null())
//            {
//                ChemicalFun::FormulaToken formula(s.second.formula());
//                auto p = formula.properties(pimpl->all_elements.elements());
//                s.second.setCharge(p.charge);
//            }
//        } else
        {
            ChemicalFun::FormulaToken formula(s.second.formula());
            auto p = formula.properties(pimpl->all_elements.elements());
            if (s.second.charge() != p.charge)
            {
                s.second.setCharge(p.charge);
                thfun_logger->warn(" for {} the record charge {} was updated to {}, calculated from the formula", s.second.symbol(), s.second.charge(), p.charge );
            }
        }
    }
}

} // namespace ThermoFun

