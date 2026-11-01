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

auto textOfRule(const py::object& rule) -> std::string
{
    return py::isinstance<py::str>(rule) ? rule.cast<std::string>() : py::module_::import("json").attr("dumps")(rule).cast<std::string>();
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

    m.def("selectSubstances", [](const Database& database, const py::object& rule) { return selectSubstances(database, textOfRule(rule)); }, py::arg("database"), py::arg("rule"),
          "The substances of a database that a rule chooses, for example {\"class\": \"aqueous_solute\", \"elements\": [\"Ca\"]}");
    m.def("selectReactions", [](const Database& database, const py::object& rule) { return selectReactions(database, textOfRule(rule)); }, py::arg("database"), py::arg("rule"),
          "The reactions of a database that a rule chooses");

    py::class_<TableProvenance>(m, "TableProvenance", "How a table or a batch calculation was made")
        .def_readonly("database", &TableProvenance::database)
        .def_readonly("templates", &TableProvenance::templates)
        .def_readonly("calculations", &TableProvenance::calculations, "For each engine: its label in compare, and its snapshot")
        .def_readonly("setup", &TableProvenance::setup, "The settings of the table, as YAML")
        .def("lines", &TableProvenance::lines, "Short lines of text: what goes at the top of an output")
        .def("to_yaml", &TableProvenance::toYaml, "All of it as YAML");

    py::class_<TableCell>(m, "TableCell", "One value of a table")
        .def_property_readonly("value", &cellValue, "The number in the unit of its column, or the status")
        .def_readonly("defined", &TableCell::defined)
        .def_property_readonly("result", [](const TableCell& c) { return c.has_result ? py::cast(c.result) : py::object(py::none()); }, "The result, in the units of the engine");

    py::class_<Table>(m, "Table", "A table of properties")
        .def_property_readonly("name", &Table::name)
        .def_property_readonly("output", &Table::output)
        .def_property_readonly("provenance", &Table::provenance, "How the table was made")
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
        .def("to_text", &Table::toText, py::arg("format") = "csv", py::arg("with_provenance") = false, "The table as text: csv, md or tex")
        .def("write", &Table::write, py::arg("path") = "", "Writes to a path, or to the output of the setup. Gives the path");

    py::class_<TableResults>(m, "TableResultsCore", "The tables of a setup")
        .def("tables", &TableResults::tables);

    py::class_<BatchCalculation>(m, "BatchCalculation", "One table of a setup as a calculation of ThermoBatch")
        .def_property_readonly("batch", &BatchCalculation::batch, py::return_value_policy::reference_internal, "The ThermoBatch, with its engine, units, digits and preferences from the setup")
        .def_property_readonly("symbols", &BatchCalculation::symbols)
        .def_property_readonly("properties", &BatchCalculation::properties)
        .def_property_readonly("points", &BatchCalculation::points, "The T-P points, in the units of the setup. P = 0 is the saturation pressure")
        .def_property_readonly("for_reactions", &BatchCalculation::forReactions)
        .def_property_readonly("provenance", &BatchCalculation::provenance)
        .def("run", &BatchCalculation::run, "Calculates, and gives the Output of ThermoBatch")
        .def("write", &BatchCalculation::write, py::arg("path") = "", "Calculates, and writes in the layout of the setup. Gives the path");

    py::class_<TableSetup>(m, "TableSetup", "How to make tables: a YAML setup or a template, changed with these functions")
        .def(py::init<>())
        .def_static("fromText", &TableSetup::fromText, "From YAML or JSON text")
        .def_static("fromFile", &TableSetup::fromFile, "From a file")
        .def_static("fromTemplate", &TableSetup::fromTemplate, "From a template of the library")
        .def_static("templates", &TableSetup::templates, "The templates: name and what it makes")
        .def_static("templateText", &TableSetup::templateText, "The YAML text of a template")
        .def_static("templateSource", &TableSetup::templateSource, "Where a template comes from: the path of its file, or library")
        .def_static("templateFolders", &TableSetup::templateFolders, "The folders of THERMOFUN_TEMPLATE_PATH")
        .def("useTemplate", &TableSetup::useTemplate, py::arg("name"), "Puts a template under the settings")
        .def("setDatabase", py::overload_cast<const Database&>(&TableSetup::setDatabase))
        .def("setDatabase", py::overload_cast<const std::string&>(&TableSetup::setDatabase))
        .def("setRecords", &TableSetup::setRecords, py::arg("symbols"), py::arg("table") = "")
        .def("setReactions", &TableSetup::setReactions, py::arg("symbols"), py::arg("table") = "")
        .def("setRecordRule", [](TableSetup& s, const py::object& rule, const std::string& table) -> TableSetup& { return s.setRecordRule(textOfRule(rule), table); }, py::arg("rule"), py::arg("table") = "")
        .def("setReactionRule", [](TableSetup& s, const py::object& rule, const std::string& table) -> TableSetup& { return s.setReactionRule(textOfRule(rule), table); }, py::arg("rule"), py::arg("table") = "")
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
        .def("runCore", &TableSetup::run)
        .def("batch", &TableSetup::batch, py::arg("table") = "", "One table as a calculation of ThermoBatch")
        .def("applyTo", &TableSetup::applyTo, py::arg("batch"), py::arg("table") = "", "Sets the units, digits and preferences of a ThermoBatch from one table");
}
