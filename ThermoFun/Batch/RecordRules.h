// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_BATCH_RECORDRULES_H
#define THERMOFUN_BATCH_RECORDRULES_H

#include <stdexcept>
#include <string>
#include <vector>

namespace ThermoFun {

class Database;

/// A setup that cannot run. The message says why. Its type information is in the library only (the destructor is defined
/// there) and visible: on macOS a type that each binary makes for itself is not caught as the same type, so the Python
/// module got a RuntimeError instead of TableSetupError.
#if defined(_WIN32)
struct TableSetupError : std::runtime_error
#else
struct __attribute__((visibility("default"))) TableSetupError : std::runtime_error
#endif
{
    using std::runtime_error::runtime_error;
    ~TableSetupError() override;
};

/// The substances of a database that a rule chooses, in the order of their symbols. The rule is YAML or JSON text:
/// {class: aqueous_solute, elements: [Ca, C]}. Keys: class, aggregate_state, elements (it has all of them),
/// only_elements (it has no others), charge, match (a symbol, with * and ?), exclude (symbols, with * and ?).
/// A rule that chooses nothing is an error. See docs/how-to/make-tables.md.
auto selectSubstances(const Database& database, const std::string& rule) -> std::vector<std::string>;

/// The reactions of a database that a rule chooses. Keys: elements, only_elements (of all its species), involving (it has all these
/// species), match, exclude.
auto selectReactions(const Database& database, const std::string& rule) -> std::vector<std::string>;

namespace detail {
/// "<where> has no key <key>", with the nearest of the known keys.
auto unknownKeyMessage(const std::string& where, const std::string& key, const std::vector<std::string>& known) -> std::string;
} // namespace detail

} // namespace ThermoFun

#endif
