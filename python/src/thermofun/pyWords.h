// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// A setting of the Python API given as a word of the format or as the enum (decision 0031: the format's words).

#pragma once

#include <optional>
#include <string>

#include <pybind11/pybind11.h>

#include <ThermoFun/Model/Vocabulary.h>

namespace ThermoFun {

namespace py = pybind11;

// The value of a checked word: an unknown word gives the words allowed and a hint
template <class E>
auto wordOf(const std::string& word) -> E
{
    const auto value = model::fromString<E>(word);
    if (!value)
        throw py::value_error(model::unknownWordMessage<E>(word));
    return *value;
}

// A setting given as the enum or as a word
template <class E>
auto valueOf(const py::object& given) -> E
{
    if (py::isinstance<py::str>(given))
        return wordOf<E>(given.cast<std::string>());
    return given.cast<E>();
}

// The same for a field that may be empty: None empties it
template <class E>
auto optionalValueOf(const py::object& given) -> std::optional<E>
{
    if (given.is_none())
        return std::nullopt;
    return valueOf<E>(given);
}

} // namespace ThermoFun
