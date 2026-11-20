// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The readable form of a YAML text.

#include <ThermoFun/IO/V2/detail/V2WriterParts.h>

namespace ThermoFun::model::v2writer {

// A word that YAML reads as the same text without quotes
auto plainWord(const std::string& s) -> bool
{
    static const std::regex safe("^[A-Za-z0-9_(][A-Za-z0-9_+\\-./()@|^*]*( [A-Za-z0-9_+\\-./()@|^*]+)*$");
    if (!std::regex_match(s, safe))
        return false;
    static const std::regex number("^[0-9.]+([eE][+-]?[0-9]+)?$");
    if (std::regex_match(s, number))
        return false;
    static const std::set<std::string> words = {"true", "True", "TRUE", "false", "False", "FALSE", "null", "Null", "NULL", "yes", "no", "on", "off", "Yes", "No", "On", "Off"};
    return !words.count(s);
}

auto text(const std::string& s) -> std::string
{
    return plainWord(s) ? s : json(s).dump();
}

auto isBlock(const json& node) -> bool
{
    return (node.is_object() || node.is_array()) && !node.empty();
}

// The node on one line
auto flow(const json& node) -> std::string
{
    if (node.is_string())
        return text(node.get<std::string>());
    if (node.is_object())
    {
        std::string out = "{";
        bool first = true;
        for (const auto& item : node.items())
        {
            out += (first ? "" : ", ") + text(item.key()) + ": " + flow(item.value());
            first = false;
        }
        return out + "}";
    }
    if (node.is_array())
    {
        std::string out = "[";
        bool first = true;
        for (const auto& item : node)
        {
            out += (first ? "" : ", ") + flow(item);
            first = false;
        }
        return out + "]";
    }
    return node.dump();
}

auto onlyNumbers(const json& node) -> bool
{
    return node.is_array() && std::all_of(node.begin(), node.end(), [](const json& x) { return x.is_number(); });
}

// a mapping or list goes on one line if it is short, and a list of numbers always does
auto staysOnOneLine(const json& node, size_t room) -> bool
{
    return !isBlock(node) || onlyNumbers(node) || flow(node).size() <= room;
}

void block(const json& node, size_t indent, std::string& out)
{
    constexpr size_t width = 110;
    const std::string pad(indent, ' ');
    if (node.is_object())
        for (const auto& item : node.items())
        {
            const std::string head = pad + text(item.key()) + ":";
            const json& value = item.value();
            if (staysOnOneLine(value, width > head.size() + 1 ? width - head.size() - 1 : 0))
                out += head + " " + flow(value) + "\n";
            else
            {
                out += head + "\n";
                block(value, indent + 2, out);
            }
        }
    else
        for (const auto& value : node)
        {
            if (staysOnOneLine(value, width > indent + 2 ? width - indent - 2 : 0))
            {
                out += pad + "- " + flow(value) + "\n";
                continue;
            }
            // the first line of the block follows the dash
            std::string inner;
            block(value, indent + 2, inner);
            out += pad + "- " + inner.substr(indent + 2);
        }
}

} // namespace ThermoFun::model::v2writer
