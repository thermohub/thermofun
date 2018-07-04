// This file is part of ThermoFun https://bitbucket.org/gems4/thermofun/
// ThermoFun is a framework for delivering standard state thermodynamic data.
//
// Copyright (c) 2016-2018 G.D.Miron, D.A.Kulik, A.Leal 
//
// ThermoFun is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.

// ThermoFun is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ThermoFun code. If not, see <http://www.gnu.org/licenses/>.

// pybind11 includes
#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace ThermoFun {

// Common module
void exportThermoScalar(py::module& m);

// Core module
// void exportThermoEngine(py::module& m);
// void exportThermoBatch(py::module& m);


// void exportChemicalProperties(py::module& m);
// void exportChemicalProperty(py::module& m);
// void exportChemicalQuantity(py::module& m);
// void exportChemicalState(py::module& m);
// void exportChemicalSystem(py::module& m);
// void exportConnectivity(py::module& m);
// void exportElement(py::module& m);
// void exportPartition(py::module& m);
// void exportPhase(py::module& m);
// void exportReaction(py::module& m);
// void exportReactionSystem(py::module& m);
// void exportSpecies(py::module& m);
// void exportThermoProperties(py::module& m);
// void exportUtils(py::module& m);

} // namespace ThermoFun
