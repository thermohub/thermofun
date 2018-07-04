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

// thermofun includes
#include <thermofun/Common/ThermoScalar.hpp>

namespace ThermoFun {

namespace r_ = Reaktoro_;

    py::enum_<Status>(m, "Status", py::arithmetic())
        .value("notdefined", r_::Status::notdefined)
        .value("read", r_::Status::read)
        .value("calculated", r_::Status::calculated)
        .value("assigned", r_::Status::assigned)
        .value("initialized", r_::Status::initialized)
        .export_values();
    
    py::bind_pair<std::pair<r_::Status, std::string>>(m, "StatusMessage");

void exportThermoScalar(py::module& m)
{
    py::class_<ThermoScalar>(m, "ThermoScalar")
        .def(py::init<>())
        .def(py::init<double>())
        .def(py::init<double, double, double, double, r_::StatusMessage>())
        .def_readwrite("val", &r_::ThermoScalar::val)
        .def_readwrite("ddT", &r_::ThermoScalar::ddT)
        .def_readwrite("ddP", &r_::ThermoScalar::ddP)
        .def_readwrite("err", &r_::ThermoScalar::err)
        .def_readwrite("sta", &r_::ThermoScalar::StatusMessage)
        ;
}

}