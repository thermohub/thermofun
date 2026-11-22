// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Reads and changes a value of a record by its path (a JSON pointer), and summarizes a database.

#include "DatabaseImpl.h"
#include <ThermoFun/IO/V2/V2Keys.h>
#include <ThermoFun/IO/V2/detail/V2WriterParts.h>
#include <optional>
#include <sstream>
#include <stdexcept>

namespace ThermoFun {

namespace {

using json = nlohmann::json;

struct RecordPath
{
    std::string section, symbol;
    std::string rest;   ///< the JSON pointer inside the record, empty for the record
};

/// "/substances/Ca+2/methods/0" -> section, symbol and "/methods/0". The symbol is unescaped (~1 is /, ~0 is ~).
auto splitPath(const std::string& path) -> RecordPath
{
    if (path.empty() || path[0] != '/')
        throw std::invalid_argument("The path '" + path + "' does not start with '/'. Write the section, the symbol and the keys, for example /substances/Calcite/standard_state_properties/G.");
    RecordPath out;
    std::size_t first = path.find('/', 1);
    out.section = path.substr(1, first == std::string::npos ? std::string::npos : first - 1);
    if (first == std::string::npos)
        throw std::invalid_argument("The path '" + path + "' has no symbol. Write /" + out.section + "/<symbol>.");
    std::size_t second = path.find('/', first + 1);
    std::string symbol = path.substr(first + 1, second == std::string::npos ? std::string::npos : second - first - 1);
    for (std::size_t i = 0; i + 1 < symbol.size(); ++i)
        if (symbol[i] == '~' && (symbol[i + 1] == '1' || symbol[i + 1] == '0'))
            symbol.replace(i, 2, symbol[i + 1] == '1' ? "/" : "~");
    out.symbol = symbol;
    out.rest = second == std::string::npos ? "" : path.substr(second);
    if (out.section != "elements" && out.section != "substances" && out.section != "reactions")
        throw std::invalid_argument("The section '" + out.section + "' is not elements, substances or reactions.");
    return out;
}

auto recordJson(const Database& db, const RecordPath& where) -> json
{
    const std::string missing = "The database has no " + where.section.substr(0, where.section.size() - 1) + " " + where.symbol + ".";
    const model::V2WriterOptions exact{false};
    if (where.section == "elements")
    {
        if (!db.containsElement(where.symbol))
            throw std::invalid_argument(missing);
        return json::parse(model::writeV2Element(*db.getElement(where.symbol).record(), exact));
    }
    if (where.section == "substances")
    {
        if (!db.containsSubstance(where.symbol))
            throw std::invalid_argument(missing);
        return json::parse(model::writeV2Substance(*db.getSubstance(where.symbol).record(), exact));
    }
    if (!db.containsReaction(where.symbol))
        throw std::invalid_argument(missing);
    return json::parse(model::writeV2Reaction(*db.getReaction(where.symbol).record(), exact));
}

auto pointerOf(const std::string& rest) -> json::json_pointer
{
    try
    {
        return json::json_pointer(rest);
    }
    catch (const json::exception& error)
    {
        throw std::invalid_argument("The path '" + rest + "' is not valid: " + error.what());
    }
}

// A key of a quantity (value, unit, uncertainty, ...). A record may write a quantity as a bare number: the path of one of its
// keys then names the number (value) or a key that the number does not write yet (uncertainty).
auto isQuantityKey(const std::string& key) -> bool
{
    namespace k = model::v2key;
    for (const char* name : {k::value, k::unit, k::uncertainty, k::quality, k::origin, k::derivation, k::source})
        if (key == name)
            return true;
    return false;
}

// The parent of the pointer if it is a quantity written as a bare number and the pointer names one of its keys
auto bareQuantityParent(const json& record, const json::json_pointer& pointer) -> std::optional<json::json_pointer>
{
    if (pointer.empty() || !isQuantityKey(pointer.back()))
        return std::nullopt;
    const auto parent = pointer.parent_pointer();
    if (parent.empty() || !record.contains(parent) || !record.at(parent).is_number())
        return std::nullopt;
    return parent;
}

} // namespace

auto Database::value(const std::string& path) const -> std::string
{
    const auto where = splitPath(path);
    const json record = recordJson(*this, where);
    if (where.rest.empty())
        return record.dump();
    const auto pointer = pointerOf(where.rest);
    if (const auto parent = bareQuantityParent(record, pointer); parent && pointer.back() == model::v2key::value)
        return record.at(*parent).dump();
    if (!record.contains(pointer))
        throw std::invalid_argument("The record " + where.symbol + " has no value at " + where.rest + ".");
    return record.at(pointer).dump();
}

auto Database::setValue(const std::string& path, const std::string& jsonValue) -> void
{
    const auto where = splitPath(path);
    json value;
    try
    {
        value = json::parse(jsonValue);
    }
    catch (const json::exception& error)
    {
        throw std::invalid_argument(std::string("The value is not JSON text: ") + error.what());
    }
    json record = where.rest.empty() && !(containsSubstance(where.symbol) || containsReaction(where.symbol) || containsElement(where.symbol)) ? json::object() : recordJson(*this, where);
    if (where.rest.empty())
        record = value;
    else
        try
        {
            const auto pointer = pointerOf(where.rest);
            if (const auto parent = bareQuantityParent(record, pointer))
                record[*parent] = json{{model::v2key::value, record.at(*parent)}};
            record[pointer] = value;
        }
        catch (const json::exception& error)
        {
            throw std::invalid_argument("Cannot set " + where.rest + " in the record " + where.symbol + ": " + error.what());
        }
    record["symbol"] = where.symbol;
    const std::string text = record.dump();
    if (where.section == "elements")
        setElement(text);
    else if (where.section == "substances")
        setSubstance(text);
    else
        setReaction(text);
}

auto Database::formatVersion() -> std::string
{
    return model::v2writer::formatVersion;
}

auto Database::summary() const -> DatabaseSummary
{
    DatabaseSummary out;
    out.name = thermoDataSet().name.empty() ? thermoDataSet().symbol : thermoDataSet().name;
    out.elements = numberOfElements();
    out.substances = numberOfSubstances();
    out.reactions = numberOfReactions();
    std::map<std::pair<std::string, std::string>, std::size_t> counts;
    for (const auto& item : mapSubstances())
    {
        const auto& record = *item.second.record();
        ++counts[{model::toString(record.class_), model::toString(record.aggregate_state)}];
        if (!record.methods.empty())
            ++out.substances_with_methods;
        if (!record.defining_reaction.empty())
            ++out.substances_from_reaction;
    }
    for (const auto& [key, count] : counts)
        out.classes.push_back({key.first, key.second, count});
    return out;
}

auto DatabaseSummary::text() const -> std::string
{
    std::ostringstream out;
    out << (name.empty() ? "database" : name) << ": " << elements << " elements, " << substances << " substances, " << reactions << " reactions\n";
    out << substances_with_methods << " substances have methods, " << substances_from_reaction << " have a defining reaction\n";
    std::size_t width = 5, width2 = 15;
    for (const auto& c : classes)
    {
        width = std::max(width, c.substance_class.size());
        width2 = std::max(width2, c.aggregate_state.size());
    }
    const auto pad = [](const std::string& s, std::size_t n) { return s + std::string(n > s.size() ? n - s.size() : 0, ' '); };
    out << pad("class", width) << "  " << pad("aggregate_state", width2) << "  substances\n";
    for (const auto& c : classes)
        out << pad(c.substance_class, width) << "  " << pad(c.aggregate_state, width2) << "  " << c.substances << "\n";
    return out.str();
}

} // namespace ThermoFun
