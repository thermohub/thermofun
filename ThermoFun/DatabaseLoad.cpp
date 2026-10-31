// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Reads records into a database: legacy files, v2 files and dataset folders.

#include "DatabaseImpl.h"

namespace ThermoFun {

auto Database::Impl::addRecord(json j, std::string _label) -> void
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

auto Database::Impl::addRecords(json j, std::string _label) -> void
{
    for(auto it = j.begin(); it != j.end(); ++it)
    {
        addRecord(it.value(), _label);
    }
}

/// Puts the records of a v2 document into the database. Warnings are logged; an error stops the reading.
auto Database::Impl::fromV2(const std::string& text, const std::string& file) -> void
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
auto Database::Impl::fromV2File(const std::string& text, const std::string& file) -> void
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

auto Database::Impl::readWhole(const std::filesystem::path& p) -> std::string
{
    std::ifstream ifs(p);
    if (!ifs.good())
        funError("File reading error", "File " + p.string() + " not found!", __LINE__, __FILE__);
    return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

// A pattern with * and ?
auto Database::Impl::matches(const std::string& pattern, const std::string& name, size_t p, size_t n) -> bool
{
    if (p == pattern.size())
        return n == name.size();
    if (pattern[p] == '*')
        return matches(pattern, name, p + 1, n) || (n < name.size() && matches(pattern, name, p, n + 1));
    return n < name.size() && (pattern[p] == '?' || pattern[p] == name[n]) && matches(pattern, name, p + 1, n + 1);
}

// The files of an include: the files of the folder that match, in sorted order
auto Database::Impl::expand(const std::filesystem::path& pattern) -> std::vector<std::filesystem::path>
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
auto Database::Impl::loadV2File(const std::filesystem::path& path, std::vector<std::string>& stack) -> void
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
auto Database::Impl::loadDirectory(const std::filesystem::path& folder) -> void
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
auto Database::Impl::fromFile(std::string filename) -> void
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

auto Database::Impl::fromJSONs(std::vector<std::string> jsons, std::string _label) -> void
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

} // namespace ThermoFun
