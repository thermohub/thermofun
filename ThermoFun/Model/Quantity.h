// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_QUANTITY_H
#define THERMOFUN_MODEL_QUANTITY_H

#include <optional>
#include <string>
#include <vector>

#include "Vocabulary.h"

namespace ThermoFun::v2 {

/// Uncertainty of a value, as a standard uncertainty (k = 1) in the SI unit of the value.
struct Uncertainty
{
    enum class Kind
    {
        Primary,
        Derived
    };

    /// The standard uncertainty u (k = 1), in the SI unit of the value it belongs to.
    double standard = 0.0;

    /// The coverage factor in the file (u = U / k at reading). 1 if the file gives none.
    double coverage_factor_as_written = 1.0;

    /// Primary: from a measurement or an evaluation, stored. Derived: computed by propagation.
    Kind kind = Kind::Primary;

    /// Name of the distribution ("normal", "uniform", ...). Empty if the file gives none.
    std::string distribution;

    /// Degrees of freedom. Empty if the file gives none.
    std::optional<double> degrees_of_freedom;

    /// For kind Derived: the records the value comes from, and the snapshot of the calculation.
    std::vector<std::string> sources;
    std::string snapshot;

    /// True if a legacy file does not say if the value is u, U or a confidence interval.
    bool convention_unknown = false;
};

/// Optional data about one value. An empty value or string means that the file gives nothing here.
struct ValueMeta
{
    std::optional<Quality> quality;
    std::optional<Origin> origin;
    std::string calculation_path;  ///< free text if origin is Calculated
    std::string source;            ///< symbol of a data source, for example "1995ROB/HEM"
};

/// A number with its uncertainty and its data. The value is in SI units.
struct Quantity
{
    double value = 0.0;                      ///< in the SI unit of the slot it fills
    std::string unit_as_written;             ///< unit in the file. Empty if the default unit was used.
    std::optional<Uncertainty> uncertainty;  ///< empty if the file gives none (not zero)
    ValueMeta meta;

    /// True if the value came from a single-precision (float32) source, as the legacy GEMS exports. A writer then
    /// writes the shortest decimal that gives the same float32, for example 162.3 and not 162.3000030517578.
    bool single_precision_source = false;

    /// True if the quantity has no data, no uncertainty and the default unit. v2 files write it as a bare number.
    auto isBare() const -> bool;
};

/// Numbers that share one unit, for example a legacy array of coefficients.
struct QuantityList
{
    std::vector<double> values;                                 ///< SI
    std::vector<std::string> units_as_written;                  ///< one for each value, or empty
    std::vector<std::optional<double>> standard_uncertainties;  ///< one for each value, or empty
};

} // namespace ThermoFun::v2

#endif
