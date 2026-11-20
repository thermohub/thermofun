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

// You should have received a copy of the GNU Lesser General Public License
// along with ThermoFun code. If not, see <http://www.gnu.org/licenses/>.

#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

// pybind11 includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

// thermofun includes
#include <ThermoFun/Common/OutsideRange.h>
#include "pyInfo.h"
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
        .value("outofbounds", r_::Status::outofbounds)
        ;
}

void exportThermoScalar(py::module& m)
{
    py::enum_<Validity>(m, "Validity", "How far a value can be used: within every range it depends on, extrapolated beyond one, or undefined (no value)")
        .value("within", Validity::within)
        .value("extrapolated", Validity::extrapolated)
        .value("undefined", Validity::undefined);
    py::enum_<ErrorState>(m, "ErrorState", "Whether err can be trusted: known (the uncertainty of the data), partial (a contribution could not be propagated: err is NaN), unknown (the data give none: err is 0, which does not mean exact)")
        .value("known", ErrorState::known)
        .value("partial", ErrorState::partial)
        .value("unknown", ErrorState::unknown);
    py::class_<InfoView>(m, "Info", "What a value says about itself besides its number and its error (decision 0029)")
        .def_readonly("validity", &InfoView::validity, "within, extrapolated or undefined: made from the status and the notes")
        .def_readonly("status", &InfoView::status, "notdefined, read, assigned or calculated; None for a whole result")
        .def_readonly("error", &InfoView::error, "known, partial or unknown: whether err can be trusted; made from the notes and err")
        .def_readonly("source", &InfoView::source, "where the value came from: methods, reaction, reactants, reference, water, hydrogen_ion")
        .def_readonly("pressure", &InfoView::pressure, "the pressure used, Pa (the saturation pressure at P = 0); None if not known")
        .def_readonly("notes", &InfoView::notes, "the list of Note behind it: range, source, undefined, uncertainty")
        .def("__repr__", [](const InfoView& v) {
            const char* names[] = {"within", "extrapolated", "undefined"};
            return std::string("<Info ") + names[static_cast<int>(v.validity)] + ", " + std::to_string(v.notes.size()) + " notes>";
        });

    py::class_<r_::ThermoScalar>(m, "ThermoScalar")
        .def(py::init<>())
        .def(py::init<double>())
        .def(py::init<double, double, double, double, r_::StatusMessage>())
        .def_property("val", [](const r_::ThermoScalar& x) { return x.val(); }, [](r_::ThermoScalar& x, double v) { x.val = v; })
        .def_property("ddt", [](const r_::ThermoScalar& x) { return x.ddt(); }, [](r_::ThermoScalar& x, double v) { x.ddt = v; })
        .def_property("ddp", [](const r_::ThermoScalar& x) { return x.ddp(); }, [](r_::ThermoScalar& x, double v) { x.ddp = v; })
        .def_readwrite("err", &r_::ThermoScalar::err)
        .def_readwrite("sta", &r_::ThermoScalar::sta)
        .def_property_readonly("info", [](const r_::ThermoScalar& x) { return infoOf(x); },
                               "validity, status, error, source, pressure and the notes behind them (decision 0029)")
        // arithmetic with the propagation of the derivatives, the errors and the statuses (as in C++)
        .def("__add__", [](const r_::ThermoScalar& l, const r_::ThermoScalar& r) { return r_::ThermoScalar(l + r); }, py::is_operator())
        .def("__add__", [](const r_::ThermoScalar& l, double r) { return r_::ThermoScalar(l + r); }, py::is_operator())
        .def("__radd__", [](const r_::ThermoScalar& r, double l) { return r_::ThermoScalar(l + r); }, py::is_operator())
        .def("__sub__", [](const r_::ThermoScalar& l, const r_::ThermoScalar& r) { return r_::ThermoScalar(l - r); }, py::is_operator())
        .def("__sub__", [](const r_::ThermoScalar& l, double r) { return r_::ThermoScalar(l - r); }, py::is_operator())
        .def("__rsub__", [](const r_::ThermoScalar& r, double l) { return r_::ThermoScalar(l - r); }, py::is_operator())
        .def("__mul__", [](const r_::ThermoScalar& l, const r_::ThermoScalar& r) { return r_::ThermoScalar(l * r); }, py::is_operator())
        .def("__mul__", [](const r_::ThermoScalar& l, double r) { return r_::ThermoScalar(l * r); }, py::is_operator())
        .def("__rmul__", [](const r_::ThermoScalar& r, double l) { return r_::ThermoScalar(l * r); }, py::is_operator())
        .def("__truediv__", [](const r_::ThermoScalar& l, const r_::ThermoScalar& r) { return r_::ThermoScalar(l / r); }, py::is_operator())
        .def("__truediv__", [](const r_::ThermoScalar& l, double r) { return r_::ThermoScalar(l / r); }, py::is_operator())
        .def("__rtruediv__", [](const r_::ThermoScalar& r, double l) { return r_::ThermoScalar(l / r); }, py::is_operator())
        .def("__neg__", [](const r_::ThermoScalar& l) { return r_::ThermoScalar(-l); })
        .def("__pow__", [](const r_::ThermoScalar& l, double p) { return r_::ThermoScalar(pow(l, p)); }, py::is_operator())
        .def("__float__", [](const r_::ThermoScalar& x) { return x.val(); })
        .def("__repr__", [](const r_::ThermoScalar& x) {
            return "ThermoScalar(val=" + std::to_string(x.val()) + ", ddt=" + std::to_string(x.ddt()) + ", ddp=" + std::to_string(x.ddp()) +
                   ", err=" + std::to_string(x.err) + ")";
        })
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
