// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_ENGINE_UNCERTAINTY_H
#define THERMOFUN_ENGINE_UNCERTAINTY_H

#include <string>
#include <vector>

#include <ThermoFun/Engine/Sensitivity.h>
#include <ThermoFun/Model/ReactionRecord.h>
#include <ThermoFun/Model/SubstanceRecord.h>

namespace ThermoFun::v2 {

/// One value of a record that can be a parameter: its path, as a covariance block names it, and how a calculation names it.
struct RecordValue
{
    std::string path;
    ParameterRef ref;
    double standard = 0.0;
};

/// All the values of a record: reference values, parameters of the methods, of their segments, and the changes at their transitions.
auto recordValues(const SubstanceRecord& record) -> std::vector<RecordValue>;
auto recordValues(const ReactionRecord& record) -> std::vector<RecordValue>;

/// The value of a record that a calculation names so. Null if the record has no such value.
auto findValue(SubstanceRecord& record, const ParameterRef& ref) -> Quantity*;
auto findValue(ReactionRecord& record, const ParameterRef& ref) -> Quantity*;

/// The uncertain values of some records, with their covariance. Inputs without an uncertainty are not in the list.
struct UncertainInputs
{
    /// One value, with its standard uncertainty in SI.
    struct Input
    {
        ParameterRef ref;
        double standard = 0.0;
        bool in_block = false;  ///< its variance is in a covariance block
    };

    /// A covariance block: places in inputs, and the matrix, row after row.
    struct Block
    {
        std::vector<std::size_t> inputs;
        std::vector<double> matrix;
    };

    std::vector<Input> inputs;
    std::vector<Block> blocks;

    auto add(const SubstanceRecord& record) -> void;
    auto add(const ReactionRecord& record) -> void;

    /// u^2 = J S J^T. J: the derivatives of one result by each input.
    auto variance(const std::vector<double>& J) const -> double;
};

} // namespace ThermoFun::v2

#endif
