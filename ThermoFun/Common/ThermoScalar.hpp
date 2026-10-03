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

#ifndef THERMOSCALAR_H
#define THERMOSCALAR_H


// C++ includes
#include <cmath>
#include <string>
#include <ostream>

// autodiff includes
#include <autodiff/forward/real.hpp>

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

/// A thermodynamic scalar built on top of autodiff::real.
/// A *thermodynamic property* is a quantity that depends on temperature and pressure.
/// autodiff::real propagates the derivative along a single seeded direction,
/// so a ThermoScalar holds two autodiff::real passes: one seeded along temperature
/// (see wrtT()) and one seeded along pressure (see wrtP()). Both passes carry the same
/// value. Any operation on a ThermoScalar is applied to both passes, so every function
/// of autodiff::real can be used, and derivatives with respect to further variables
/// can be obtained by seeding them in the passes (see autodiff::seed).
/// In addition, the error and the status of the property are propagated.
class ThermoScalar
{
public:
    /// The error of the value of the thermodynamic property
    double err;

    /// The status of the themrodyanic property
    StatusMessage sta;

    /// Construct a default ThermoScalar instance
    ThermoScalar()
    : ThermoScalar(0.0) {}

    /// Construct a custom ThermoScalar instance with given value only.
    /// @param val The value of the thermodynamic property
    explicit ThermoScalar(double val)
        : ThermoScalar(val, 0.0, 0.0, 0.0, {Status::notdefined, ""}) {}

    /// Construct a custom ThermoScalar instance with given value and derivatives.
    /// @param val The value of the thermodynamic property
    /// @param ddt The partial temperature derivative of the thermodynamic property
    /// @param ddp The partial pressure derivative of the thermodynamic property
    /// @param err The error of the value of the thermodynamic property
    ThermoScalar(double val, double ddt, double ddp, double err, const StatusMessage& sta)
    : err(fabs(err)), sta(sta)
    {
        m_t[0] = val; m_t[1] = ddt;
        m_p[0] = val; m_p[1] = ddp;
    }

    /// Construct a ThermoScalar instance from its autodiff::real passes and its error and status.
    /// @param wrtT The quantity evaluated with temperature seeded
    /// @param wrtP The quantity evaluated with pressure seeded
    ThermoScalar(const autodiff::real& wrtT, const autodiff::real& wrtP, double err, const StatusMessage& sta)
    : err(fabs(err)), sta(sta), m_t(wrtT), m_p(wrtP) {}

    /// The value of the thermodynamic property.
    auto val() const -> double { return m_t[0]; }

    /// The partial temperature derivative of the thermodynamic property.
    auto ddt() const -> double { return m_t[1]; }

    /// The partial pressure derivative of the thermodynamic property.
    auto ddp() const -> double { return m_p[1]; }

    /// Set the value of the thermodynamic property.
    auto setVal(double v) -> void { m_t[0] = v; m_p[0] = v; }

    /// Set the partial temperature derivative of the thermodynamic property.
    auto setDdt(double v) -> void { m_t[1] = v; }

    /// Set the partial pressure derivative of the thermodynamic property.
    auto setDdp(double v) -> void { m_p[1] = v; }

    /// The autodiff::real with the derivative taken along temperature.
    auto wrtT() const -> const autodiff::real& { return m_t; }
    auto wrtT() -> autodiff::real& { return m_t; }

    /// The autodiff::real with the derivative taken along pressure.
    auto wrtP() const -> const autodiff::real& { return m_p; }
    auto wrtP() -> autodiff::real& { return m_p; }

    /// Assign a scalar to this ThermoScalar instance.
    ThermoScalar& operator=(double other)
    {
        m_t[0] = m_p[0] = other;
        m_t[1] = m_p[1] = 0.0;
        err = 0.0;
        sta = {Status::assigned, std::string("")};
        return *this;
    }

    /// Assign-addition of a ThermoScalar instance
    ThermoScalar& operator+=(const ThermoScalar& other)
    {
        m_t += other.m_t;
        m_p += other.m_p;
        err  = std::sqrt(err*err + other.err*other.err);
        sta = statusOf(sta, other.sta);
        return *this;
    }

    /// Assign-subtraction of a ThermoScalar instance
    ThermoScalar& operator-=(const ThermoScalar& other)
    {
        m_t -= other.m_t;
        m_p -= other.m_p;
        err  = std::sqrt(err*err + other.err*other.err);
        sta = statusOf(sta, other.sta);
        return *this;
    }

    /// Assign-multiplication of a ThermoScalar instance
    ThermoScalar& operator*=(const ThermoScalar& other)
    {
        const double tmp_err = err / val();
        m_t *= other.m_t;
        m_p *= other.m_p;
        if (other.val() == 0)
            err = 0.0;
        else
            err  = val()*sqrt(tmp_err*tmp_err + other.err/other.val()*other.err/other.val());
        sta = statusOf(sta, other.sta);
        return *this;
    }

    /// Assign-division of a ThermoScalar instance
    ThermoScalar& operator/=(const ThermoScalar& other)
    {
        const double tmp_err = err / val();
        m_t /= other.m_t;
        m_p /= other.m_p;
        if (other.val() == 0)
            err = 0.0;
        else
            err  = val()*sqrt(tmp_err*tmp_err + other.err/other.val()*other.err/other.val());
        sta = statusOf(sta, other.sta);
        return *this;
    }

    /// Assign-addition of a scalar
    ThermoScalar& operator+=(double other)
    {
        m_t[0] += other;
        m_p[0] += other;
        return *this;
    }

    /// Assign-subtraction of a scalar
    ThermoScalar& operator-=(double other)
    {
        m_t[0] -= other;
        m_p[0] -= other;
        return *this;
    }

    /// Assign-multiplication by a scalar
    ThermoScalar& operator*=(double other)
    {
        const double tmp_err = err/val()*err/val();
        m_t *= other;
        m_p *= other;
        if (val() == 0)
            err = 0.0;
        else
            err = val()*std::sqrt(tmp_err);
        return *this;
    }

    /// Assign-division by a scalar
    ThermoScalar& operator/=(double other)
    {
        const double tmp_err = err/val()*err/val();
        *this *= 1.0/other;
        if (val() == 0)
            err = 0.0;
        else
            err = val()*std::sqrt(tmp_err);
        return *this;
    }

    /// Explicitly converts this ThermoScalar instance into a double.
    explicit operator double() const
    {
        return val();
    }

    /// The status resulting from combining two statuses
    static auto statusOf(const StatusMessage& l, const StatusMessage& r) -> StatusMessage
    {
        if (l.first == Status::notdefined || r.first == Status::notdefined)
            return {Status::notdefined, std::string("")};
        return {Status::calculated, std::string("")};
    }

private:
    /// The value and its derivative along temperature
    autodiff::real m_t;

    /// The value and its derivative along pressure
    autodiff::real m_p;
};

/// A type that describes temperature in units of K
class Temperature : public ThermoScalar
{
public:
    /// Construct a default Temperature instance
    Temperature() : Temperature(0.0) {}

    /// Construct a Temperature instance with given value
    Temperature(double val) : ThermoScalar(val, 1.0, 0.0, 0.0, {Status::assigned, std::string("")}) {}
};

/// A type that describes pressure in units of Pa
class Pressure : public ThermoScalar
{
public:
    /// Construct a default Pressure instance
    Pressure() : Pressure(0.0) {}

    /// Construct a Pressure instance with given value
    Pressure(double val) : ThermoScalar(val, 0.0, 1.0, 0.0, {Status::assigned, std::string("")}) {}
};


inline auto status(const ThermoScalar& l, const ThermoScalar& r) -> StatusMessage
{
    return ThermoScalar::statusOf(l.sta, r.sta);
}

inline auto status(const ThermoScalar& l) -> StatusMessage
{
    return ThermoScalar::statusOf(l.sta, l.sta);
}

/// Unary addition operator for a ThermoScalar instance
inline auto operator+(const ThermoScalar& l) -> ThermoScalar
{
    return {l.wrtT(), l.wrtP(), l.err, status(l)};
}

/// Add two ThermoScalar instances
inline auto operator+(const ThermoScalar& l, const ThermoScalar& r) -> ThermoScalar
{
    return {l.wrtT() + r.wrtT(), l.wrtP() + r.wrtP(), std::sqrt(l.err*l.err + r.err*r.err), status(l,r)};
}

inline auto operator+(double l, const ThermoScalar& r) -> ThermoScalar
{
    return {l + r.wrtT(), l + r.wrtP(), r.err, status(r)};
}

inline auto operator+(const ThermoScalar& l, double r) -> ThermoScalar
{
    return r + l;
}

/// Unary subtraction operator for a ThermoScalar instance
inline auto operator-(const ThermoScalar& l) -> ThermoScalar
{
    return {-l.wrtT(), -l.wrtP(), l.err, status(l)};
}

/// Subtract two ThermoScalar instances
inline auto operator-(const ThermoScalar& l, const ThermoScalar& r) -> ThermoScalar
{
    return {l.wrtT() - r.wrtT(), l.wrtP() - r.wrtP(), std::sqrt(l.err*l.err + r.err*r.err), status(l, r)};
}

/// Right-subtract a ThermoScalar instance by a scalar
inline auto operator-(const ThermoScalar& l, double r) -> ThermoScalar
{
    return {l.wrtT() - r, l.wrtP() - r, l.err, status(l)};
}

/// Left-subtract a ThermoScalar instance by a scalar
inline auto operator-(double l, const ThermoScalar& r) -> ThermoScalar
{
    return {l - r.wrtT(), l - r.wrtP(), r.err, status(r)};
}

/// Multiply two ThermoScalar instances
inline auto operator*(const ThermoScalar& l, const ThermoScalar& r) -> ThermoScalar
{
    double a = 0.0; double b = 0.0;
    if (l.val() != 0)
        a = l.err/l.val()*l.err/l.val();
    if (r.val() != 0)
        b = r.err/r.val()*r.err/r.val();
    return {l.wrtT() * r.wrtT(), l.wrtP() * r.wrtP(), (l.val() * r.val())*std::sqrt(a+b), status(l,r)};
}

/// Left-multiply a ThermoScalar instance by a scalar
inline auto operator*(double l, const ThermoScalar& r) -> ThermoScalar
{
    if (r.val() == 0)
        return {l * r.wrtT(), l * r.wrtP(), 0.0, status(r)};
    auto err = (l * r.val())*std::sqrt(r.err/r.val()*r.err/r.val());
    return {l * r.wrtT(), l * r.wrtP(), err, status(r)};
}

/// Right-multiply a ThermoScalar instance by a scalar
inline auto operator*(const ThermoScalar& l, double r) -> ThermoScalar
{
    return r * l;
}

/// Divide a ThermoScalar instance by another
inline auto operator/(const ThermoScalar& l, const ThermoScalar& r) -> ThermoScalar
{
    const double tmp1 = 1.0/r.val();
    double a = 0.0; double b = 0.0;
    if (l.val() != 0)
        a = l.err/l.val()*l.err/l.val();
    if (r.val() != 0)
        b = r.err/r.val()*r.err/r.val();
    return {l.wrtT() / r.wrtT(), l.wrtP() / r.wrtP(), (tmp1 * l.val())*std::sqrt(a+b), status(l,r)};
}

/// Left-divide a ThermoScalar instance by a scalar
inline auto operator/(double l, const ThermoScalar& r) -> ThermoScalar
{
    const double tmp1 = 1.0/r.val();
    if (r.val() == 0)
        return {l / r.wrtT(), l / r.wrtP(), 0.0, status(r)};
    return {l / r.wrtT(), l / r.wrtP(), (tmp1 * r.val())*std::sqrt(r.err/r.val()*r.err/r.val()), status(r)};
}

/// Right-divide a ThermoScalar instance by a scalar
inline auto operator/(const ThermoScalar& l, double r) -> ThermoScalar
{
    return (1.0/r) * l;
}

/// Return the square root of a ThermoScalar instance
inline auto sqrt(const ThermoScalar& l) -> ThermoScalar
{
    if (l.val() == 0)
        return {std::sqrt(l.val()), 0.0, 0.0, 0.0, status(l)};
    return {sqrt(l.wrtT()), sqrt(l.wrtP()), 0.5*(l.err/l.val()), status(l)};
}

/// Return the power of a ThermoScalar instance
inline auto pow(const ThermoScalar& l, double power) -> ThermoScalar
{
    if (l.val() == 0)
        return {std::pow(l.val(), power), 0.0, 0.0, 0.0, status(l)};
    return {pow(l.wrtT(), power), pow(l.wrtP(), power), std::fabs(power)*(l.err/l.val()), status(l)};
}

/// Return the power of a ThermoScalar instance
inline auto pow(const ThermoScalar& l, const ThermoScalar& power) -> ThermoScalar
{
    const double powl = std::pow(l.val(), power.val());
    if (l.val() == 0)
    {
        // derivatives are not defined at a zero base; evaluating log(0) would give 0*(-inf) = NaN
        return {powl, 0.0, 0.0, 0.0, status(l,power)};
    }
    return {pow(l.wrtT(), power.wrtT()), pow(l.wrtP(), power.wrtP()), powl*(l.err/l.val()), status(l,power)};
}

/// Return the natural exponential of a ThermoScalar instance
inline auto exp(const ThermoScalar& l) -> ThermoScalar
{
    return {exp(l.wrtT()), exp(l.wrtP()), l.err*std::exp(l.val()), status(l)};
}

/// Return the natural log of a ThermoScalar instance
inline auto log(const ThermoScalar& l) -> ThermoScalar
{
    if (l.val() == 0)
        return {std::log(l.val()), 0.0, 0.0, 0.0, status(l)};
    return {log(l.wrtT()), log(l.wrtP()), 0.434*(l.err/l.val()), status(l)};
}

/// Return the log10 of a ThermoScalar instance
inline auto log10(const ThermoScalar& l) -> ThermoScalar
{
    const double ln10 = 2.302585092994046;
    return log(l)/ln10;
}

/// Return true if a ThermoScalar instance is less than another
inline auto operator<(const ThermoScalar& l, const ThermoScalar& r) -> bool
{
    return l.val() < r.val();
}

/// Return true if a ThermoScalar instance is less or equal than another
inline auto operator<=(const ThermoScalar& l, const ThermoScalar& r) -> bool
{
    return l.val() <= r.val();
}

/// Return true if a ThermoScalar instance is greater than another
inline auto operator>(const ThermoScalar& l, const ThermoScalar& r) -> bool
{
    return l.val() > r.val();
}

/// Return true if a ThermoScalar instance is greater or equal than another
inline auto operator>=(const ThermoScalar& l, const ThermoScalar& r) -> bool
{
    return l.val() >= r.val();
}

/// Return true if a ThermoScalar instance is equal to another
inline auto operator==(const ThermoScalar& l, const ThermoScalar& r) -> bool
{
    return l.val() == r.val();
}

/// Return true if a ThermoScalar instance is not equal to another
inline auto operator!=(const ThermoScalar& l, const ThermoScalar& r) -> bool
{
    return l.val() != r.val();
}

/// Return true if a scalar is less than a ThermoScalar instance
inline auto operator<(double l, const ThermoScalar& r) -> bool
{
    return l < r.val();
}

/// Return true if a ThermoScalar instance is less than a scalar
inline auto operator<(const ThermoScalar& l, double r) -> bool
{
    return l.val() < r;
}

/// Return true if a scalar is less or equal than a ThermoScalar instance
inline auto operator<=(double l, const ThermoScalar& r) -> bool
{
    return l <= r.val();
}

/// Return true if a ThermoScalar instance is less or equal than a scalar
inline auto operator<=(const ThermoScalar& l, double r) -> bool
{
    return l.val() <= r;
}

/// Return true if a scalar is greater than a ThermoScalar instance
inline auto operator>(double l, const ThermoScalar& r) -> bool
{
    return l > r.val();
}

/// Return true if a ThermoScalar is greater than a scalar
inline auto operator>(const ThermoScalar& l, double r) -> bool
{
    return l.val() > r;
}

/// Return true if a scalar is greater or equal than a ThermoScalar instance
inline auto operator>=(double l, const ThermoScalar& r) -> bool
{
    return l >= r.val();
}

/// Return true if a ThermoScalar instance is greater or equal than a scalar
inline auto operator>=(const ThermoScalar& l, double r) -> bool
{
    return l.val() >= r;
}

/// Return true if a scalar is equal to a ThermoScalar instance
inline auto operator==(double l, const ThermoScalar& r) -> bool
{
    return l == r.val();
}

/// Return true if a ThermoScalar instance is equal to a scalar
inline auto operator==(const ThermoScalar& l, double r) -> bool
{
    return l.val() == r;
}

/// Return true if a scalar is not equal to a ThermoScalar instance
inline auto operator!=(double l, const ThermoScalar& r) -> bool
{
    return l != r.val();
}

/// Return true if a ThermoScalar instance is not equal to a scalar
inline auto operator!=(const ThermoScalar& l, double r) -> bool
{
    return l.val() != r;
}

/// Output a ThermoScalar instance
inline auto operator<<(std::ostream& out, const ThermoScalar& scalar) -> std::ostream&
{
    out << scalar.val();
    return out;
}

} // namespace Reaktoro

#endif // THERMOSCALAR_H
