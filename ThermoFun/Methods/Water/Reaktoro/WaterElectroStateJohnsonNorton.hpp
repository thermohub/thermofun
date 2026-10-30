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
struct WaterElectroState;
struct WaterThermoState;

// Calculate the electrostatic state of water using the model of Johnson and Norton (1991)
auto waterElectroStateJohnsonNorton(real T, const WaterThermoState& wts, int state=-1) -> WaterElectroState;

} // namespace Reaktoro
