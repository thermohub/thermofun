// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_METHODAPPLICATION_H
#define THERMOFUN_MODEL_METHODAPPLICATION_H

#include <map>
#include <optional>
#include <string>
#include <vector>

#include <ThermoFun/Model/Applicability.h>
#include <ThermoFun/Model/Quantity.h>
#include <ThermoFun/Model/Vocabulary.h>

namespace ThermoFun::v2 {

/// A table of property values on a grid of T and P, in SI units: values[name][i + j * temperatures.size()].
struct TableData
{
    std::vector<double> temperatures;                   ///< K, ascending
    std::vector<double> pressures;                      ///< Pa, ascending (one value for a table in T only)
    std::map<std::string, std::vector<double>> values;  ///< "G", "H", "V", "Cp" (substances) or "logK" (reactions)
    std::optional<double> reference_pressure;           ///< Pa

    auto operator==(const TableData&) const -> bool = default;
};

/// The use of one method on one record: which method, its parameter values and its range.
struct MethodApplication
{
    std::string method;                          ///< id of the method after the alias is resolved, for example "HKF88"
    std::string method_as_written;               ///< id or legacy name as in the file, to write it back
    std::string implementation;                  ///< empty: the default implementation of the method; else "gems", "reaktoro", "symbolic"
    std::map<std::string, Quantity> parameters;  ///< named parameters, SI values
    std::string parameter_set;                   ///< name of a parameter set, empty if none
    Applicability applicability;
    std::vector<Segment> segments;               ///< pieces of a method that is piecewise in T; empty for other methods
    /// Phase transitions in the range, each {T, dS, dH, dV, dTdP} (the rows of legacy m_phase_trans_props)
    std::vector<std::map<std::string, Quantity>> transitions;
    std::optional<TableData> table;              ///< tabulated methods
    std::optional<Interpolation> interpolation;  ///< tabulated methods; empty: the default of the method
    int interpolation_order = 3;                 ///< Lagrange only
    std::string extra_json;                      ///< fields of this entry that the format does not define, as JSON text

    auto operator==(const MethodApplication&) const -> bool = default;
};

} // namespace ThermoFun::v2

#endif
