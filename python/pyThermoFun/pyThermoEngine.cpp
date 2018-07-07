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
#include <pybind11/stl.h>
namespace py = pybind11;

// ThermoFun includes
#include <thermofun/ThermoEngine.h>
#include <thermofun/Database.h>
#include <thermofun/ThermoProperties.h>

namespace ThermoFun {

void exportThermoEngine(py::module& m)
{
    py::class_<ThermoEngine>(m, "ThermoEngine")
//        .def(py::init<>())
        .def(py::init<const std::string>())
        .def(py::init<const Database&>())
        .def("thermoPropertiesSubstance", &ThermoEngine::thermoPropertiesSubstance, (py::arg("T"), py::arg("P"), "substance"))
        .def("electroPropertiesSolvent", &ThermoEngine::electroPropertiesSolvent, (py::arg("T"), py::arg("P"), "solvent"))
        .def("propertiesSolvent", &ThermoEngine::propertiesSolvent, (py::arg("T"), py::arg("P"), "solvent"))
        .def("thermoPropertiesReaction", &ThermoEngine::thermoPropertiesReaction, (py::arg("T"), py::arg("P"), "reaction"))
        .def("thermoPropertiesReactionFromReactants", &ThermoEngine::thermoPropertiesReaction, (py::arg("T"), py::arg("P"), "symbol"))
        ;
}

}
