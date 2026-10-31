// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "Database.h"

// ThermoFun includes
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
#include <set>

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
            Substance substance(props.get<std::string>());
            checkIfSymbolExists(substances_map, "substance", substance.symbol());
            substances_map[substance.symbol()] = substance;
        } else
        if (_label == "reaction")
        {
           Reaction reaction(props.get<std::string>());
           checkIfSymbolExists(reactions_map, "reaction", reaction.symbol());
           reactions_map[reaction.symbol()] = reaction;
        } else
        if (_label == "element")
        {
            Element element(props.get<std::string>());
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

    /// Puts the records of a v2 document into the database. Warnings are logged; an error stops the reading.
    auto fromV2(const std::string& text, const std::string& file) -> void
    {
        Diagnostics diagnostics;
        v2::V2Document document = v2::readV2Document(text, diagnostics, file);
        v2::resolveV2Document(document, diagnostics);
        for (const auto& d : diagnostics.all())
        {
            load_diagnostics.add(d);
            if (d.severity == Severity::Warning)
                thfun_logger->warn("{} {}", d.code, d.message);
        }
        // how the methods of each substance combine; a legacy record is read as it is, so only here
        for (const auto& record : document.substances)
        {
            const Diagnostics conflicts = v2::checkMethods(record);
            for (const auto& d : conflicts.all())
            {
                diagnostics.add(d);
                load_diagnostics.add(d);
                if (d.severity == Severity::Warning)
                    thfun_logger->warn("{} {}", d.code, d.message);
            }
        }
        if (!diagnostics.ok())
            funError("The file cannot be read", diagnostics.toString(), __LINE__, __FILE__);
        // a file with no thermodataset block (a part of a dataset) leaves the dataset as it is
        if (document.has_dataset)
        {
            conventions.water_triple_point_reference = document.info.water_triple_point_reference;
            dataset_info = document.info;
        }
        for (const auto& m : document.methods)
            methods[m.symbol] = m;
        for (const auto& p : document.parameter_sets)
            parameter_sets[p.symbol] = p;
        for (const auto& d : document.data_sources)
            data_sources[d.symbol] = d;
        for (auto& record : document.elements)
        {
            checkIfSymbolExists(elements_map, "element", record.symbol);
            const std::string symbol = record.symbol;
            elements_map[symbol] = Element(std::make_shared<const v2::ElementRecord>(std::move(record)));
        }
        for (auto& record : document.substances)
        {
            checkIfSymbolExists(substances_map, "substance", record.symbol);
            const std::string symbol = record.symbol;
            substances_map[symbol] = Substance(std::make_shared<const v2::SubstanceRecord>(std::move(record)));
        }
        for (auto& record : document.reactions)
        {
            checkIfSymbolExists(reactions_map, "reaction", record.symbol);
            const std::string symbol = record.symbol;
            reactions_map[symbol] = Reaction(std::make_shared<const v2::ReactionRecord>(std::move(record)));
        }
    }

    /// A v2 text: a file can include others, and a text has no folder to look in
    auto fromV2File(const std::string& text, const std::string& file) -> void
    {
        if (file.empty())
        {
            fromV2(text, file);
            return;
        }
        std::vector<std::string> stack;
        loaded_files.clear();
        loadV2File(file, stack);
    }

    // ---- files that include other files, and dataset folders ----

    static auto readWhole(const std::filesystem::path& p) -> std::string
    {
        std::ifstream ifs(p);
        if (!ifs.good())
            funError("File reading error", "File " + p.string() + " not found!", __LINE__, __FILE__);
        return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
    }

    // A pattern with * and ?
    static auto matches(const std::string& pattern, const std::string& name, size_t p = 0, size_t n = 0) -> bool
    {
        if (p == pattern.size())
            return n == name.size();
        if (pattern[p] == '*')
            return matches(pattern, name, p + 1, n) || (n < name.size() && matches(pattern, name, p, n + 1));
        return n < name.size() && (pattern[p] == '?' || pattern[p] == name[n]) && matches(pattern, name, p + 1, n + 1);
    }

    // The files of an include: the files of the folder that match, in sorted order
    static auto expand(const std::filesystem::path& pattern) -> std::vector<std::filesystem::path>
    {
        const std::string name = pattern.filename().string();
        if (name.find_first_of("*?") == std::string::npos)
            return {pattern};
        std::vector<std::filesystem::path> out;
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(pattern.parent_path(), ec))
            if (entry.is_regular_file() && matches(name, entry.path().filename().string()))
                out.push_back(entry.path());
        std::sort(out.begin(), out.end());
        return out;
    }

    /// Reads a file of the v2 format: the files that it includes first, each one once, then the file. A file with `record_type`
    /// holds one record.
    auto loadV2File(const std::filesystem::path& path, std::vector<std::string>& stack) -> void
    {
        const std::string key = std::filesystem::weakly_canonical(path).string();
        if (loaded_files.count(key))
            return;
        if (std::find(stack.begin(), stack.end(), key) != stack.end())
            funError("The files include each other", "V2-INC-001 The file " + key + " includes itself.", __LINE__, __FILE__);
        stack.push_back(key);
        const std::string text = readWhole(path);

        using ordered = nlohmann::ordered_json;
        ordered tree;
        if (v2::detail::looksLikeJson(text))
            tree = ordered::parse(text, nullptr, false);
        else
        {
            auto yaml = v2::detail::readYaml(text);
            if (yaml.ok)
                tree = std::move(yaml.tree);
            else
                tree = ordered::object();   // the reader reports the place in the text
        }
        if (tree.is_discarded() || !tree.is_object())
            funError("File reading error", "The file " + key + " is not a mapping.", __LINE__, __FILE__);

        if (tree.contains("include") && tree["include"].is_array())
            for (const auto& item : tree["include"])
                if (item.is_string())
                    for (const auto& file : expand(path.parent_path() / item.get<std::string>()))
                        loadV2File(file, stack);

        if (tree.contains("record_type") && tree["record_type"].is_string() && !tree.contains("thermohub"))
        {
            static const std::map<std::string, std::string> sections = {{"element", "elements"}, {"substance", "substances"}, {"reaction", "reactions"},
                                                                        {"method", "methods"}, {"parameter_set", "parameter_sets"}, {"data_source", "data_sources"}};
            const std::string type = tree["record_type"].get<std::string>();
            const auto section = sections.find(type);
            if (section == sections.end())
                funError("File reading error", "The file " + key + " has the record_type " + type + ".", __LINE__, __FILE__);
            ordered bundle = ordered::object();
            bundle["thermohub"] = "2.0.0-draft.1";
            tree.erase("record_type");
            bundle[section->second] = ordered::array({tree});
            fromV2(bundle.dump(), key);
        }
        else if (tree.contains("thermohub") || !v2::detail::looksLikeJson(text))
            fromV2(text, key);
        else
            fromFile(key);
        loaded_files.insert(key);
        stack.pop_back();
    }

    /// Reads a dataset folder: the records of the project in the nearest folder above it that has them, then thermodataset, then the
    /// files of elements/, substances/ and reactions/, in sorted order.
    auto loadDirectory(const std::filesystem::path& folder) -> void
    {
        namespace fs = std::filesystem;
        const auto dataFile = [](const fs::path& p) {
            const auto e = p.extension();
            return e == ".json" || e == ".yaml" || e == ".yml";
        };
        const auto filesIn = [&](const fs::path& where) {
            std::vector<fs::path> files;
            std::error_code ec;
            if (fs::is_directory(where, ec))
                for (const auto& entry : fs::recursive_directory_iterator(where, ec))
                    if (entry.is_regular_file() && dataFile(entry.path()))
                        files.push_back(entry.path());
            std::sort(files.begin(), files.end());
            return files;
        };
        fs::path dataset;
        for (const char* name : {"thermodataset.yaml", "thermodataset.yml", "thermodataset.json"})
            if (fs::exists(folder / name))
            {
                dataset = folder / name;
                break;
            }
        if (dataset.empty())
            funError("File reading error", "The folder " + folder.string() + " has no thermodataset.yaml or thermodataset.json.", __LINE__, __FILE__);

        std::vector<std::string> stack;
        loaded_files.clear();
        const fs::path own = fs::weakly_canonical(folder);
        for (fs::path p = own.parent_path(); p != p.parent_path(); p = p.parent_path())
            if (fs::is_directory(p / "methods") || fs::is_directory(p / "bibliography"))
            {
                for (const char* part : {"methods/definitions", "methods/parameter_sets", "bibliography"})
                    for (const auto& file : filesIn(p / part))
                        loadV2File(file, stack);
                break;
            }
        loadV2File(dataset, stack);
        for (const char* part : {"elements", "substances", "reactions"})
            for (const auto& file : filesIn(folder / part))
                loadV2File(file, stack);
    }

    /// Parses the JSON file (or string) and puts the data into the internal data structure
    /// @param filename name of the file (in the working directory)
    auto fromFile(std::string filename) -> void
    {
        try {

            {
                std::error_code ec;
                if (filename.size() < 4096 && std::filesystem::is_directory(filename, ec))
                {
                    loadDirectory(filename);
                    return;
                }
            }

            json j;
            std::string text = filename;
            std::string file;   // empty: the text was given, not a file

            try
            {
                // trying to see if the string is a valid json
                j = json::parse(filename);
            }
            catch (json::parse_error &e)
            {
                std::ifstream ifs(filename);
                if (ifs.good())
                {
                    file = filename;
                    text.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
                }
                else if (v2::detail::looksLikeJson(filename) || filename.find("thermohub") == std::string::npos)
                    funError("File reading error", std::string("Database file " + filename + " not found!"), __LINE__, __FILE__);

                if (v2::detail::looksLikeJson(text))
                    j = json::parse(text);
                else
                {
                    // YAML: the v2 reader reports what it finds in the text
                    fromV2File(text, file);
                    return;
                }
            }

            if (v2::detail::detectDocumentFormat(j) == v2::DataFormat::V2)
            {
                fromV2File(text, file);
                return;
            }

            // the data about the dataset, if the file is the first one of the database
            if (dataset_info.symbol.empty())
                dataset_info = v2::readLegacyDocumentInfo(text);

            if (j.contains("conventions"))
                conventions = readConventions(j["conventions"].dump());
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

namespace {

auto allRecords(const Database& db) -> v2::V2Records
{
    v2::V2Records out;
    out.dataset = db.thermoDataSet();
    out.dataset.water_triple_point_reference = db.conventions().water_triple_point_reference;
    for (const auto& item : db.mapElements())
        out.elements.push_back(*item.second.record());
    for (const auto& item : db.mapSubstances())
        out.substances.push_back(*item.second.record());
    for (const auto& item : db.mapReactions())
        out.reactions.push_back(*item.second.record());
    for (const auto& item : db.methods())
        out.methods.push_back(item.second);
    for (const auto& item : db.parameterSets())
        out.parameter_sets.push_back(item.second);
    for (const auto& item : db.dataSources())
        out.data_sources.push_back(item.second);
    return out;
}

} // namespace

auto Database::toJson(bool readable) const -> std::string
{
    return v2::writeV2Json(allRecords(*this), {readable});
}

auto Database::toYaml(bool readable) const -> std::string
{
    return v2::writeV2Yaml(allRecords(*this), {readable});
}

auto Database::toLegacyJson(bool useOriginalText) const -> std::string
{
    Diagnostics problems;
    const std::string text = v2::writeLegacyDocument(allRecords(*this), {useOriginalText}, &problems);
    for (const auto& d : problems.all())
        thfun_logger->warn("{} {}: {}", d.code, d.record, d.message);
    return text;
}

auto Database::legacyWriteProblems() const -> Diagnostics
{
    Diagnostics problems;
    v2::writeLegacyDocument(allRecords(*this), {false}, &problems);
    return problems;
}

namespace {

/// The checks of the conventions of the records (14 section 5 and section 7)
void checkConventions(const Database& db, const v2::V2Records& records, Diagnostics& all)
{
    const auto add = [&](Severity severity, const char* code, const std::string& record, const std::string& path, const std::string& message) {
        Diagnostic d;
        d.severity = severity;
        d.code = code;
        d.record = record;
        d.path = path;
        d.message = message;
        all.add(d);
    };
    const auto join = [](const std::vector<std::string>& words) {
        std::string text;
        for (const auto& w : words)
            text += (text.empty() ? "" : ", ") + w;
        return text;
    };
    const auto datasetReference = records.dataset.convention.energy_reference.value_or(v2::EnergyReference::BensonHelgeson);
    bool hasSolvent = false;
    std::map<std::string, v2::EnergyReference> references;
    for (const auto& r : records.substances)
    {
        hasSolvent = hasSolvent || r.class_ == v2::SubstanceClass::WaterSolvent;
        const v2::Convention own = r.convention.value_or(v2::Convention{});
        std::optional<v2::EnergyReference> native;
        for (const auto& a : r.methods)
            if (const auto* spec = v2::MethodCatalog::instance().find(a.method, v2::OperatesOn::Substance); spec && spec->effects.base && spec->native_convention)
                native = spec->native_convention;
        if (native && !own.energy_reference && *native != datasetReference)
            add(Severity::Warning, "CNV-003", r.symbol, "/substances/" + r.symbol + "/methods",
                "The method gives " + v2::toString(*native) + ", the dataset uses " + v2::toString(datasetReference) + ", and the record declares no convention. The values are converted.");
        const auto reference = native ? *native : own.energy_reference.value_or(datasetReference);
        references[r.symbol] = reference;
        if (reference != v2::EnergyReference::BensonHelgeson && !r.formula.empty())
        {
            const auto sources = db.elementEntropySources(r.formula);
            if (!sources.missing.empty())
                add(Severity::Error, "CNV-001", r.symbol, "/substances/" + r.symbol + "/formula", "A conversion needs the entropies of the elements " + join(sources.missing) + ", and no source has them.");
            if (!sources.from_table.empty())
                add(Severity::Warning, "CNV-006", r.symbol, "/substances/" + r.symbol + "/formula", "The entropies of the elements " + join(sources.from_table) + " come from the element table of ChemicalFun, not from the database.");
        }
    }
    for (const auto& r : records.substances)
        if (r.class_ == v2::SubstanceClass::AqueousSolute && !hasSolvent &&
            mergeConvention(r.convention.value_or(v2::Convention{}), records.dataset.convention).concentration_scale == v2::ConcentrationScale::Molarity)
            add(Severity::Error, "CNV-002", r.symbol, "/substances/" + r.symbol + "/convention", "A solute in molarity needs a solvent, and the database has none.");
    for (const auto& r : records.reactions)
    {
        std::set<v2::EnergyReference> seen;
        for (const auto& [symbol, coefficient] : r.stoichiometry)
            if (const auto it = references.find(symbol); it != references.end())
                seen.insert(it->second);
        if (seen.size() > 1)
            add(Severity::Warning, "CNV-004", r.symbol, "/reactions/" + r.symbol + "/equation", "The species of this reaction use different energy references. The values are converted.");
    }
}

} // namespace

auto Database::validate() const -> Diagnostics
{
    const auto parse = [this](const std::string& formula) {
        std::map<std::string, double> out;
        for (const auto& element : parseSubstanceFormula(formula))
            out[element.first.symbol()] += element.second;
        return out;
    };
    const auto records = allRecords(*this);
    Diagnostics all = v2::validateRecords(records, parse);
    for (const auto& record : records.substances)
    {
        const Diagnostics conflicts = v2::checkMethods(record);
        for (const auto& d : conflicts.all())
            all.add(d);
        if (v2::isHydrogenIon(record) && v2::hydrogenIonConventionIsZero(record, records.dataset.convention) && v2::recordHasOwnValues(record))
        {
            Diagnostic d;
            d.severity = Severity::Warning;
            d.code = "CNV-007";
            d.record = record.symbol;
            d.path = "/substances/" + record.symbol;
            d.message = "The geochemical convention defines the properties of the hydrogen ion as 0, and this record gives other values. They are used.";
            all.add(d);
        }
    }
    checkConventions(*this, records, all);
    return all;
}

auto Database::save(const std::string& path, v2::DataFormat format, bool readable) const -> void
{
    const auto endsWith = [&](const std::string& tail) { return path.size() >= tail.size() && path.compare(path.size() - tail.size(), tail.size(), tail) == 0; };
    const std::string text = format == v2::DataFormat::Legacy ? toLegacyJson(false) : endsWith(".yaml") || endsWith(".yml") ? toYaml(readable) : toJson(readable);
    std::ofstream out(path);
    if (!out.good())
        funError("File writing error", "The file " + path + " cannot be written.", __LINE__, __FILE__);
    out << text;
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

