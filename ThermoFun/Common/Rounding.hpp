#ifndef THERMOFUN_ROUNDING_HPP
#define THERMOFUN_ROUNDING_HPP

#include <algorithm>
#include <cfloat>
#include <cmath>

namespace ThermoFun {

/// Rounding rules of the NEA TDB project (Guidelines for the assignment of uncertainties, TDB-3, Wanner 1999, and
/// TDB-1): a value is given with as many decimals as its uncertainty, with the "round half to even" rule.
namespace rounding {

namespace detail {
/// the tolerance with which a scaled number is taken as exactly at a half (or at an integer): the binary
/// representation of a decimal number like 25.45 is not exact (capped, so that it stays far below the 0.5 of a
/// rounding interval for the very large scaled numbers)
inline double tolerance(double scaled) { return std::min(8.0 * DBL_EPSILON * std::max(1.0, std::fabs(scaled)), 1e-3); }
}

/// Round to a number of decimals (negative: to tens, hundreds, ...): a digit following the last digit retained that
/// is less than 5 is dropped, a digit greater than 5 increases the last digit by 1; if the digit is 5 followed by
/// no other non-zero digit, an odd last digit is increased by 1 and an even digit is kept; if other non-zero digits
/// follow the 5, the last digit is increased by 1.
inline double roundHalfEven(double x, int decimals)
{
    if (!std::isfinite(x)) return x;
    const double p = std::pow(10.0, decimals);
    const double scaled = x * p;
    const double fl = std::floor(scaled);
    const double r = scaled - fl;
    double result;
    if (std::fabs(r - 0.5) <= detail::tolerance(scaled))
        result = (std::fmod(fl, 2.0) == 0.0) ? fl : fl + 1.0;
    else
        result = (r < 0.5) ? fl : fl + 1.0;
    return result / p;
}

/// Round up (away from the smaller values) to a number of decimals, used for the uncertainties
inline double roundUp(double x, int decimals)
{
    if (!std::isfinite(x)) return x;
    const double p = std::pow(10.0, decimals);
    const double scaled = x * p;
    const double fl = std::floor(scaled);
    const double result = (scaled - fl <= detail::tolerance(scaled)) ? fl : fl + 1.0;
    return result / p;
}

/// Round an uncertainty to a number of significant digits (up), and the value to the same number of decimals
/// (rounded half to even). Nothing is changed if the uncertainty is not positive or not finite.
/// @param value the value
/// @param error the uncertainty
/// @param significantDigits the number of significant digits of the uncertainty (>= 1)
inline void toUncertainty(double& value, double& error, int significantDigits)
{
    if (!(error > 0.0) || !std::isfinite(error) || !std::isfinite(value)) return;
    const int digits = std::max(1, significantDigits);
    int exponent = static_cast<int>(std::floor(std::log10(error)));
    int decimals = digits - 1 - exponent;
    double rounded = roundUp(error, decimals);
    if (rounded >= std::pow(10.0, exponent + 1) * (1.0 - 1e-12)) // the uncertainty gained a digit: 0.96 -> 1.0
    {
        ++exponent;
        decimals = digits - 1 - exponent;
        rounded = roundUp(error, decimals);
    }
    error = rounded;
    value = roundHalfEven(value, decimals);
}

} // namespace rounding
} // namespace ThermoFun

#endif // THERMOFUN_ROUNDING_HPP
