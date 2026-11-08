// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// What the parts of the v2 reader share. Only for the .cpp files of this reader or writer.

#ifndef THERMOFUN_IO_V2_DETAIL_V2READERPARTS_H
#define THERMOFUN_IO_V2_DETAIL_V2READERPARTS_H

#include <ThermoFun/IO/V2/V2Reader.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <regex>
#include <set>
#include <tuple>
#include <nlohmann/json.hpp>
#include <ThermoFun/IO/V2/V2Keys.h>
#include <ThermoFun/IO/V2/detail/Normalize.h>
#include <ThermoFun/IO/detail/Yaml.h>
#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/Common/Ucum.h>
#include <ThermoFun/IO/Equation.h>

namespace ThermoFun::model::v2reader {

using json = nlohmann::ordered_json;

struct Context
{
    Diagnostics& diag;
    const ThermoDataSetInfo* dataset = nullptr;
    std::string file;
    std::map<std::string, MethodSpec> symbolic_specs;   ///< the symbolic Method records of the file, to read the applications that name them
};

// The format version that this reader knows
constexpr int knownMinor = 0;

void report(Context& c, Severity severity, const char* code, const std::string& record, const std::string& path,
            const std::string& message);

// the nlohmann type of Normalize is json; this file uses ordered_json to keep the order of a map: convert
auto plain(const json& node) -> nlohmann::json;

auto optionalString(const json& node, const char* key) -> std::string;

auto stringList(const json& node, const char* key) -> std::vector<std::string>;

// The keys that the reader knows. Other keys are kept in extra_json; one that starts with "_" is kept silently.
auto extraKeys(Context& c, const json& node, const std::set<std::string>& known, const std::string& record,
               const std::string& path) -> std::string;

auto quantity(Context& c, const json& node, const char* key, const std::string& slot, const UnitScope& scope,
              const std::string& record, const std::string& path) -> std::optional<Quantity>;

auto optionalInt(const json& node, const char* key) -> std::optional<int>;

auto readScope(Context& c, const json& node, const UnitScope* parent, const std::string& record, const std::string& path) -> UnitScope;

auto readApplicability(Context& c, const json& node, const UnitScope& scope, const std::string& record, const std::string& path)
    -> Applicability;

auto closest(const MethodSpec& spec, const std::string& name) -> std::string;

// the named parameters of a method (or of one segment)
auto readParameters(Context& c, const json& node, const MethodSpec& spec, const UnitScope& scope, const std::string& record,
                    const std::string& path, bool checkRequired) -> std::map<std::string, Quantity>;

auto readTable(Context& c, const json& node, const UnitScope& scope, const std::string& record, const std::string& path)
    -> std::optional<TableData>;

auto readMethods(Context& c, const json& list, OperatesOn operatesOn, const UnitScope& parent, const std::string& record,
                 const std::string& path) -> std::vector<MethodApplication>;

auto readConvention(Context& c, const json& node, const std::string& record, const std::string& path) -> std::optional<Convention>;

// The source that a record prefers; `record` is a setting of the engine, not of a file
auto readPropertySource(Context& c, const json& node, const std::string& record, const std::string& path) -> std::optional<PropertySource>;

// The covariance of values of the record. The checks of the values are in validate (V2-COV-001, 002, 004).
auto readCovariances(Context& c, const json& node, const std::string& record, const std::string& path) -> std::vector<CovarianceBlock>;

auto readReferenceConditions(Context& c, const json& node, const UnitScope& scope, const std::string& record,
                             const std::string& path, ReferenceConditions fallback) -> ReferenceConditions;

auto symbolOf(Context& c, const json& node, const std::string& key, const std::string& path) -> std::string;

auto readElement(Context& c, const json& node, const std::string& key, const UnitScope& parent, const std::string& path) -> ElementRecord;

auto readSubstance(Context& c, const json& node, const std::string& key, const UnitScope& parent, const std::string& path) -> SubstanceRecord;

auto readReaction(Context& c, const json& node, const std::string& key, const UnitScope& parent, const std::string& path) -> ReactionRecord;

auto effectsText(const json& node) -> std::string;

auto readMethodRecord(Context& c, const json& node, const std::string& key, const UnitScope&, const std::string& path) -> MethodDefinition;

auto readParameterSet(Context& c, const json& node, const std::string& key, const UnitScope& scope, const std::string& path,
                      const std::vector<MethodDefinition>& methods) -> MethodParameterSet;

auto readDataSource(Context& c, const json& node, const std::string& key, const std::string& path) -> DataSource;

auto readDataset(Context& c, const json& node, const UnitScope& scope) -> ThermoDataSetInfo;

void versionCheck(Context& c, const json& root);

auto readDocument(const json& root, Diagnostics& diagnostics, const std::string& file) -> V2Document;

// A word of a vocabulary: an unknown word gives a warning with the words allowed, and no value
template <class E>
auto vocabulary(Context& c, const std::string& word, const std::string& record, const std::string& path) -> std::optional<E>
{
    if (word.empty())
        return std::nullopt;
    const auto value = fromString<E>(word);
    if (!value)
        report(c, Severity::Warning, "V2-VOC-001", record, path, unknownWordMessage<E>(word));
    return value;
}

// a map symbol -> record, or a list of records with a symbol
template <class Record, class Reader>
void readRecords(Context& c, const json& node, const UnitScope& scope, const std::string& path, std::vector<Record>& out, Reader reader)
{
    std::set<std::string> seen;
    auto add = [&](Record&& r) {
        if (r.symbol.empty())
            return;
        if (!seen.insert(r.symbol).second)
            report(c, Severity::Warning, "V2-ID-003", r.symbol, path, "The symbol " + r.symbol + " is used twice; the later record replaces the earlier.");
        out.push_back(std::move(r));
    };
    if (node.is_object())
        for (const auto& item : node.items())
            add(reader(c, item.value(), item.key(), scope, path + "/" + item.key()));
    else if (node.is_array())
    {
        int i = 0;
        for (const auto& item : node)
            add(reader(c, item, std::string(), scope, path + "/" + std::to_string(i++)));
    }
}

template <class Record, class Reader>
auto readOne(const std::string& text, Diagnostics& diagnostics, Reader reader) -> Record
{
    Context c{diagnostics, nullptr, ""};
    json node = json::parse(text, nullptr, false);
    if (node.is_discarded())
    {
        report(c, Severity::Error, "V2-ID-001", "", "/", "The text is not JSON.");
        return Record{};
    }
    UnitScope scope;
    return reader(c, node, std::string(), scope, "");
}

} // namespace ThermoFun::model::v2reader

#endif
