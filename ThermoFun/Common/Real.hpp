#pragma once

// autodiff includes
#include <cmath>
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

} // namespace Reaktoro_
