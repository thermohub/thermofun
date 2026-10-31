// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "DatabaseImpl.h"

namespace ThermoFun {

auto readConventions(const std::string &data) -> DatabaseConventions
{
    DatabaseConventions c;

    try {
    const auto& jc = json::parse(data);

    if (jc.contains("water_triple_point_reference") && !jc["water_triple_point_reference"].is_null())
        c.water_triple_point_reference = jc["water_triple_point_reference"].get<std::string>();
    }
    catch (json::exception &ex)
    {
        thfun_logger->error(" exception id:  {} message: {}", ex.id, ex.what());
    }

    return c;
}

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
    pimpl->changed();
    auto elements_number = pimpl->mapElements().size();
    pimpl->fromFile(filename);
    if (elements_number != pimpl->mapElements().size())
        pimpl->setDBElements(pimpl->mapElements());
}

auto Database::appendData(std::vector<std::string> jsonRecords, std::string _label = "unknown label") -> void
{
    pimpl->changed();
    auto elements_number = pimpl->mapElements().size();
    pimpl->fromJSONs(jsonRecords, _label);
    if (elements_number != pimpl->mapElements().size())
        pimpl->setDBElements(pimpl->mapElements());
}

auto Database::addElement(const Element& element) -> void
{
    pimpl->changed();
    pimpl->addElement(element);
}

auto Database::setElement(const Element& element) -> void
{
    pimpl->changed();
    pimpl->setElement(element);
}

auto Database::addElement(const std::string& jsonElement) -> void
{
    pimpl->changed();
    pimpl->addElement(Element(jsonElement));
}

auto Database::setElement(const std::string& jsonElement) -> void
{
    pimpl->changed();
    pimpl->setElement(Element(jsonElement));
}


auto Database::addMapElements(const ElementsMap& elements) -> void
{
    pimpl->changed();
    pimpl->addMapElements(elements);
}

auto Database::setMapElements(const ElementsMap& elements) -> void
{
    pimpl->changed();
    pimpl->setMapElements(elements);
}

auto Database::setMapSubstances(const SubstancesMap& substances) -> void
{
    pimpl->changed();
    pimpl->setMapSubstances(substances);
}

auto Database::setMapReactions(const ReactionsMap& reactions) -> void
{
    pimpl->changed();
    pimpl->setMapReactions(reactions);
}

auto Database::addSubstance(const Substance& substance) -> void
{
    pimpl->changed();
    pimpl->addSubstance(substance);
}

auto Database::setSubstance(const Substance& substance) -> void
{
    pimpl->changed();
    pimpl->setSubstance(substance);
}

auto Database::addSubstance(const std::string &jsonSubstance) -> void
{
    pimpl->changed();
    pimpl->addSubstance(Substance(jsonSubstance));
}

auto Database::setSubstance(const std::string &jsonSubstance) -> void
{
    pimpl->changed();
    pimpl->setSubstance(Substance(jsonSubstance));
}

auto Database::addMapSubstances(const SubstancesMap& substances) -> void
{
    pimpl->changed();
    pimpl->addMapSubstances(substances);
}

auto Database::addReaction(const Reaction& reaction) -> void
{
    pimpl->changed();
    pimpl->addReaction(reaction);
}

auto Database::setReaction(const Reaction& reaction) -> void
{
    pimpl->changed();
    pimpl->setReaction(reaction);
}

auto Database::addReaction(const std::string &jsonReaction) -> void
{
    pimpl->changed();
    pimpl->addReaction(Reaction(jsonReaction));
}

auto Database::setReaction(const std::string &jsonReaction) -> void
{
    pimpl->changed();
    pimpl->setReaction(Reaction(jsonReaction));
}

auto Database::addMapReactions(const ReactionsMap& reactions) -> void
{
    pimpl->changed();
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
    pimpl->changed();
    return pimpl->element(symbol);
}

auto Database::substance(std::string symbol) -> Substance&
{
    pimpl->changed();
    return pimpl->substance(symbol);
}

auto Database::reaction(std::string symbol) -> Reaction&
{
    pimpl->changed();
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

auto Database::thermoDataSet() const -> const v2::ThermoDataSetInfo&
{
    return pimpl->dataset_info;
}

auto Database::methods() const -> const std::map<std::string, v2::MethodDefinition>&
{
    return pimpl->methods;
}

auto Database::parameterSets() const -> const std::map<std::string, v2::MethodParameterSet>&
{
    return pimpl->parameter_sets;
}

auto Database::dataSources() const -> const std::map<std::string, v2::DataSource>&
{
    return pimpl->data_sources;
}

auto Database::loadDiagnostics() const -> const Diagnostics&
{
    return pimpl->load_diagnostics;
}

auto Database::revision() const -> std::uint64_t
{
    return pimpl->revision;
}

DatabaseConventions& Database::conventions() { 
    return pimpl->conventions; }

const DatabaseConventions& Database::conventions() const { 
    return pimpl->conventions; }

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

auto Database::elementEntropySources(const std::string& formula) const -> ElementEntropySources
{
    ElementEntropySources sources;
    const auto& table = pimpl->all_elements.elements();
    const ChemicalFun::FormulaToken token(formula);
    for (const auto& key : token.getElementsList())
    {
        const auto it = table.find(key);
        if (it == table.end() || it->second.entropy == 0.0)
            sources.missing.push_back(key.Symbol());
        else if (!pimpl->elements_map.count(key.Symbol()))
            sources.from_table.push_back(key.Symbol());
    }
    return sources;
}

auto Database::setSubstancesChargeFromFromula() -> void
{
    pimpl->changed();
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

