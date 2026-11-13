// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Builders of v2 records, field by field, with values as in a file: a number, a text "number unit", or a map
// {value, unit, uncertainty}. The builders write ordinary v2 records; the reader checks them.
//
//     auto quartz = RecordBuilder::substance("Quartz", {{"formula", "SiO2"}, {"aggregate_state", "crystal"}, {"class", "component"}});
//     quartz.standardState("G", -856288.0).standardState("V", "22.688 cm3/mol")
//           .method("MurnaghanHP98", {{"alpha0", 1.24e-5}, {"kappa0", "730 kbar"}});
//     Diagnostics problems = quartz.check();
//     database.addSubstance(quartz.toJson());

#ifndef THERMOFUN_RECORD_BUILDER_H
#define THERMOFUN_RECORD_BUILDER_H

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <ThermoFun/Common/Diagnostics.h>
#include <ThermoFun/Model/ElementRecord.h>
#include <ThermoFun/Model/ReactionRecord.h>
#include <ThermoFun/Model/SubstanceRecord.h>

namespace ThermoFun {

/// A value of a field of a record, kept as its JSON text: a number, a text, true or false, null, a list or a map.
class RecordValue
{
public:
    RecordValue();  ///< null
    RecordValue(double value);
    RecordValue(int value);
    RecordValue(long long value);
    RecordValue(bool value);
    RecordValue(const char* text);
    RecordValue(const std::string& text);

    /// A value from JSON text. Throws std::invalid_argument if the text is not JSON.
    static auto json(const std::string& text) -> RecordValue;
    /// A list of values.
    static auto list(const std::vector<RecordValue>& values) -> RecordValue;
    /// A map of values, in this order.
    static auto object(const std::vector<std::pair<std::string, RecordValue>>& values) -> RecordValue;

    /// The JSON text.
    auto text() const -> const std::string&;

private:
    std::string json_;
};

/// Named values, in order.
using RecordFields = std::vector<std::pair<std::string, RecordValue>>;

/// A segment of a method that is piecewise in T.
struct RecordSegment
{
    std::optional<RecordValue> t_min, t_max;
    RecordFields parameters;
};

/// A method of a record, as in a file.
struct RecordMethod
{
    std::string method;
    std::string implementation;                    ///< empty: the default implementation
    RecordFields parameters;                       ///< left out if empty, or if there are segments
    std::optional<std::vector<RecordSegment>> segments;
    std::string parameter_set;                     ///< empty: none
    std::optional<RecordValue> transitions, table, applicability;
};

/// Builds an element, substance or reaction record.
class RecordBuilder
{
public:
    static auto element(const std::string& symbol, const RecordFields& fields = {}) -> RecordBuilder;    ///< number, molar_mass, class, ...
    static auto substance(const std::string& symbol, const RecordFields& fields = {}) -> RecordBuilder;  ///< formula, class, aggregate_state, charge, name, ...
    static auto reaction(const std::string& symbol, const RecordFields& fields = {}) -> RecordBuilder;   ///< equation or stoichiometry, name, ...

    RecordBuilder(const RecordBuilder& other);
    RecordBuilder(RecordBuilder&& other) noexcept;
    auto operator=(const RecordBuilder& other) -> RecordBuilder&;
    auto operator=(RecordBuilder&& other) noexcept -> RecordBuilder&;
    ~RecordBuilder();

    /// "element", "substance" or "reaction".
    auto kind() const -> std::string;
    /// "elements", "substances" or "reactions": the section of a file.
    auto section() const -> std::string;
    auto symbol() const -> const std::string&;

    /// Sets a field of the record (replaces it if it is there).
    auto set(const std::string& key, const RecordValue& value) -> RecordBuilder&;
    auto set(const RecordFields& fields) -> RecordBuilder&;

    /// A standard-state property G, H, S, Cp, V of a substance. Throws std::logic_error for another kind of record.
    auto standardState(const std::string& name, const RecordValue& value) -> RecordBuilder&;
    /// A reaction property logK, dG, dH, dS, dCp, dV of a reaction. Throws std::logic_error for another kind of record.
    auto properties(const std::string& name, const RecordValue& value) -> RecordBuilder&;

    /// Adds a method.
    auto method(const RecordMethod& method) -> RecordBuilder&;
    auto method(const std::string& name, const RecordFields& parameters = {}) -> RecordBuilder&;

    /// The record as JSON text, with its symbol.
    auto toJson() const -> std::string;

    /// The problems of this record, now: from the reader and from validate. The record is alone here, so a reference to
    /// another record is not a problem.
    auto check() const -> Diagnostics;

    /// The record, read. Throws if it is of another kind, or if the reader refuses it.
    auto elementRecord() const -> model::ElementRecord;
    auto substanceRecord() const -> model::SubstanceRecord;
    auto reactionRecord() const -> model::ReactionRecord;

private:
    struct Impl;
    explicit RecordBuilder(std::unique_ptr<Impl> impl);
    std::unique_ptr<Impl> pimpl;
};

} // namespace ThermoFun

#endif
