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
#include "Common/ThermoScalar.hpp"

namespace Reaktoro_ {

/// The status of a result calculated from other results: not defined if any of them is not defined
inline auto combineStatus(const StatusMessage& l, const StatusMessage& r) -> StatusMessage
{
    if (l.first == Status::notdefined || r.first == Status::notdefined)
        return {Status::notdefined, std::string("")};
    return {Status::calculated, std::string("")};
}

/// A thermodynamic property as returned by the engine: the value `val`, its partial derivatives with respect to
/// temperature `ddt` and pressure `ddp` (calculated with autodiff in the models), its error `err` and its status `sta`.
/// This is the type ThermoScalar of the interface of ThermoFun (Common/ThermoScalar.hpp): code using the properties
/// of ThermoFun, e.g. `tps.gibbs_energy.val`, `.ddt`, `.ddp`, `.err`, `.sta` or its arithmetic, is not affected.
using ThermoProperty = ThermoScalar;

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
        real r(x.val());
        r[1] = (wrt == Wrt::T) ? x.ddt() : (wrt == Wrt::P) ? x.ddp() : 0.0;
        return r;
    }

    /// A constant as an autodiff number (zero derivative)
    auto operator()(double x) const -> real { return real(x); }
};

/// A value of the pass with the derivatives dT and dP of that value with respect to T and P (the derivative of the pass is the
/// one with respect to the variable of the pass)
inline auto along(const Pass& pass, double v, double dT, double dP) -> real
{
    real r(v);
    r[1] = (pass.wrt == Wrt::T) ? dT : (pass.wrt == Wrt::P) ? dP : 0.0;
    return r;
}

/// A property that is known to be constant (e.g. a reference value of the database) as an autodiff number
inline auto constant(const ThermoProperty& x) -> real { return real(x.val()); }

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
