// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "RecordBuilder.h"

#include <stdexcept>

#include <nlohmann/json.hpp>

#include "Database.h"
#include "Element.h"
#include "Reaction.h"
#include "Substance.h"

namespace ThermoFun {

using Json = nlohmann::ordered_json;

namespace {

auto parse(const RecordValue& value) -> Json
{
    return Json::parse(value.text());
}

auto object(const RecordFields& fields) -> Json
{
    Json out = Json::object();
    for (const auto& [key, value] : fields)
        out[key] = parse(value);
    return out;
}

} // namespace

RecordValue::RecordValue() : json_("null") {}
RecordValue::RecordValue(double value) : json_(Json(value).dump()) {}
RecordValue::RecordValue(int value) : json_(std::to_string(value)) {}
RecordValue::RecordValue(long long value) : json_(std::to_string(value)) {}
RecordValue::RecordValue(bool value) : json_(value ? "true" : "false") {}
RecordValue::RecordValue(const char* text) : json_(Json(std::string(text)).dump()) {}
RecordValue::RecordValue(const std::string& text) : json_(Json(text).dump()) {}

auto RecordValue::json(const std::string& text) -> RecordValue
{
    const Json j = Json::parse(text, nullptr, false);
    if (j.is_discarded())
        throw std::invalid_argument("RecordValue: the text is not JSON: " + text);
    RecordValue out;
    out.json_ = j.dump();
    return out;
}

auto RecordValue::list(const std::vector<RecordValue>& values) -> RecordValue
{
    Json j = Json::array();
    for (const auto& v : values)
        j.push_back(parse(v));
    RecordValue out;
    out.json_ = j.dump();
    return out;
}

auto RecordValue::object(const std::vector<std::pair<std::string, RecordValue>>& values) -> RecordValue
{
    RecordValue out;
    out.json_ = ThermoFun::object(values).dump();
    return out;
}

auto RecordValue::text() const -> const std::string&
{
    return json_;
}

struct RecordBuilder::Impl
{
    std::string kind, section, symbol;
    Json data = Json::object();
    Json methods = Json::array();
};

RecordBuilder::RecordBuilder(std::unique_ptr<Impl> impl) : pimpl(std::move(impl)) {}
RecordBuilder::RecordBuilder(const RecordBuilder& other) : pimpl(std::make_unique<Impl>(*other.pimpl)) {}
RecordBuilder::RecordBuilder(RecordBuilder&& other) noexcept = default;
auto RecordBuilder::operator=(const RecordBuilder& other) -> RecordBuilder&
{
    pimpl = std::make_unique<Impl>(*other.pimpl);
    return *this;
}
auto RecordBuilder::operator=(RecordBuilder&& other) noexcept -> RecordBuilder& = default;
RecordBuilder::~RecordBuilder() = default;

auto RecordBuilder::element(const std::string& symbol, const RecordFields& fields) -> RecordBuilder
{
    return RecordBuilder(std::make_unique<Impl>(Impl{"element", "elements", symbol})).set(fields);
}

auto RecordBuilder::substance(const std::string& symbol, const RecordFields& fields) -> RecordBuilder
{
    return RecordBuilder(std::make_unique<Impl>(Impl{"substance", "substances", symbol})).set(fields);
}

auto RecordBuilder::reaction(const std::string& symbol, const RecordFields& fields) -> RecordBuilder
{
    return RecordBuilder(std::make_unique<Impl>(Impl{"reaction", "reactions", symbol})).set(fields);
}

auto RecordBuilder::kind() const -> std::string { return pimpl->kind; }
auto RecordBuilder::section() const -> std::string { return pimpl->section; }
auto RecordBuilder::symbol() const -> const std::string& { return pimpl->symbol; }

auto RecordBuilder::set(const std::string& key, const RecordValue& value) -> RecordBuilder&
{
    pimpl->data[key] = parse(value);
    return *this;
}

auto RecordBuilder::set(const RecordFields& fields) -> RecordBuilder&
{
    for (const auto& [key, value] : fields)
        set(key, value);
    return *this;
}

auto RecordBuilder::standardState(const std::string& name, const RecordValue& value) -> RecordBuilder&
{
    if (pimpl->kind != "substance")
        throw std::logic_error("RecordBuilder: standard-state properties are of a substance, not of a " + pimpl->kind + ".");
    auto& block = pimpl->data["standard_state_properties"];
    if (!block.is_object())
        block = Json::object();
    block[name] = parse(value);
    return *this;
}

auto RecordBuilder::properties(const std::string& name, const RecordValue& value) -> RecordBuilder&
{
    if (pimpl->kind != "reaction")
        throw std::logic_error("RecordBuilder: reaction properties are of a reaction, not of a " + pimpl->kind + ".");
    auto& block = pimpl->data["reaction_properties"];
    if (!block.is_object())
        block = Json::object();
    block[name] = parse(value);
    return *this;
}

auto RecordBuilder::method(const RecordMethod& m) -> RecordBuilder&
{
    Json entry = {{"method", m.method}};
    if (!m.implementation.empty())
        entry["implementation"] = m.implementation;
    if (m.segments)
    {
        Json segments = Json::array();
        for (const auto& s : *m.segments)
        {
            Json one = Json::object();
            if (s.t_min)
                one["t_min"] = parse(*s.t_min);
            if (s.t_max)
                one["t_max"] = parse(*s.t_max);
            one["parameters"] = object(s.parameters);
            segments.push_back(one);
        }
        entry["segments"] = segments;
    }
    else if (!m.parameters.empty())
        entry["parameters"] = object(m.parameters);
    if (!m.parameter_set.empty())
        entry["parameter_set"] = m.parameter_set;
    if (m.transitions)
        entry["transitions"] = parse(*m.transitions);
    if (m.table)
        entry["table"] = parse(*m.table);
    if (m.applicability)
        entry["applicability"] = parse(*m.applicability);
    pimpl->methods.push_back(entry);
    return *this;
}

auto RecordBuilder::method(const std::string& name, const RecordFields& parameters) -> RecordBuilder&
{
    RecordMethod m;
    m.method = name;
    m.parameters = parameters;
    return method(m);
}

auto RecordBuilder::toJson() const -> std::string
{
    Json out = pimpl->data;
    if (!pimpl->methods.empty())
        out["methods"] = pimpl->methods;
    out["symbol"] = pimpl->symbol;
    return out.dump();
}

auto RecordBuilder::check() const -> Diagnostics
{
    return checkRecord(pimpl->section, pimpl->symbol, toJson());
}

auto RecordBuilder::elementRecord() const -> model::ElementRecord
{
    if (pimpl->kind != "element")
        throw std::logic_error("RecordBuilder: the record is a " + pimpl->kind + ", not an element.");
    return *Element(toJson()).record();
}

auto RecordBuilder::substanceRecord() const -> model::SubstanceRecord
{
    if (pimpl->kind != "substance")
        throw std::logic_error("RecordBuilder: the record is a " + pimpl->kind + ", not a substance.");
    return *Substance(toJson()).record();
}

auto RecordBuilder::reactionRecord() const -> model::ReactionRecord
{
    if (pimpl->kind != "reaction")
        throw std::logic_error("RecordBuilder: the record is a " + pimpl->kind + ", not a reaction.");
    return *Reaction(toJson()).record();
}

} // namespace ThermoFun
