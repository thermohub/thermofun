// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_COVARIANCE_H
#define THERMOFUN_MODEL_COVARIANCE_H

#include <string>
#include <vector>

namespace ThermoFun::v2 {

/// The covariance of some values of a record: standard-state properties and method parameters.
struct CovarianceBlock
{
    /// The values, as paths in the record, for example "/standard_state_properties/G".
    std::vector<std::string> paths;

    /// The symmetric matrix, row after row, size paths.size() squared, in the SI units of the values.
    std::vector<double> matrix;

    auto operator==(const CovarianceBlock&) const -> bool = default;
};

} // namespace ThermoFun::v2

#endif
