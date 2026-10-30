// SPDX-License-Identifier: LGPL-2.1-or-later
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

// Forward declarations
struct WaterHelmholtzState;

/// Calculate the Helmholtz free energy state of water using the Haar-Gallagher-Kell (1984) equation of state
/// @param T The temperature of water (in units of K)
/// @param D The density of water (in units of kg/m3)
/// @return The Helmholtz free energy state of water
/// @see WaterHelmholtzState
auto waterHelmholtzStateHGK(real T, real D) -> WaterHelmholtzState;

} // namespace Reaktoro
