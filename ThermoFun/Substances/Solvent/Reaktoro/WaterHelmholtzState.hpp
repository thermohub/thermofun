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

#pragma once

// Reaktoro includes (modified DM 11.05.2016)
#include "Common/Real.hpp"

namespace ThermoFun {

struct WaterHelmholtzState
{
	/// The specific Helmholtz free energy of water (in units of J/kg)
    real helmholtz;

	/// The first-order partial derivative of the specific Helmholtz free energy of water with respect to temperature
    real helmholtzT;

	/// The first-order partial derivative of the specific Helmholtz free energy of water with respect to density
    real helmholtzD;

	/// The second-order partial derivative of the specific Helmholtz free energy of water with respect to temperature
    real helmholtzTT;

	/// The second-order partial derivative of the specific Helmholtz free energy of water with respect to temperature and density
    real helmholtzTD;

	/// The second-order partial derivative of the specific Helmholtz free energy of water with respect to density
    real helmholtzDD;

	/// The third-order partial derivative of the specific Helmholtz free energy of water with respect to temperature
    real helmholtzTTT;

	/// The third-order partial derivative of the specific Helmholtz free energy of water with respect to temperature, temperature, and density
    real helmholtzTTD;

	/// The third-order partial derivative of the specific Helmholtz free energy of water with respect to temperature, density, and density
    real helmholtzTDD;

	/// The third-order partial derivative of the specific Helmholtz free energy of water with respect to density
    real helmholtzDDD;
};

} // namespace Reaktoro
