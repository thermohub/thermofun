// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Python bindings of the utility functions of ThermoFun: units, parsing of JSON records, the thermodynamic variables

#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include <ThermoFun/Common/Units.hpp>
#include <ThermoFun/Common/ParseJsonToData.h>
#include <ThermoFun/Database.h>
#include <ThermoFun/Element.h>
#include <ThermoFun/Substance.h>
#include <ThermoFun/Reaction.h>
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun {

void exportUtilities(py::module& m)
{
    // units
    m.def("convert_units", &units::convert, py::arg("value"), py::arg("from_unit"), py::arg("to_unit"),
          "Convert a value between units, for example convert_units(1.0, 'cal/mol', 'J/mol')");
    m.def("units_convertible", &units::convertible, py::arg("from_unit"), py::arg("to_unit"),
          "Check if a unit can be converted to another unit");

    // records
    m.def("parseElement", &parseElement, py::arg("json"), "Parse an element record given as a JSON string");
    m.def("parseSubstance", &parseSubstance, py::arg("json"), "Parse a substance record given as a JSON string");
    m.def("parseReaction", &parseReaction, py::arg("json"), "Parse a reaction record given as a JSON string");
    m.def("readConventions", &readConventions, py::arg("json"), "Read the conventions of a database given as a JSON string");

    // the thermodynamic variables
    py::class_<ThermoVariables>(m, "ThermoVariables", "The temperature (K) and the pressure (Pa)")
        .def(py::init<>())
        .def_readwrite("temperature", &ThermoVariables::temperature)
        .def_readwrite("pressure", &ThermoVariables::pressure);
}

} // namespace ThermoFun
