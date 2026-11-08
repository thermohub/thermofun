// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <ThermoFun/Batch/RecordRules.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <set>

#include <nlohmann/json.hpp>

#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Database.h>
#include <ThermoFun/Element.h>
#include <ThermoFun/IO/Legacy/LegacyVocabulary.h>
#include <ThermoFun/IO/V2/V2Reader.h>
#include <ThermoFun/Model/Vocabulary.h>
#include <ThermoFun/Reaction.h>
#include <ThermoFun/Substance.h>

namespace ThermoFun {

TableSetupError::~TableSetupError() = default;


namespace {

using json = nlohmann::ordered_json;

const std::vector<std::string> substanceKeys = {"class", "aggregate_state", "elements", "only_elements", "charge", "match", "exclude"};
const std::vector<std::string> reactionKeys = {"elements", "only_elements", "involving", "match", "exclude"};

/// The charge is not an element of a rule.
const std::string chargeElement = "Zz";

auto editDistance(const std::string& a, const std::string& b) -> std::size_t
{
    std::vector<std::size_t> row(b.size() + 1);
    std::iota(row.begin(), row.end(), std::size_t{0});
    for (std::size_t i = 1; i <= a.size(); ++i)
    {
        std::size_t diagonal = row[0];
        row[0] = i;
        for (std::size_t j = 1; j <= b.size(); ++j)
        {
            const std::size_t above = row[j];
            row[j] = std::min({row[j] + 1, row[j - 1] + 1, diagonal + (a[i - 1] == b[j - 1] ? 0 : 1)});
            diagonal = above;
        }
    }
    return row[b.size()];
}

auto ruleOf(const std::string& text) -> json
{
    json rule;
    try
    {
        rule = json::parse(text);
    }
    catch (const json::exception&)
    {
        try
        {
            // the reader of YAML starts from a block
            rule = json::parse(model::yamlToJson("rule: " + text))["rule"];
        }
        catch (const std::exception& error)
        {
            throw TableSetupError("The rule cannot be read: " + std::string(error.what()));
        }
    }
    if (!rule.is_object())
        throw TableSetupError("A rule is a mapping, for example {class: aqueous_solute}, not " + rule.dump() + ".");
    return rule;
}

auto checkKeys(const json& rule, const std::vector<std::string>& known) -> void
{
    if (rule.empty())
        throw TableSetupError("The rule is empty: it would choose all records. Give at least one key, or match: \"*\".");
    for (const auto& item : rule.items())
        if (std::find(known.begin(), known.end(), item.key()) == known.end())
            throw TableSetupError(detail::unknownKeyMessage("A rule", item.key(), known));
}

/// A word or a list of words.
auto wordsOf(const json& rule, const char* key) -> std::vector<std::string>
{
    std::vector<std::string> out;
    if (!rule.contains(key))
        return out;
    const auto& node = rule[key];
    const auto add = [&](const json& x) {
        if (!x.is_string())
            throw TableSetupError(std::string(key) + " has a value that is not a text: " + x.dump() + ".");
        out.push_back(x.get<std::string>());
    };
    if (node.is_array())
        for (const auto& x : node)
            add(x);
    else
        add(node);
    return out;
}

template <class E>
auto vocabularyWords(const json& rule, const char* key) -> std::set<E>
{
    std::set<E> out;
    for (const auto& word : wordsOf(rule, key))
    {
        const auto value = model::fromString<E>(word);
        if (!value)
            throw TableSetupError(model::unknownWordMessage<E>(word));
        out.insert(*value);
    }
    return out;
}

/// A symbol against a pattern with * and ?.
auto matches(const char* pattern, const char* text) -> bool
{
    if (*pattern == '\0')
        return *text == '\0';
    if (*pattern == '*')
        return matches(pattern + 1, text) || (*text != '\0' && matches(pattern, text + 1));
    return *text != '\0' && (*pattern == '?' || *pattern == *text) && matches(pattern + 1, text + 1);
}

auto symbolAllowed(const json& rule, const std::string& symbol) -> bool
{
    const auto included = wordsOf(rule, "match");
    if (!included.empty() && std::none_of(included.begin(), included.end(), [&](const std::string& p) { return matches(p.c_str(), symbol.c_str()); }))
        return false;
    const auto excluded = wordsOf(rule, "exclude");
    return std::none_of(excluded.begin(), excluded.end(), [&](const std::string& p) { return matches(p.c_str(), symbol.c_str()); });
}

auto elementsOf(const Database& database, const Substance& substance, std::set<std::string>& out) -> bool
{
    try
    {
        for (const auto& item : database.parseSubstanceFormula(substance.formula()))
            if (item.first.symbol() != chargeElement)
                out.insert(item.first.symbol());
        return true;
    }
    catch (const std::exception& error)
    {
        thfun_logger->warn("The rule cannot read the formula of {}, and leaves it out: {}", substance.symbol(), error.what());
        return false;
    }
}

auto elementsAllowed(const json& rule, const std::set<std::string>& elements) -> bool
{
    const auto needed = wordsOf(rule, "elements");
    if (std::any_of(needed.begin(), needed.end(), [&](const std::string& e) { return !elements.count(e); }))
        return false;
    if (!rule.contains("only_elements"))
        return true;
    const auto allowed = wordsOf(rule, "only_elements");
    return std::all_of(elements.begin(), elements.end(), [&](const std::string& e) { return std::find(allowed.begin(), allowed.end(), e) != allowed.end(); });
}

auto chosen(std::vector<std::string> symbols, const char* kind, const json& rule) -> std::vector<std::string>
{
    if (symbols.empty())
        throw TableSetupError(std::string("The rule ") + rule.dump() + " chooses no " + kind + " of the database.");
    return symbols;
}

} // namespace

auto selectSubstances(const Database& database, const std::string& text) -> std::vector<std::string>
{
    const json rule = ruleOf(text);
    checkKeys(rule, substanceKeys);
    const auto classes = vocabularyWords<model::SubstanceClass>(rule, "class");
    const auto states = vocabularyWords<model::AggregateState>(rule, "aggregate_state");
    const bool byElements = rule.contains("elements") || rule.contains("only_elements");
    std::vector<int> charges;
    if (rule.contains("charge"))
    {
        const auto& node = rule["charge"];
        for (const auto& x : node.is_array() ? node : json::array({node}))
        {
            if (!x.is_number_integer())
                throw TableSetupError("charge has a value that is not a whole number: " + x.dump() + ".");
            charges.push_back(x.get<int>());
        }
    }
    std::vector<std::string> out;
    for (const auto& item : database.mapSubstances())
    {
        const auto& substance = item.second;
        if (!symbolAllowed(rule, item.first))
            continue;
        if (!classes.empty() && !classes.count(substanceClassFromLegacy(substance.substanceClass())))
            continue;
        if (!states.empty() && !states.count(aggregateStateFromLegacy(substance.aggregateState())))
            continue;
        if (!charges.empty() && std::find(charges.begin(), charges.end(), substance.charge()) == charges.end())
            continue;
        if (byElements)
        {
            std::set<std::string> elements;
            if (!elementsOf(database, substance, elements) || !elementsAllowed(rule, elements))
                continue;
        }
        out.push_back(item.first);
    }
    return chosen(std::move(out), "substance", rule);
}

auto selectReactions(const Database& database, const std::string& text) -> std::vector<std::string>
{
    const json rule = ruleOf(text);
    checkKeys(rule, reactionKeys);
    const auto involving = wordsOf(rule, "involving");
    const bool byElements = rule.contains("elements") || rule.contains("only_elements");
    std::vector<std::string> out;
    for (const auto& item : database.mapReactions())
    {
        if (!symbolAllowed(rule, item.first))
            continue;
        const auto species = item.second.reactants();
        if (std::any_of(involving.begin(), involving.end(), [&](const std::string& s) { return !species.count(s); }))
            continue;
        if (byElements)
        {
            std::set<std::string> elements;
            const bool known = std::all_of(species.begin(), species.end(), [&](const auto& s) {
                return database.containsSubstance(s.first) && elementsOf(database, database.getSubstance(s.first), elements);
            });
            if (!known || !elementsAllowed(rule, elements))
                continue;
        }
        out.push_back(item.first);
    }
    return chosen(std::move(out), "reaction", rule);
}

auto detail::unknownKeyMessage(const std::string& where, const std::string& key, const std::vector<std::string>& known) -> std::string
{
    std::string message = where + " has no key " + key + ".";
    const auto near = std::min_element(known.begin(), known.end(), [&](const std::string& a, const std::string& b) { return editDistance(key, a) < editDistance(key, b); });
    if (near != known.end() && editDistance(key, *near) <= std::max<std::size_t>(2, key.size() / 3))
        message += " Did you mean " + *near + "?";
    message += " It has:";
    for (std::size_t i = 0; i < known.size(); ++i)
        message += (i ? ", " : " ") + known[i];
    return message + ".";
}

} // namespace ThermoFun
