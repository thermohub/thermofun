// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <ThermoFun/Model/BuiltinParameterSets.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <set>
#include <sstream>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/IO/V2/V2Reader.h>

namespace ThermoFun::v2 {

namespace detail {

/// One file of Methods/ParameterSets. The build generates the table.
struct BuiltinFile
{
    const char* name;
    const unsigned char* text;
    unsigned long size;
};
extern const BuiltinFile builtin_parameter_files[];
extern const unsigned long builtin_parameter_file_count;

} // namespace detail

namespace {

auto readSets(const std::string& text, const std::string& name) -> std::vector<MethodParameterSet>
{
    Diagnostics diagnostics;
    auto document = readV2Document(text, diagnostics, name);
    if (!diagnostics.ok())
        funError("The constants of the methods in " + name + " cannot be read", diagnostics.toString(), __LINE__, __FILE__);
    return document.parameter_sets;
}

/// HKF88.yaml and HKF88-gems.yaml hold the sets of HKF88.
auto methodOfFile(const std::string& name) -> std::string
{
    const auto stem = std::filesystem::path(name).stem().string();
    return stem.substr(0, stem.find('-'));
}

auto readMethod(const std::string& method) -> std::vector<MethodParameterSet>
{
    std::vector<MethodParameterSet> out;
    std::set<std::string> defaults;
    for (unsigned long i = 0; i < detail::builtin_parameter_file_count; ++i)
    {
        const auto& file = detail::builtin_parameter_files[i];
        if (methodOfFile(file.name) != method)
            continue;
        for (auto& set : readSets(std::string(reinterpret_cast<const char*>(file.text), file.size), file.name))
        {
            if (set.method != method)
                funError(std::string("The file of the constants ") + file.name + " is named after another method", set.symbol, __LINE__, __FILE__);
            if (set.is_default && !defaults.insert(set.implementation).second)
                funError("Two default sets of constants for the method " + method, set.symbol + ": give the other one default: false", __LINE__, __FILE__);
            out.push_back(std::move(set));
        }
    }
    return out;
}

auto filesInPath() -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> out;
    const char* value = std::getenv("THERMOFUN_PARAMETER_PATH");
    std::stringstream folders(value ? value : "");
#ifdef _WIN32
    const char separator = ';';
#else
    const char separator = ':';
#endif
    for (std::string folder; std::getline(folders, folder, separator);)
    {
        std::error_code ignored;
        for (const auto& entry : std::filesystem::directory_iterator(folder, ignored))
            if (const auto extension = entry.path().extension(); extension == ".yaml" || extension == ".yml" || extension == ".json")
                out.push_back(entry.path());
    }
    return out;
}

} // namespace

auto builtinSetsOf(const std::string& method) -> const std::vector<MethodParameterSet>&
{
    static std::mutex mutex;
    static std::map<std::string, std::vector<MethodParameterSet>> read;  // a node of a map does not move
    const std::lock_guard<std::mutex> lock(mutex);
    const auto it = read.find(method);
    return it != read.end() ? it->second : read.emplace(method, readMethod(method)).first->second;
}

auto builtinConstants(const std::string& method, const std::string& implementation) -> const MethodParameterSet*
{
    const auto& sets = builtinSetsOf(method);
    for (const auto* wanted : {&implementation, static_cast<const std::string*>(nullptr)})
        for (const auto& set : sets)
            if (set.is_default && (wanted ? set.implementation == *wanted : set.implementation.empty()))
                return &set;
    return nullptr;
}

auto builtinParameterSets() -> const std::map<std::string, MethodParameterSet>&
{
    static const auto sets = [] {
        std::map<std::string, MethodParameterSet> out;
        for (unsigned long i = 0; i < detail::builtin_parameter_file_count; ++i)
            for (const auto& set : builtinSetsOf(methodOfFile(detail::builtin_parameter_files[i].name)))
                out[set.symbol] = set;
        return out;
    }();
    return sets;
}

auto parameterSetVariant(const std::string& symbol) -> const MethodParameterSet*
{
    static const auto sets = [] {
        auto out = builtinParameterSets();
        for (const auto& path : filesInPath())
        {
            std::ifstream file(path);
            std::stringstream text;
            text << file.rdbuf();
            for (auto& set : readSets(text.str(), path.string()))
                out.emplace(set.symbol, std::move(set));  // a set of the library stays
        }
        return out;
    }();
    const auto it = sets.find(symbol);
    return it == sets.end() ? nullptr : &it->second;
}

} // namespace ThermoFun::v2

namespace ThermoFun {

auto waterTripleData() -> const std::unordered_map<std::string, WaterTripleProperties>&
{
    static const auto data = [] {
        std::unordered_map<std::string, WaterTripleProperties> out;
        for (const auto& set : v2::builtinSetsOf("WaterTriplePoint"))
        {
            const auto joules = [&set = set](const char* name) { return set.parameters.at(name).value * cal_to_J; };
            out[set.implementation] = {joules("Str"), joules("Gtr"), joules("Htr"), joules("Utr"), joules("Atr")};
        }
        return out;
    }();
    return data;
}

} // namespace ThermoFun
