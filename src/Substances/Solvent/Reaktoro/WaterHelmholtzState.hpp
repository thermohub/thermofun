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
#include <Substances/Solvent/Reaktoro/WaterThermoStateUtils.hpp>

namespace ThermoFun {

struct WaterHelmholtzState
{
	/// The specific Helmholtz free energy of water (in units of J/kg)
    Reaktoro_::ThermoScalar helmholtz;

	/// The first-order partial derivative of the specific Helmholtz free energy of water with respect to temperature
    Reaktoro_::ThermoScalar helmholtzT;

	/// The first-order partial derivative of the specific Helmholtz free energy of water with respect to density
    Reaktoro_::ThermoScalar helmholtzD;

	/// The second-order partial derivative of the specific Helmholtz free energy of water with respect to temperature
    Reaktoro_::ThermoScalar helmholtzTT;

	/// The second-order partial derivative of the specific Helmholtz free energy of water with respect to temperature and density
    Reaktoro_::ThermoScalar helmholtzTD;

	/// The second-order partial derivative of the specific Helmholtz free energy of water with respect to density
    Reaktoro_::ThermoScalar helmholtzDD;

	/// The third-order partial derivative of the specific Helmholtz free energy of water with respect to temperature
    Reaktoro_::ThermoScalar helmholtzTTT;

	/// The third-order partial derivative of the specific Helmholtz free energy of water with respect to temperature, temperature, and density
    Reaktoro_::ThermoScalar helmholtzTTD;

	/// The third-order partial derivative of the specific Helmholtz free energy of water with respect to temperature, density, and density
    Reaktoro_::ThermoScalar helmholtzTDD;

	/// The third-order partial derivative of the specific Helmholtz free energy of water with respect to density
    Reaktoro_::ThermoScalar helmholtzDDD;
};

} // namespace Reaktoro
