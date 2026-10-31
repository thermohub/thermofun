// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include <ThermoFun/Common/Diagnostics.h>

namespace ThermoFun {

void exportDiagnostics(py::module& m)
{
    py::enum_<Severity>(m, "Severity", "Severity of a diagnostic")
        .value("Info", Severity::Info).value("Warning", Severity::Warning).value("Error", Severity::Error);

    py::class_<Diagnostic>(m, "Diagnostic", "A problem or a note about a record")
        .def(py::init<>())
        .def_readwrite("severity", &Diagnostic::severity)
        .def_readwrite("code", &Diagnostic::code)
        .def_readwrite("file", &Diagnostic::file)
        .def_readwrite("record", &Diagnostic::record)
        .def_readwrite("path", &Diagnostic::path)
        .def_readwrite("message", &Diagnostic::message)
        .def("__str__", [](const Diagnostic& d) { return toString(d); })
        .def("__repr__", [](const Diagnostic& d) { return "Diagnostic(" + d.code + ")"; });

    py::class_<Diagnostics>(m, "Diagnostics", "A list of diagnostics")
        .def(py::init<>())
        .def("add", &Diagnostics::add)
        .def("ok", &Diagnostics::ok, "True if the list has no error")
        .def("all", &Diagnostics::all)
        .def("errors", &Diagnostics::errors)
        .def("__len__", [](const Diagnostics& d) { return d.all().size(); })
        .def("__iter__", [](const Diagnostics& d) { return py::make_iterator(d.all().begin(), d.all().end()); },
             py::keep_alive<0, 1>())
        .def("__str__", &Diagnostics::toString);

    py::class_<DiagnosticInfo>(m, "DiagnosticInfo", "A code of the catalog of diagnostics")
        .def_readonly("code", &DiagnosticInfo::code)
        .def_readonly("severity", &DiagnosticInfo::severity)
        .def_readonly("meaning", &DiagnosticInfo::meaning);

    m.def("diagnosticCatalog", &diagnosticCatalog, "All codes of diagnostics with their meaning");
}

} // namespace ThermoFun
