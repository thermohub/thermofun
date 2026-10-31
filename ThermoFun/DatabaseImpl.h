// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The inside of a Database: what Database.cpp and the Database*.cpp files share. Not installed.

#ifndef THERMOFUN_DATABASEIMPL_H
#define THERMOFUN_DATABASEIMPL_H

#include "Database.h"
#include "ChemicalFun/FormulaParser/ChemicalData.h"
#include <iterator>
#include "Common/Exception.h"
#include "IO/V2/V2Reader.h"
#include "IO/V2/V2Writer.h"
#include "IO/Legacy/LegacyReader.h"
#include "IO/Legacy/LegacyWriter.h"
#include "IO/Validate.h"
#include "Engine/Binding.h"
#include "Conventions/Convention.h"
#include "Conventions/HydrogenIon.h"
#include "Methods/MethodCatalog.h"
#include "IO/detail/JsonDetect.h"
#include "IO/detail/Yaml.h"
#include "Substance.h"
#include "Reaction.h"
#include "Element.h"
#include <nlohmann/json.hpp>
#include <atomic>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <set>

using json = nlohmann::json;

namespace ThermoFun {

[[ noreturn ]] inline auto errorNonExistent(std::string type, std::string name, int line) -> void
{
    Exception exception;
    exception.error << "Cannot get an instance of the " << type << " `" << name << "` in the database.";
    exception.reason << "There is no such " << type << " in the database.";
    exception.line = line;
    RaiseError(exception)
}

struct Database::Impl
{
    /// Data about the dataset, and the records of the project that a v2 file gives
    v2::ThermoDataSetInfo dataset_info;
    std::map<std::string, v2::MethodDefinition> methods;
    std::map<std::string, v2::MethodParameterSet> parameter_sets;
    std::map<std::string, v2::DataSource> data_sources;
    Diagnostics load_diagnostics;

    /// The files that a load has read, so that an included file is read once
    std::set<std::string> loaded_files;

    /// The map of all aqueous species in the database
    SubstancesMap substances_map;

    /// The map of all reactions in the database
    ReactionsMap reactions_map;

    /// The map of all elements in the database
    ElementsMap elements_map;

    /// Downloaded elements data for formula parser
    ChemicalFun::DBElements all_elements;
    //char type_ = jsonio::FileTypes::Undef_;

    /// For calculating and outputting properties of a database e.g., the water triple properties
    mutable DatabaseConventions conventions;

    /// Changes with every change of the records (unique over all databases, see Database::revision)
    std::uint64_t revision = nextRevision();

    static auto nextRevision() -> std::uint64_t
    {
        static std::atomic<std::uint64_t> counter{0};
        return ++counter;
    }

    auto changed() -> void { revision = nextRevision(); }

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

    auto setMapElements(const ElementsMap& elements) -> void
    {
        for (const auto& element : elements)
        {
            elements_map[element.first] = element.second;
            setDBElement(elements_map[element.first]);
        }
    }

    auto setMapSubstances(const SubstancesMap& substances) -> void
    {
        for (const auto& substance : substances)
            substances_map[substance.first] = substance.second;
    }

    auto setMapReactions(const ReactionsMap& reactions) -> void
    {
        for (const auto& reaction : reactions)
            reactions_map[reaction.first] = reaction.second;
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

    auto addRecord(json j, std::string _label) -> void;

    auto addRecords(json j, std::string _label = "unknown label") -> void;

    /// Puts the records of a v2 document into the database. Warnings are logged; an error stops the reading.
    auto fromV2(const std::string& text, const std::string& file) -> void;

    /// A v2 text: a file can include others, and a text has no folder to look in
    auto fromV2File(const std::string& text, const std::string& file) -> void;

    // ---- files that include other files, and dataset folders ----

    static auto readWhole(const std::filesystem::path& p) -> std::string;

    // A pattern with * and ?
    static auto matches(const std::string& pattern, const std::string& name, size_t p = 0, size_t n = 0) -> bool;

    // The files of an include: the files of the folder that match, in sorted order
    static auto expand(const std::filesystem::path& pattern) -> std::vector<std::filesystem::path>;

    /// Reads a file of the v2 format: the files that it includes first, each one once, then the file. A file with `record_type`
    /// holds one record.
    auto loadV2File(const std::filesystem::path& path, std::vector<std::string>& stack) -> void;

    /// Reads a dataset folder: the records of the project in the nearest folder above it that has them, then thermodataset, then the
    /// files of elements/, substances/ and reactions/, in sorted order.
    auto loadDirectory(const std::filesystem::path& folder) -> void;

    /// Parses the JSON file (or string) and puts the data into the internal data structure
    /// @param filename name of the file (in the working directory)
    auto fromFile(std::string filename) -> void;

    auto fromJSONs(std::vector<std::string> jsons, std::string _label) -> void;
};

} // namespace ThermoFun

#endif
