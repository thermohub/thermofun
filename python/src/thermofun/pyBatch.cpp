// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <limits>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <ThermoFun/Batch/BatchSetup.h>
#include <ThermoFun/ThermoEngine.h>
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

auto cellValue(const ResultCell& cell) -> py::object
{
    if (!cell.text.empty())
        return py::str(cell.text);
    return py::float_(cell.defined ? cell.value : std::numeric_limits<double>::quiet_NaN());
}

} // namespace

void ThermoFun::exportBatch(py::module& m)
{
    py::register_exception<BatchSetupError>(m, "BatchSetupError", PyExc_RuntimeError);

    m.def("selectSubstances", [](const Database& database, const py::object& rule) { return selectSubstances(database, textOfRule(rule)); }, py::arg("database"), py::arg("rule"),
          "The substances of a database that a rule chooses, for example {\"class\": \"aqueous_solute\", \"elements\": [\"Ca\"]}");
    m.def("selectReactions", [](const Database& database, const py::object& rule) { return selectReactions(database, textOfRule(rule)); }, py::arg("database"), py::arg("rule"),
          "The reactions of a database that a rule chooses");

    py::class_<Provenance>(m, "Provenance", "How the results of a batch calculation were made (decision 0032)")
        .def_readonly("database", &Provenance::database)
        .def_readonly("templates", &Provenance::templates)
        .def_readonly("calculations", &Provenance::calculations, "For each engine: its label in compare, and its summary")
        .def_readonly("setup", &Provenance::setup, "The settings of the result, as YAML")
        .def("text", &Provenance::text, "Short lines of text: what goes at the top of an output")
        .def("__str__", &Provenance::text)
        .def("to_yaml", &Provenance::toYaml, "All of it as YAML");

    py::class_<ResultCell>(m, "ResultCell", "One value of a result table")
        .def_property_readonly("value", &cellValue, "The number in the unit of its column, or the status")
        .def_readonly("defined", &ResultCell::defined)
        .def_property_readonly("result", [](const ResultCell& c) { return c.has_result ? py::cast(c.result) : py::object(py::none()); }, "The result, in the units of the engine");

    py::class_<ResultTable>(m, "ResultTable", "One result table of a batch calculation")
        .def_property_readonly("name", &ResultTable::name)
        .def_property_readonly("output", &ResultTable::output)
        .def_property_readonly("provenance", &ResultTable::provenance, "How the result was made")
        .def_property_readonly("columns", [](const ResultTable& t) {
            py::list out;
            for (const auto& c : t.columns())
            {
                py::dict row;
                row["property"] = c.property; row["label"] = c.label; row["show"] = c.show; row["unit"] = c.unit;
                out.append(row);
            }
            return out;
        }, "The columns: property, label, show, unit")
        .def_property_readonly("rows", [](const ResultTable& t) {
            py::list out;
            for (const auto& r : t.rows())
                out.append(py::make_tuple(r.record, r.T, pressureObject(r.P, r.saturation), r.cells));
            return out;
        }, "The rows: (record, T, P, cells), with T and P as in the setup")
        .def_property_readonly("problems", [](const ResultTable& t) {
            py::list out;
            for (const auto& p : t.problems())
                out.append(py::make_tuple(p.record, p.has_point ? py::object(py::float_(p.T)) : py::object(py::none()), p.has_point ? pressureObject(p.P, p.saturation) : py::object(py::none()), p.message));
            return out;
        }, "What could not be calculated: (record, T, P, message)")
        .def("values", &ResultTable::values, "The rows of numbers, in the units of the columns; NaN if not defined")
        .def("value", [](const ResultTable& t, const std::string& record, double T, const py::object& P, const std::string& column) {
            bool saturation = false;
            const double p = pressureOf(P, saturation);
            return cellValue(t.cell(record, T, p, column, saturation));
        }, py::arg("record"), py::arg("T"), py::arg("P"), py::arg("column"), "One number, in the unit of its column. P can be \"saturation\"")
        .def("pressure_used", [](const ResultTable& t, const std::string& record, double T, const py::object& P) {
            bool saturation = false;
            const double p = pressureOf(P, saturation);
            return t.pressureUsed(record, T, p, saturation);
        }, py::arg("record"), py::arg("T"), py::arg("P"), "The pressure that the calculation of a row used, in the unit of the conditions. P can be \"saturation\": then the saturation pressure at T")
        .def("scalar", [](const ResultTable& t, const std::string& record, double T, const py::object& P, const std::string& column) {
            bool saturation = false;
            const double p = pressureOf(P, saturation);
            return t.cell(record, T, p, column, saturation).result;
        }, py::arg("record"), py::arg("T"), py::arg("P"), py::arg("column"), "One result with its derivatives, uncertainty and status, in the units of the engine")
        .def("grid", &ResultTable::lines, "The header and the rows as text, in the layout long or wide")
        .def("property_grids", &ResultTable::propertyGrids, "One grid for each column: (label, lines)")
        .def("to_text", &ResultTable::toText, py::arg("format") = "csv", py::arg("with_provenance") = false, "The table as text: csv, md or tex")
        .def("write", &ResultTable::write, py::arg("path") = "", "Writes to a path, or to output.file of the setup. Gives the path");

    py::class_<BatchResults>(m, "BatchResultsCore", "The result tables of a batch setup")
        .def("tables", &BatchResults::tables);

    py::class_<BatchCalculation>(m, "BatchCalculation", "One result of a setup as a calculation of ThermoBatch, the 0.x batch API")
        .def_property_readonly("batch", &BatchCalculation::batch, py::return_value_policy::reference_internal, "The ThermoBatch, with its engine, units, digits and preferences from the setup")
        .def_property_readonly("symbols", &BatchCalculation::symbols)
        .def_property_readonly("properties", &BatchCalculation::properties)
        .def_property_readonly("points", &BatchCalculation::points, "The T-P points, in the units of the setup. P = 0 is the saturation pressure")
        .def_property_readonly("for_reactions", &BatchCalculation::forReactions)
        .def_property_readonly("provenance", &BatchCalculation::provenance)
        .def("run", &BatchCalculation::run, "Calculates, and gives the Output of ThermoBatch")
        .def("write", &BatchCalculation::write, py::arg("path") = "", "Calculates, and writes in the layout of the setup. Gives the path");

    py::class_<BatchSetup>(m, "BatchSetupCore", "A batch setup (decision 0032); thermofun.BatchSetup wraps it")
        .def(py::init<>())
        .def_static("fromText", &BatchSetup::fromText, "From YAML or JSON text")
        .def_static("fromFile", &BatchSetup::fromFile, "From a file")
        .def_static("fromTemplate", &BatchSetup::fromTemplate, "From a batch template")
        .def_static("templates", &BatchSetup::templates, "The batch templates: name and what it makes")
        .def_static("templateText", &BatchSetup::templateText, "The YAML text of a template")
        .def_static("templateSource", &BatchSetup::templateSource, "Where a template comes from: the path of its file, or library")
        .def_static("templateFolders", &BatchSetup::templateFolders, "The folders of THERMOFUN_TEMPLATE_PATH")
        .def("useTemplate", &BatchSetup::useTemplate, py::arg("name"), "Puts a template under the settings")
        .def("setDatabase", py::overload_cast<const Database&>(&BatchSetup::setDatabase))
        .def("setDatabase", py::overload_cast<const std::string&>(&BatchSetup::setDatabase))
        .def("setSubstances", &BatchSetup::setSubstances, py::arg("symbols"), py::arg("result") = "")
        .def("setReactions", &BatchSetup::setReactions, py::arg("symbols"), py::arg("result") = "")
        .def("setSubstanceRule", [](BatchSetup& s, const py::object& rule, const std::string& result) -> BatchSetup& { return s.setSubstanceRule(textOfRule(rule), result); }, py::arg("rule"), py::arg("result") = "")
        .def("setReactionRule", [](BatchSetup& s, const py::object& rule, const std::string& result) -> BatchSetup& { return s.setReactionRule(textOfRule(rule), result); }, py::arg("rule"), py::arg("result") = "")
        .def("setTemperatures", py::overload_cast<const std::vector<double>&, const std::string&, const std::string&>(&BatchSetup::setTemperatures), py::arg("values"), py::arg("unit") = "K", py::arg("result") = "")
        .def("setTemperatures", py::overload_cast<double, double, double, const std::string&, const std::string&>(&BatchSetup::setTemperatures), py::arg("first"), py::arg("last"), py::arg("step"), py::arg("unit") = "K", py::arg("result") = "")
        .def("setPressures", py::overload_cast<const std::vector<double>&, const std::string&, const std::string&>(&BatchSetup::setPressures), py::arg("values"), py::arg("unit") = "Pa", py::arg("result") = "")
        .def("setPressures", py::overload_cast<double, double, double, const std::string&, const std::string&>(&BatchSetup::setPressures), py::arg("first"), py::arg("last"), py::arg("step"), py::arg("unit") = "Pa", py::arg("result") = "")
        .def("setSaturationPressure", &BatchSetup::setSaturationPressure, py::arg("result") = "")
        .def("setLayout", &BatchSetup::setLayout, py::arg("layout"), py::arg("result") = "")
        .def("setOutputFile", &BatchSetup::setOutputFile, py::arg("path"), py::arg("result") = "")
        .def("setProperty", &BatchSetup::setProperty, py::arg("property"), py::arg("unit"), py::arg("digits") = -1, py::arg("result") = "")
        .def("addProperty", &BatchSetup::addProperty, py::arg("property"), py::arg("unit") = "", py::arg("digits") = -1, py::arg("result") = "")
        .def("removeProperty", &BatchSetup::removeProperty, py::arg("property"), py::arg("result") = "")
        .def("changeProperty", &BatchSetup::changeProperty, py::arg("property"), py::arg("json"), py::arg("result") = "")
        .def("addPropertyFromJson", &BatchSetup::addPropertyFromJson, py::arg("json"), py::arg("result") = "")
        .def("setJson", &BatchSetup::set, py::arg("path"), py::arg("json"), "Any value by its path (a JSON pointer), as JSON text")
        .def("getJson", &BatchSetup::get, py::arg("path"), "The value at a path, as JSON text")
        .def("toYaml", &BatchSetup::toYaml)
        .def("save", &BatchSetup::save)
        .def("runCore", &BatchSetup::run)
        .def("batch", &BatchSetup::batch, py::arg("result") = "", "One result as a calculation of ThermoBatch")
        .def("applyTo", &BatchSetup::applyTo, py::arg("batch"), py::arg("result") = "", "Sets the units, digits and preferences of a ThermoBatch from one result");
}
