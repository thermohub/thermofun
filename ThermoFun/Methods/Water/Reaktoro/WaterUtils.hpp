// SPDX-License-Identifier: LGPL-2.1-or-later
#include <ThermoFun/Common/ThermoProperties.h>
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

#include <ThermoFun/Methods/Water/Reaktoro/WaterHelmholtzModelParameters.hpp>

namespace ThermoFun {

/// Calculate the density of water using the Haar-Gallagher-Kell (1984) equation of state
/// @param T The temperature of water (in units of K)
/// @param P The pressure of water (in units of Pa)
/// @return The density of water (in units of kg/m3)
auto waterDensityHGK(real T, real P, int state, const HGKModelParameters& constants) -> real;

/// Calculate the density of water using the Wagner and Pruss (1995) equation of state
/// @param T The temperature of water (in units of K)
/// @param P The pressure of water (in units of Pa)
/// @return The density of water (in units of kg/m3)
auto waterDensityWagnerPruss(real T, real P, int state, const WagnerPrussModelParameters& constants) -> real;

/// The density of the vapour at T (K) and P (Pa) from the gas equation of Wagner and Pruss (2002), Eq. 3.2: the constants are
/// those of IAPWS-95 with the residual terms replaced by the 7 terms of the gas equation. ok is false if the vapour has no
/// stable root at P (beyond its spinodal) or its density is above Dmax (kg/m3), the limit of the equation.
auto waterDensityGasEquation(real T, real P, const WagnerPrussModelParameters& constants, double Dmax, bool& ok) -> real;

/// Calculate the saturated pressure of water using the Wagner and Pruss (1995) equation of state
/// @param T The temperature of water (in units of K)
/// @return The saturated pressure of water (in units of Pa)
auto waterSaturatedPressureWagnerPruss(real T, const WaterSaturationModelParameters& s) -> real;

/// Calculate the saturated liquid-density of water using the Wagner and Pruss (1995) equation of state
/// @param T The temperature of water (in units of K)
/// @return The saturated liquid-density of water (in units of kg/m3)
auto waterSaturatedLiquidDensityWagnerPruss(real T, const WaterSaturationModelParameters& s) -> real;

/// Calculate the saturated vapour-density of water using the Wagner and Pruss (1995) equation of state
/// @param T The temperature of water (in units of K)
/// @return The saturated vapour-density of water (in units of kg/m3)
auto waterSaturatedVapourDensityWagnerPruss(real T, const WaterSaturationModelParameters& s) -> real;

} // namespace Reaktoro

namespace ThermoFun
{
// Forward declarations

auto waterIdealGas (real t, ThermoFun::PropertiesSolvent &wp) -> void;

}
