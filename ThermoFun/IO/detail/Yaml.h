// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_IO_DETAIL_YAML_H
#define THERMOFUN_IO_DETAIL_YAML_H

#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

// YAML through rapidyaml. Only this file and IO/Yaml/Yaml.cpp know the parser.

namespace ThermoFun::v2::detail {

/// A problem in the text. Line and column start at 1; 0 if not known.
struct TextProblem
{
    std::string message;
    int line = 0;
    int column = 0;
};

/// The tree of a YAML text, with the types of YAML 1.2: a plain number is a number, true and false are booleans, null or nothing is
/// null, anything else or anything in quotes is a string. Anchors and merge keys are resolved. The order of a map is kept.
/// paths_written_twice gets the path of each key that a map has more than once; the last value counts. If the text cannot be read,
/// ok is false and problem says why.
struct YamlResult
{
    nlohmann::ordered_json tree;
    std::vector<std::string> paths_written_twice;
    bool ok = false;
    TextProblem problem;
};
auto readYaml(const std::string& text) -> YamlResult;

/// The line and the column (from 1) of the value at a path such as "/substances/A/methods/0", for a JSON or a YAML text: of the deepest
/// node of the path that exists. A symbol may hold a "/". {0, 0} if the text cannot be read.
class TextLocations
{
public:
    explicit TextLocations(const std::string& text);
    ~TextLocations();
    TextLocations(const TextLocations&) = delete;
    auto operator=(const TextLocations&) -> TextLocations& = delete;
    auto find(const std::string& path) const -> std::pair<int, int>;

private:
    struct Impl;
    Impl* pimpl;
};

/// True if the text is JSON, not YAML: the first character is a brace or a bracket. The JSON parser reads it faster.
auto looksLikeJson(const std::string& text) -> bool;

} // namespace ThermoFun::v2::detail

#endif
