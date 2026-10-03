// Reaktoro is a unified framework for modeling chemically reactive systems.
//
// Copyright (C) 2014-2015 Allan Leal
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef THERMOPROPERTY_H
#define THERMOPROPERTY_H

// C++ includes
#include <cmath>
#include <string>
#include <utility>
#include <ostream>

#include "Common/Real.hpp"

namespace Reaktoro_ {

enum Status {
    notdefined = 0,
    read,
    calculated,
    assigned,
    initialized,
    outofbounds
};

using StatusMessage = std::pair <Status, std::string>;

/// The status of a result calculated from other results: not defined if any of them is not defined
inline auto combineStatus(const StatusMessage& l, const StatusMessage& r) -> StatusMessage
{
    if (l.first == Status::notdefined || r.first == Status::notdefined)
        return {Status::notdefined, std::string("")};
    return {Status::calculated, std::string("")};
}

/// The error of a result calculated from other results (quadrature sum of the errors)
inline auto combineError(double l, double r) -> double
{
    return std::sqrt(l*l + r*r);
}

/// A thermodynamic property as returned by the engine.
/// It holds the value, its partial derivatives with respect to temperature and pressure (calculated with autodiff
/// in the models), and its error and status. It has no arithmetic: the calculations are done with autodiff::real
/// in the models, the error and the status are propagated explicitly (see combineStatus and combineError).
struct ThermoProperty
{
    /// The value of the thermodynamic property
    double val = 0.0;

    /// The partial temperature derivative of the thermodynamic property
    double ddt = 0.0;

    /// The partial pressure derivative of the thermodynamic property
    double ddp = 0.0;

    /// The error of the value of the thermodynamic property
    double err = 0.0;

    /// The status of the thermodynamic property
    StatusMessage sta = {Status::notdefined, ""};

    ThermoProperty() = default;

    /// Construct a property with given value only (its status is not defined)
    explicit ThermoProperty(double val) : val(val) {}

    ThermoProperty(double val, double ddt, double ddp, double err, const StatusMessage& sta)
    : val(val), ddt(ddt), ddp(ddp), err(std::fabs(err)), sta(sta) {}

    /// Assign a value (a constant: derivatives and error are zero)
    ThermoProperty& operator=(double other)
    {
        val = other; ddt = 0.0; ddp = 0.0; err = 0.0;
        sta = {Status::assigned, std::string("")};
        return *this;
    }

    /// Set the error and the status from the properties it was calculated from
    template<typename... Props>
    auto propagateFrom(const Props&... props) -> ThermoProperty&
    {
        StatusMessage status = {Status::calculated, std::string("")};
        double error = 0.0;
        ((status = combineStatus(status, props.sta), error = combineError(error, props.err)), ...);
        sta = status;
        err = error;
        return *this;
    }

    /// Set the error and the status as those of a property calculated from constants
    auto asCalculated() -> ThermoProperty&
    {
        sta = {Status::calculated, std::string("")};
        err = 0.0;
        return *this;
    }
};

/// Output the value of a property
inline auto operator<<(std::ostream& out, const ThermoProperty& property) -> std::ostream&
{
    out << property.val;
    return out;
}

/// The variable the derivatives are taken with respect to in one autodiff pass (None: all derivatives are zero)
enum class Wrt { T, P, None };

/// The temperature and pressure of one autodiff pass. The derivatives of every number calculated in the pass
/// are taken with respect to the variable that is seeded.
struct Pass
{
    Wrt wrt;
    real T;
    real P;

    Pass(double T0, double P0, Wrt wrt) : wrt(wrt), T(T0), P(P0)
    {
        if (wrt == Wrt::T) T[1] = 1.0; else if (wrt == Wrt::P) P[1] = 1.0;
    }

    /// A property, with the derivative of this pass, as an autodiff number
    auto operator()(const ThermoProperty& x) const -> real
    {
        real r = x.val;
        r[1] = (wrt == Wrt::T) ? x.ddt : (wrt == Wrt::P) ? x.ddp : 0.0;
        return r;
    }

    /// A constant as an autodiff number (zero derivative)
    auto operator()(double x) const -> real { return real(x); }
};

/// A property that is known to be constant (e.g. a reference value of the database) as an autodiff number
inline auto constant(const ThermoProperty& x) -> real { return real(x.val); }

/// Combine the autodiff numbers of the pass seeded with temperature and the pass seeded with pressure in a property
inline auto toProperty(const real& wrtT, const real& wrtP) -> ThermoProperty
{
    ThermoProperty r;
    r.val = wrtT[0];
    r.ddt = wrtT[1];
    r.ddp = wrtP[1];
    r.sta = {Status::calculated, std::string("")};
    return r;
}

} // namespace Reaktoro_

#endif // THERMOPROPERTY_H
