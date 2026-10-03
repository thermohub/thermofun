#pragma once

#include <cmath>
#include <cstddef>

#ifndef THERMOFUN_NO_AUTODIFF

// autodiff includes
#include <autodiff/forward/real.hpp>

namespace ThermoFun {

/// The number type used in the models to calculate derivatives with autodiff
using real = autodiff::real;

/// The absolute value (autodiff names it abs)
inline auto fabs(const real& x) -> real { return abs(x); }
inline auto fabs(double x) -> double { return std::fabs(x); }

/// The power with an exponent that does not depend on the variable (e.g. an integer coefficient kept as an autodiff
/// number): the derivative with respect to the exponent is not calculated (autodiff would evaluate log(x), which is
/// not defined for x <= 0 and would make the derivatives not-a-number)
inline auto pow(const real& x, const real& y) -> real
{
    if (y[1] == 0.0)
        return pow(x, y[0]);
    return autodiff::detail::pow(x, y);
}

} // namespace ThermoFun

namespace Reaktoro_ {

using real = autodiff::real;

/// True if ThermoFun is built with autodiff (the derivatives are calculated), false if built with plain numbers
/// (-DTFUN_USE_AUTODIFF=OFF): then every derivative ddt, ddp is 0
inline constexpr bool kWithAutodiff = true;

} // namespace Reaktoro_

#else // THERMOFUN_NO_AUTODIFF: the models calculate with plain numbers, without derivatives

namespace ThermoFun {

/// The number type of the models without autodiff: a double that has the interface of autodiff::real used in the
/// models (`val()`, `[0]` is the value, `[1]` the derivative, which is always 0 and cannot be set), so that the models are
/// the same in both builds
class PlainReal
{
public:
    PlainReal() = default;
    PlainReal(double x) : x_(x) {}

    auto val() const -> double { return x_; }
    explicit operator double() const { return x_; }
    explicit operator bool() const { return x_ != 0.0; }

    auto operator[](std::size_t i) -> double& { return i == 0 ? x_ : zero(); }
    auto operator[](std::size_t i) const -> const double& { return i == 0 ? x_ : zeroConst(); }

    auto operator+=(const PlainReal& r) -> PlainReal& { x_ += r.x_; return *this; }
    auto operator-=(const PlainReal& r) -> PlainReal& { x_ -= r.x_; return *this; }
    auto operator*=(const PlainReal& r) -> PlainReal& { x_ *= r.x_; return *this; }
    auto operator/=(const PlainReal& r) -> PlainReal& { x_ /= r.x_; return *this; }

private:
    /// the (zero) derivative: a write to it is discarded by the next access
    static auto zero() -> double& { static thread_local double z = 0.0; z = 0.0; return z; }
    static auto zeroConst() -> const double& { static const double z = 0.0; return z; }

    double x_ = 0.0;
};

using real = PlainReal;

inline auto val(const PlainReal& x) -> double { return x.val(); }

inline auto operator+(const PlainReal& l, const PlainReal& r) -> PlainReal { return l.val() + r.val(); }
inline auto operator-(const PlainReal& l, const PlainReal& r) -> PlainReal { return l.val() - r.val(); }
inline auto operator*(const PlainReal& l, const PlainReal& r) -> PlainReal { return l.val() * r.val(); }
inline auto operator/(const PlainReal& l, const PlainReal& r) -> PlainReal { return l.val() / r.val(); }
inline auto operator+(const PlainReal& x) -> PlainReal { return x; }
inline auto operator-(const PlainReal& x) -> PlainReal { return -x.val(); }

inline auto operator==(const PlainReal& l, const PlainReal& r) -> bool { return l.val() == r.val(); }
inline auto operator!=(const PlainReal& l, const PlainReal& r) -> bool { return l.val() != r.val(); }
inline auto operator< (const PlainReal& l, const PlainReal& r) -> bool { return l.val() <  r.val(); }
inline auto operator> (const PlainReal& l, const PlainReal& r) -> bool { return l.val() >  r.val(); }
inline auto operator<=(const PlainReal& l, const PlainReal& r) -> bool { return l.val() <= r.val(); }
inline auto operator>=(const PlainReal& l, const PlainReal& r) -> bool { return l.val() >= r.val(); }

#define THERMOFUN_PLAIN_REAL_FUNCTION(name) \
    inline auto name(const PlainReal& x) -> PlainReal { return std::name(x.val()); }
THERMOFUN_PLAIN_REAL_FUNCTION(sqrt)
THERMOFUN_PLAIN_REAL_FUNCTION(cbrt)
THERMOFUN_PLAIN_REAL_FUNCTION(exp)
THERMOFUN_PLAIN_REAL_FUNCTION(log)
THERMOFUN_PLAIN_REAL_FUNCTION(log10)
THERMOFUN_PLAIN_REAL_FUNCTION(sin)
THERMOFUN_PLAIN_REAL_FUNCTION(cos)
THERMOFUN_PLAIN_REAL_FUNCTION(tan)
THERMOFUN_PLAIN_REAL_FUNCTION(asin)
THERMOFUN_PLAIN_REAL_FUNCTION(acos)
THERMOFUN_PLAIN_REAL_FUNCTION(atan)
THERMOFUN_PLAIN_REAL_FUNCTION(sinh)
THERMOFUN_PLAIN_REAL_FUNCTION(cosh)
THERMOFUN_PLAIN_REAL_FUNCTION(tanh)
THERMOFUN_PLAIN_REAL_FUNCTION(erf)
THERMOFUN_PLAIN_REAL_FUNCTION(erfc)
THERMOFUN_PLAIN_REAL_FUNCTION(floor)
THERMOFUN_PLAIN_REAL_FUNCTION(ceil)
THERMOFUN_PLAIN_REAL_FUNCTION(fabs)
THERMOFUN_PLAIN_REAL_FUNCTION(abs)
#undef THERMOFUN_PLAIN_REAL_FUNCTION

inline auto pow(const PlainReal& x, const PlainReal& y) -> PlainReal { return std::pow(x.val(), y.val()); }
inline auto atan2(const PlainReal& y, const PlainReal& x) -> PlainReal { return std::atan2(y.val(), x.val()); }
inline auto hypot(const PlainReal& x, const PlainReal& y) -> PlainReal { return std::hypot(x.val(), y.val()); }

inline auto fabs(double x) -> double { return std::fabs(x); }

} // namespace ThermoFun

namespace Reaktoro_ {

using real = ThermoFun::PlainReal;

inline constexpr bool kWithAutodiff = false;

} // namespace Reaktoro_

#endif // THERMOFUN_NO_AUTODIFF
