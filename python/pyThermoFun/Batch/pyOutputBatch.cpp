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
#include <ThermoFun/Batch/ThermoBatch.h>
#include <ThermoFun/Batch/OutputBatch.h>
//#include <ThermoFun/Database.h>
//#include <ThermoFun/ThermoProperties.h>
using namespace std;

namespace ThermoFun {

void exportOutput(py::module& m)
{
    py::class_<Output>(m, "Output")
        .def(py::init<const ThermoBatch&>())
        .def("toCSV", &Output::toCSV)
        .def("toCSVTransposed", &Output::toCSVTransposed)
        .def("toCSVPropertyGrid", &Output::toCSVPropertyGrid)
        .def("toDouble", &Output::toDouble)
        .def("toThermoScalar", &Output::toThermoScalar)
        .def("to2DVectorDouble", &Output::to2DVectorDouble)
        ;
}
}
