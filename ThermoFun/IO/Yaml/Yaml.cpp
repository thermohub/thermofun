// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <ThermoFun/IO/detail/Yaml.h>

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string_view>

#define RYML_SINGLE_HDR_DEFINE_NOW
#include <rapidyaml.hpp>

namespace ThermoFun::v2::detail {

namespace {

using json = nlohmann::ordered_json;

/// What the handlers of rapidyaml raise: they must not return.
struct ParseFailure : std::runtime_error
{
    using std::runtime_error::runtime_error;
    int line = 0, column = 0;
};

[[noreturn]] void raise(ryml::csubstr message, const ryml::Location& where)
{
    ParseFailure failure(std::string(message.str, message.len));
    if (where.line != ryml::npos)
        failure.line = static_cast<int>(where.line) + 1;
    if (where.col != ryml::npos)
        failure.column = static_cast<int>(where.col) + 1;
    throw failure;
}

void onBasic(ryml::csubstr message, const ryml::ErrorDataBasic& data, void*) { raise(message, data.location); }
void onParse(ryml::csubstr message, const ryml::ErrorDataParse& data, void*) { raise(message, data.ymlloc); }
void onVisit(ryml::csubstr message, const ryml::ErrorDataVisit& data, void*) { raise(message, data.cpploc); }

auto callbacks() -> ryml::Callbacks
{
    ryml::Callbacks c;
    c.set_error_basic(onBasic);
    c.set_error_parse(onParse);
    c.set_error_visit(onVisit);
    return c;
}

/// A parser and its tree.
struct Parsed
{
    ryml::EventHandlerTree handler;
    ryml::Parser parser;
    ryml::Tree tree;

    Parsed(const std::string& text, bool keepLocations)
        : handler(callbacks()), parser(&handler, ryml::ParserOptions().locations(keepLocations)), tree(callbacks())
    {
        const ryml::csubstr view(text.data(), text.size());
        if (looksLikeJson(text))
            ryml::parse_json_in_arena(&parser, view, &tree);
        else
            ryml::parse_in_arena(&parser, view, &tree);
    }

    /// The first document
    auto root() -> ryml::ConstNodeRef
    {
        ryml::ConstNodeRef node = tree.crootref();
        if (node.is_stream())
        {
            if (node.num_children() != 1)
                throw std::runtime_error("The text has " + std::to_string(node.num_children()) + " documents: it must have one.");
            return node.first_child();
        }
        return node;
    }
};

auto textOf(ryml::csubstr s) -> std::string
{
    return s.str ? std::string(s.str, s.len) : std::string();
}

// A plain number of YAML 1.2, whole text: a sign, digits, a point, an exponent
auto plainNumber(std::string_view s, json& out) -> bool
{
    if (s.empty())
        return false;
    size_t i = s[0] == '+' || s[0] == '-' ? 1 : 0;
    bool digits = false, decimal = false;
    for (; i < s.size() && ((s[i] >= '0' && s[i] <= '9') || s[i] == '.'); ++i)
    {
        digits = digits || s[i] != '.';
        if (s[i] == '.')
        {
            if (decimal)
                return false;
            decimal = true;
        }
    }
    if (!digits)
        return false;
    bool exponent = false;
    if (i < s.size() && (s[i] == 'e' || s[i] == 'E'))
    {
        exponent = true;
        ++i;
        if (i < s.size() && (s[i] == '+' || s[i] == '-'))
            ++i;
        const size_t first = i;
        while (i < s.size() && s[i] >= '0' && s[i] <= '9')
            ++i;
        if (i == first)
            return false;
    }
    if (i != s.size())
        return false;
    const std::string text(s.substr(s[0] == '+' ? 1 : 0));
    if (!decimal && !exponent)
    {
        long long whole = 0;
        const auto end = text.data() + text.size();
        if (auto r = std::from_chars(text.data(), end, whole); r.ec == std::errc() && r.ptr == end)
        {
            out = whole;
            return true;
        }
    }
    const double value = std::strtod(text.c_str(), nullptr);
    if (!std::isfinite(value))
        return false;
    out = value;
    return true;
}

auto scalar(ryml::ConstNodeRef node) -> json
{
    const std::string text = textOf(node.val());
    if (node.is_val_quoted())
        return text;
    if (node.val_is_null() || text.empty() || text == "~" || text == "null" || text == "Null" || text == "NULL")
        return nullptr;
    if (text == "true" || text == "True" || text == "TRUE")
        return true;
    if (text == "false" || text == "False" || text == "FALSE")
        return false;
    json number;
    if (plainNumber(text, number))
        return number;
    return text;
}

auto toJson(ryml::ConstNodeRef node, const std::string& path, std::vector<std::string>& twice) -> json
{
    if (node.is_map())
    {
        json out = json::object();
        for (const ryml::ConstNodeRef child : node.children())
        {
            const std::string key = textOf(child.key());
            const std::string at = path + "/" + key;
            if (out.contains(key))
                twice.push_back(at);
            out[key] = toJson(child, at, twice);
        }
        return out;
    }
    if (node.is_seq())
    {
        json out = json::array();
        int index = 0;
        for (const ryml::ConstNodeRef child : node.children())
            out.push_back(toJson(child, path + "/" + std::to_string(index++), twice));
        return out;
    }
    return node.has_val() ? scalar(node) : json();
}

} // namespace

auto looksLikeJson(const std::string& text) -> bool
{
    const auto first = text.find_first_not_of(" \t\r\n");
    return first != std::string::npos && (text[first] == '{' || text[first] == '[');
}

auto readYaml(const std::string& text) -> YamlResult
{
    YamlResult out;
    try
    {
        Parsed parsed(text, false);
        parsed.tree.resolve();
        out.tree = toJson(parsed.root(), "", out.paths_written_twice);
        out.ok = true;
    }
    catch (const ParseFailure& failure)
    {
        out.problem = {failure.what(), failure.line, failure.column};
    }
    catch (const std::exception& failure)
    {
        out.problem.message = failure.what();
    }
    return out;
}

struct TextLocations::Impl
{
    std::string text;
    std::unique_ptr<Parsed> parsed;
};

TextLocations::TextLocations(const std::string& text) : pimpl(new Impl{text, nullptr})
{
    try
    {
        pimpl->parsed = std::make_unique<Parsed>(pimpl->text, true);
    }
    catch (const std::exception&)
    {
        pimpl->parsed.reset();
    }
}

TextLocations::~TextLocations()
{
    delete pimpl;
}

auto TextLocations::find(const std::string& path) const -> std::pair<int, int>
{
    if (!pimpl->parsed)
        return {0, 0};
    try
    {
        ryml::ConstNodeRef node = pimpl->parsed->root();
        size_t at = path.empty() || path[0] != '/' ? 0 : 1;
        while (at < path.size())
        {
            // the next part of the path; a key of a map may hold a "/", so a longer part is tried if the short one is not there
            bool found = false;
            for (size_t end = path.find('/', at);; end = path.find('/', end + 1))
            {
                const std::string part = path.substr(at, end == std::string::npos ? std::string::npos : end - at);
                if (node.is_map())
                {
                    const ryml::ConstNodeRef child = node.find_child(ryml::csubstr(part.data(), part.size()));
                    if (!child.invalid())
                    {
                        node = child;
                        found = true;
                    }
                }
                else if (node.is_seq() && !part.empty() && part.find_first_not_of("0123456789") == std::string::npos)
                {
                    const size_t index = std::stoul(part);
                    if (index < node.num_children())
                    {
                        node = node.child(index);
                        found = true;
                    }
                }
                if (found)
                {
                    at = end == std::string::npos ? path.size() : end + 1;
                    break;
                }
                if (end == std::string::npos)
                    break;
            }
            if (!found)
                break;
        }
        const ryml::Location where = node.location(pimpl->parsed->parser);
        if (where.line == ryml::npos)
            return {0, 0};
        return {static_cast<int>(where.line) + 1, static_cast<int>(where.col) + 1};
    }
    catch (const std::exception&)
    {
        return {0, 0};
    }
}

} // namespace ThermoFun::v2::detail
