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
#include <ThermoFun/ThermoEngine.h>
#include <ThermoFun/Database.h>
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun {

void exportThermoEngine(py::module& m)
{
    auto appendData1 = static_cast<void(ThermoEngine::*)(std::string)>(&ThermoEngine::appendData);
    auto appendData2 = static_cast<void(ThermoEngine::*)(std::vector<std::string>)>(&ThermoEngine::appendData);

    py::class_<ThermoEngine>(m, "ThermoEngine")
//        .def(py::init<>())
        .def(py::init<const std::string>())
        .def(py::init<const Database&>())
        .def(py::init<const ThermoEngine&>())
        .def("appendData", appendData1, "Append records to the database from a file.")
        .def("appendData", appendData2, "Append records to the database from a list of JSON strings.")
        .def("setSolventSymbol", &ThermoEngine::setSolventSymbol)
        .def("solventSymbol", &ThermoEngine::solventSymbol)
        .def("thermoPropertiesSubstance", &ThermoEngine::thermoPropertiesSubstance, (py::arg("T"), py::arg("P"), "substance"))
        .def("electroPropertiesSolvent", &ThermoEngine::electroPropertiesSolvent, (py::arg("T"), py::arg("P"), "solvent"))
        .def("propertiesSolvent", &ThermoEngine::propertiesSolvent, (py::arg("T"), py::arg("P"), "solvent"))
        .def("thermoPropertiesReaction", &ThermoEngine::thermoPropertiesReaction, (py::arg("T"), py::arg("P"), "reaction"))
        .def("thermoPropertiesReactionFromReactants", &ThermoEngine::thermoPropertiesReactionFromReactants, (py::arg("T"), py::arg("P"), "symbol"))
        ;
}

}
