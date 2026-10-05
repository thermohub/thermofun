// SPDX-License-Identifier: LGPL-2.1-or-later
#include <ThermoFun/ThermoProperties.h>
// Reaktoro is a unified framework for modeling chemically reactive systems.
//
// Copyright (C) 2014-2015 Allan Leal
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

// Reaktoro includes (modified DM 11.05.2016)
#include <ThermoFun/Common/Real.hpp>

namespace ThermoFun {

/// Calculate the density of water using the Haar-Gallagher-Kell (1984) equation of state
/// @param T The temperature of water (in units of K)
/// @param P The pressure of water (in units of Pa)
/// @return The density of water (in units of kg/m3)
auto waterDensityHGK(real T, real P, int state) -> real;

/// Calculate the density of water using the Wagner and Pruss (1995) equation of state
/// @param T The temperature of water (in units of K)
/// @param P The pressure of water (in units of Pa)
/// @return The density of water (in units of kg/m3)
auto waterDensityWagnerPruss(real T, real P, int state) -> real;

/// Calculate the pressure of water using the Haar-Gallagher-Kell (1984) equation of state
/// @param T The temperature of water (in units of K)
/// @param D The density of water (in units of kg/m3)
/// @return The pressure of water (in units of Pa)
auto waterPressureHGK(real T, real D) -> real;

/// Calculate the pressure of water using the Wagner and Pruss (1995) equation of state
/// @param T The temperature of water (in units of K)
/// @param D The density of water (in units of kg/m3)
/// @return The pressure of water (in units of Pa)
auto waterPressureWagnerPruss(real T, real D) -> real;

/// Calculate the saturated pressure of water using the Wagner and Pruss (1995) equation of state
/// @param T The temperature of water (in units of K)
/// @return The saturated pressure of water (in units of Pa)
auto waterSaturatedPressureWagnerPruss(real T) -> real;

/// Calculate the saturated liquid-density of water using the Wagner and Pruss (1995) equation of state
/// @param T The temperature of water (in units of K)
/// @return The saturated liquid-density of water (in units of kg/m3)
auto waterSaturatedLiquidDensityWagnerPruss(real T) -> real;

/// Calculate the saturated vapour-density of water using the Wagner and Pruss (1995) equation of state
/// @param T The temperature of water (in units of K)
/// @return The saturated vapour-density of water (in units of kg/m3)
auto waterSaturatedVapourDensityWagnerPruss(real T) -> real;

} // namespace Reaktoro

namespace ThermoFun
{
// Forward declarations

auto waterIdealGas (real t, ThermoFun::PropertiesSolvent &wp) -> void;

}
