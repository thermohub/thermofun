// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Convert.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <stdexcept>
#include <vector>

#include <nlohmann/json.hpp>

#include "Common/Exception.h"
#include "Reaction.h"
#include "Substance.h"

namespace ThermoFun {

namespace {

void numbers(const nlohmann::json& node, std::vector<double>& out)
{
    if (node.is_structured())
        for (const auto& value : node)
            numbers(value, out);
    else if (node.is_number())
        out.push_back(node.get<double>());
}

auto format(const char* pattern, double value) -> std::string
{
    char text[64];
    std::snprintf(text, sizeof text, pattern, value);
    return text;
}

void writeText(const std::string& path, const std::string& text)
{
    const std::filesystem::path file(path);
    if (file.has_parent_path())
        std::filesystem::create_directories(file.parent_path());
    std::ofstream out(file, std::ios::binary);
    if (!out.good())
        funError("File writing error", "The file " + path + " cannot be written.", __LINE__, __FILE__);
    out << text;
    if (text.empty() || text.back() != '\n')
        out << '\n';
}

/// A section of the report: for each code, the count and the first five records.
void table(std::string& out, const std::string& title, const std::vector<Diagnostic>& items)
{
    out += "## " + title + "\n\n";
    if (items.empty())
    {
        out += "None.\n\n";
        return;
    }
    std::map<std::string, std::pair<std::size_t, std::vector<std::string>>> counts;
    for (const auto& d : items)
    {
        auto& entry = counts[d.code];
        ++entry.first;
        if (!d.record.empty() && entry.second.size() < 5 && std::find(entry.second.begin(), entry.second.end(), d.record) == entry.second.end())
            entry.second.push_back(d.record);
    }
    out += "| Code | Count | First records |\n|---|---|---|\n";
    for (const auto& [code, entry] : counts)
    {
        std::string records;
        for (const auto& r : entry.second)
            records += (records.empty() ? "" : ", ") + r;
        out += "| " + code + " | " + std::to_string(entry.first) + " | " + records + " |\n";
    }
    out += "\n";
}

auto startsWith(const std::string& text, const std::string& head) -> bool
{
    return text.rfind(head, 0) == 0;
}

} // namespace

auto shortenedNumbers(const Database& database) -> std::optional<ShortenedNumbers>
{
    std::vector<double> exact, shortened;
    numbers(nlohmann::json::parse(database.toJson(false)), exact);
    numbers(nlohmann::json::parse(database.toJson(true)), shortened);
    if (exact.size() != shortened.size())
        return std::nullopt;
    ShortenedNumbers out;
    for (std::size_t i = 0; i < exact.size(); ++i)
        if (exact[i] != shortened[i])
        {
            ++out.count;
            out.largest = std::max(out.largest, std::abs(exact[i] - shortened[i]) / std::max(std::abs(exact[i]), 1e-300));
        }
    return out;
}

auto conversionReport(const Database& database, const std::string& input, const ConvertOptions& options) -> std::string
{
    std::vector<Diagnostic> load = database.loadDiagnostics().all();
    const Diagnostics legacy = legacyFileDiagnostics(input);
    for (const auto& d : legacy.all())
        load.push_back(d);
    const Diagnostics validated = database.validate();
    const auto& check = validated.all();
    const bool v2 = options.to != model::DataFormat::Legacy;

    std::string out = "# Conversion report\n\n## Summary\n\n";
    out += "- input: `" + input + "`\n";
    out += std::string("- output format: ") + (v2 ? "v2" : "legacy") + "\n";
    out += "- elements: " + std::to_string(database.numberOfElements()) + ", substances: " + std::to_string(database.numberOfSubstances()) +
           ", reactions: " + std::to_string(database.numberOfReactions()) + "\n";
    out += "- methods: " + std::to_string(database.methods().size()) + ", parameter sets: " + std::to_string(database.parameterSets().size()) +
           ", data sources: " + std::to_string(database.dataSources().size()) + "\n";
    if (v2 && options.readable)
    {
        const auto shortened = shortenedNumbers(database);
        if (shortened)
            out += "- numbers written as short decimals (--readable): " + std::to_string(shortened->count) + ", the largest relative change " +
                   format("%.2g", shortened->largest) + "; results can differ by that much from the input. Convert without --readable to keep every number.\n";
    }
    else if (v2)
        out += "- numbers written exactly: a result from the output is the same as from the input\n";
    out += "\n";

    const auto select = [](const std::vector<Diagnostic>& from, auto keep) {
        std::vector<Diagnostic> out;
        std::copy_if(from.begin(), from.end(), std::back_inserter(out), keep);
        return out;
    };
    table(out, "Quirks applied", select(load, [](const Diagnostic& d) { return startsWith(d.code, "LEG-QRK"); }));
    table(out, "Unread legacy keys", select(load, [](const Diagnostic& d) { return d.code == "LEG-KEY-001" || d.code == "LEG-KEY-002"; }));
    table(out, "Unresolved references", select(check, [](const Diagnostic& d) { return startsWith(d.code, "V2-REF"); }));
    table(out, "Other problems of the data", select(check, [](const Diagnostic& d) { return !startsWith(d.code, "V2-REF"); }));

    std::map<std::string, std::size_t> used;
    for (const auto& item : database.mapSubstances())
        for (const auto& m : item.second.record()->methods)
            ++used[m.method];
    for (const auto& item : database.mapReactions())
        for (const auto& m : item.second.record()->methods)
            ++used[m.method];
    out += "## Methods used\n\n| Method | Records |\n|---|---|\n";
    for (const auto& [method, count] : used)
        out += "| " + method + " | " + std::to_string(count) + " |\n";
    out += "\n## Uncertainty convention notice\n\n"
           "A legacy `error` is read as a standard uncertainty (coverage factor 1). The legacy format does not say if an error is a standard "
           "uncertainty, an expanded one or a confidence interval. Check the source of the data before you use the uncertainties.\n";
    return out;
}

auto convertFile(const std::string& input, const std::string& output, const ConvertOptions& options) -> Diagnostics
{
    if (options.to != model::DataFormat::V2 && options.to != model::DataFormat::Legacy)
        throw std::invalid_argument("convertFile: the format to write is V2 or Legacy.");
    if (options.to == model::DataFormat::Legacy && (options.split || options.yaml))
        throw std::invalid_argument("convertFile: the legacy format is one JSON file.");
    const Database database(input);
    if (options.to == model::DataFormat::Legacy)
        writeText(output, database.toLegacyJson(true));
    else if (options.split)
        database.saveDataset(output, options.yaml, options.readable);
    else
        writeText(output, options.yaml ? database.toYaml(options.readable) : database.toJson(options.readable));
    if (!options.report.empty())
        writeText(options.report, conversionReport(database, input, options));
    return database.loadDiagnostics();
}

} // namespace ThermoFun
