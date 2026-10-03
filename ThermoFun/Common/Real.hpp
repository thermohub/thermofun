#pragma once

// autodiff includes
#include <autodiff/forward/real.hpp>

namespace ThermoFun {

/// The number type used in the models to calculate derivatives with autodiff
using real = autodiff::real;

} // namespace ThermoFun

namespace Reaktoro_ {

using real = autodiff::real;

} // namespace Reaktoro_
