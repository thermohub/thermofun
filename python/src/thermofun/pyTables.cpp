// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <limits>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <ThermoFun/Batch/Tables.h>
#include <ThermoFun/Database.h>

#include "PyThermoFun.hpp"

namespace py = pybind11;
using namespace ThermoFun;

namespace {

auto pressureOf(const py::object& P, bool& saturation) -> double
{
    saturation = py::isinstance<py::str>(P);
    return saturation ? 0.0 : P.cast<double>();
}

auto pressureObject(double P, bool saturation) -> py::object
{
    return saturation ? py::object(py::str("saturation")) : py::object(py::float_(P));
}

auto cellValue(const TableCell& cell) -> py::object
{
    if (!cell.text.empty())
        return py::str(cell.text);
    return py::float_(cell.defined ? cell.value : std::numeric_limits<double>::quiet_NaN());
}

} // namespace

void ThermoFun::exportTables(py::module& m)
{
    py::register_exception<TableSetupError>(m, "TableSetupError", PyExc_RuntimeError);

    py::class_<TableCell>(m, "TableCell", "One value of a table")
        .def_property_readonly("value", &cellValue, "The number in the unit of its column, or the status")
        .def_readonly("defined", &TableCell::defined)
        .def_property_readonly("result", [](const TableCell& c) { return c.has_result ? py::cast(c.result) : py::object(py::none()); }, "The result, in the units of the engine");

    py::class_<Table>(m, "Table", "A table of properties")
        .def_property_readonly("name", &Table::name)
        .def_property_readonly("output", &Table::output)
        .def_property_readonly("columns", [](const Table& t) {
            py::list out;
            for (const auto& c : t.columns())
            {
                py::dict row;
                row["property"] = c.property; row["label"] = c.label; row["show"] = c.show; row["unit"] = c.unit;
                out.append(row);
            }
            return out;
        }, "The columns: property, label, show, unit")
        .def_property_readonly("rows", [](const Table& t) {
            py::list out;
            for (const auto& r : t.rows())
                out.append(py::make_tuple(r.record, r.T, pressureObject(r.P, r.saturation), r.cells));
            return out;
        }, "The rows: (record, T, P, cells), with T and P as in the setup")
        .def_property_readonly("problems", [](const Table& t) {
            py::list out;
            for (const auto& p : t.problems())
                out.append(py::make_tuple(p.record, p.has_point ? py::object(py::float_(p.T)) : py::object(py::none()), p.has_point ? pressureObject(p.P, p.saturation) : py::object(py::none()), p.message));
            return out;
        }, "What could not be calculated: (record, T, P, message)")
        .def("values", &Table::values, "The rows of numbers, in the units of the columns; NaN if not defined")
        .def("value", [](const Table& t, const std::string& record, double T, const py::object& P, const std::string& column) {
            bool saturation = false;
            const double p = pressureOf(P, saturation);
            return cellValue(t.cell(record, T, p, column, saturation));
        }, py::arg("record"), py::arg("T"), py::arg("P"), py::arg("column"), "One number, in the unit of its column. P can be \"saturation\"")
        .def("scalar", [](const Table& t, const std::string& record, double T, const py::object& P, const std::string& column) {
            bool saturation = false;
            const double p = pressureOf(P, saturation);
            return t.cell(record, T, p, column, saturation).result;
        }, py::arg("record"), py::arg("T"), py::arg("P"), py::arg("column"), "One result with its derivatives, uncertainty and status, in the units of the engine")
        .def("grid", &Table::lines, "The header and the rows as text, in the layout long or transposed")
        .def("property_grids", &Table::propertyGrids, "One grid for each column: (label, lines)")
        .def("to_text", &Table::toText, py::arg("format") = "csv", "The table as text: csv, md or tex")
        .def("write", &Table::write, py::arg("path") = "", "Writes to a path, or to the output of the setup. Gives the path");

    py::class_<TableResults>(m, "TableResultsCore", "The tables of a setup")
        .def("tables", &TableResults::tables);

    py::class_<TableSetup>(m, "TableSetup", "How to make tables: a YAML setup or a template, changed with these functions")
        .def(py::init<>())
        .def_static("fromText", &TableSetup::fromText, "From YAML or JSON text")
        .def_static("fromFile", &TableSetup::fromFile, "From a file")
        .def_static("fromTemplate", &TableSetup::fromTemplate, "From a template of the library")
        .def_static("templates", &TableSetup::templates, "The templates: name and what it makes")
        .def_static("templateText", &TableSetup::templateText, "The YAML text of a template")
        .def("setDatabase", py::overload_cast<const Database&>(&TableSetup::setDatabase))
        .def("setDatabase", py::overload_cast<const std::string&>(&TableSetup::setDatabase))
        .def("setRecords", &TableSetup::setRecords, py::arg("symbols"), py::arg("table") = "")
        .def("setReactions", &TableSetup::setReactions, py::arg("symbols"), py::arg("table") = "")
        .def("setTemperatures", py::overload_cast<const std::vector<double>&, const std::string&, const std::string&>(&TableSetup::setTemperatures), py::arg("values"), py::arg("unit") = "K", py::arg("table") = "")
        .def("setTemperatures", py::overload_cast<double, double, double, const std::string&, const std::string&>(&TableSetup::setTemperatures), py::arg("first"), py::arg("last"), py::arg("step"), py::arg("unit") = "K", py::arg("table") = "")
        .def("setPressures", py::overload_cast<const std::vector<double>&, const std::string&, const std::string&>(&TableSetup::setPressures), py::arg("values"), py::arg("unit") = "Pa", py::arg("table") = "")
        .def("setPressures", py::overload_cast<double, double, double, const std::string&, const std::string&>(&TableSetup::setPressures), py::arg("first"), py::arg("last"), py::arg("step"), py::arg("unit") = "Pa", py::arg("table") = "")
        .def("setSaturationPressure", &TableSetup::setSaturationPressure, py::arg("table") = "")
        .def("setLayout", &TableSetup::setLayout, py::arg("layout"), py::arg("table") = "")
        .def("setOutput", &TableSetup::setOutput, py::arg("path"), py::arg("table") = "")
        .def("setColumn", &TableSetup::setColumn, py::arg("property"), py::arg("unit"), py::arg("digits") = -1, py::arg("table") = "")
        .def("addColumn", &TableSetup::addColumn, py::arg("property"), py::arg("unit") = "", py::arg("digits") = -1, py::arg("table") = "")
        .def("removeColumn", &TableSetup::removeColumn, py::arg("property"), py::arg("table") = "")
        .def("setJson", &TableSetup::set, py::arg("key"), py::arg("json"), py::arg("table") = "")
        .def("changeColumn", &TableSetup::changeColumn, py::arg("property"), py::arg("json"), py::arg("table") = "")
        .def("addColumnFromJson", &TableSetup::addColumnFromJson, py::arg("json"), py::arg("table") = "")
        .def("toYaml", &TableSetup::toYaml)
        .def("save", &TableSetup::save)
        .def("runCore", &TableSetup::run);
}
