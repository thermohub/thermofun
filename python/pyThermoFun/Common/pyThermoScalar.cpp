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

#if (_MSC_VER > 1929)
#include <corecrt.h>
#endif

// pybind11 includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

// thermofun includes
#include <ThermoFun/Common/ThermoScalar.hpp>
namespace r_ = Reaktoro_;

namespace ThermoFun {

auto exportStatus(py::module& m) -> void
{
    py::enum_<r_::Status>(m, "Status")
        .value("notdefined", r_::Status::notdefined)
        .value("read", r_::Status::read)
        .value("calculated", r_::Status::calculated)
        .value("assigned", r_::Status::assigned)
        .value("initialized", r_::Status::initialized)
        ;
}

void exportThermoScalar(py::module& m)
{
    py::class_<r_::ThermoScalar>(m, "ThermoScalar")
        .def(py::init<>())
        .def(py::init<double>())
        .def(py::init<double, double, double, double, r_::StatusMessage>())
        .def_readwrite("val", &r_::ThermoScalar::val)
        .def_readwrite("ddt", &r_::ThermoScalar::ddt)
        .def_readwrite("ddp", &r_::ThermoScalar::ddp)
        .def_readwrite("err", &r_::ThermoScalar::err)
        .def_readwrite("sta", &r_::ThermoScalar::sta)
        ;
}

void exportTemperature(py::module& m)
{
    py::class_<r_::Temperature, r_::ThermoScalar>(m, "Temperature")
        .def(py::init<>())
        .def(py::init<double>())
        ;

    py::implicitly_convertible<double, r_::Temperature>();
}

void exportPressure(py::module& m)
{
    py::class_<r_::Pressure, r_::ThermoScalar>(m, "Pressure")
        .def(py::init<>())
        .def(py::init<double>())
        ;

    py::implicitly_convertible<double, r_::Pressure>();
}

}
