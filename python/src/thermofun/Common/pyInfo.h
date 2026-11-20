// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The info of a value or of a result as Python sees it (decision 0029): the stored part (notes, source, pressure) and what is
// made from it, the status code and the error (validity, status, error).

#ifndef THERMOFUN_PYTHON_PYINFO_H
#define THERMOFUN_PYTHON_PYINFO_H

#include <cmath>
#include <optional>
#include <string>

#include <ThermoFun/Common/OutsideRange.h>
#include <ThermoFun/Common/ThermoScalar.hpp>

namespace ThermoFun {

struct InfoView
{
    Validity validity = Validity::within;
    std::optional<Reaktoro_::Status> status;  ///< of a value; none for a whole result
    ErrorState error = ErrorState::unknown;
    std::string source;
    std::optional<double> pressure;           ///< Pa; none if not known
    Notes notes;
};

/// The info of one value.
inline auto infoOf(const Reaktoro_::ThermoScalar& x) -> InfoView
{
    InfoView v;
    v.validity = validity(x);
    v.status = x.sta.first;
    v.error = errorState(x);
    v.source = x.info.source;
    if (!std::isnan(x.info.pressure))
        v.pressure = x.info.pressure;
    v.notes = x.info.notes;
    return v;
}

/// The info of a result: the notes of all its values; extrapolated if one of them is.
inline auto infoOf(const Reaktoro_::Info& info) -> InfoView
{
    InfoView v;
    for (const auto& note : info.notes)
    {
        if (note.kind == Note::Kind::Range)
            v.validity = Validity::extrapolated;
        if (note.kind == Note::Kind::Uncertainty)
            v.error = ErrorState::partial;
    }
    v.source = info.source;
    if (!std::isnan(info.pressure))
        v.pressure = info.pressure;
    v.notes = info.notes;
    return v;
}

} // namespace ThermoFun

#endif
