// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The builders of v2 records (thermofun.records): keyword arguments of Python become the fields; a keyword with a
// trailing "_" (class_) is the field without it.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include <ThermoFun/RecordBuilder.h>

namespace ThermoFun {

namespace {

auto value(const py::handle& object) -> RecordValue
{
    return RecordValue::json(py::module_::import("json").attr("dumps")(object).cast<std::string>());
}

auto key(std::string name) -> std::string
{
    while (!name.empty() && name.back() == '_')
        name.pop_back();
    return name;
}

auto fields(const py::dict& items) -> RecordFields
{
    RecordFields out;
    for (const auto& [k, v] : items)
        out.emplace_back(key(py::str(k)), value(v));
    return out;
}

auto segment(const py::handle& object) -> RecordSegment
{
    py::dict s(py::module_::import("builtins").attr("dict")(object));
    RecordSegment out;
    if (s.contains("t_min"))
        out.t_min = value(s.attr("pop")("t_min"));
    if (s.contains("t_max"))
        out.t_max = value(s.attr("pop")("t_max"));
    py::object explicitParameters = s.attr("pop")("parameters", py::none());
    out.parameters = fields(explicitParameters.is_none() ? s : py::dict(explicitParameters));
    return out;
}

auto truthy(const py::object& object) -> bool
{
    return !object.is_none() && py::bool_(object);
}

auto builder(RecordBuilder (*make)(const std::string&, const RecordFields&))
{
    return [make](const std::string& symbol, const py::kwargs& kwargs) { return make(symbol, fields(kwargs)); };
}

} // namespace

void exportRecordBuilder(py::module& m)
{
    py::class_<RecordBuilder>(m, "RecordBuilder",
                              "Builds a v2 record. A value is a number, a text \"number unit\", or a dictionary {value, unit, uncertainty}: "
                              "the same forms as in a file")
        .def_static("element", builder(&RecordBuilder::element), py::arg("symbol"), "An element record: number, molar_mass, class_, ...")
        .def_static("substance", builder(&RecordBuilder::substance), py::arg("symbol"), "A substance record: formula, class_, aggregate_state, charge, name, ...")
        .def_static("reaction", builder(&RecordBuilder::reaction), py::arg("symbol"), "A reaction record: equation or stoichiometry, name, ...")
        .def_property_readonly("kind", &RecordBuilder::kind, "element, substance or reaction")
        .def_property_readonly("section", &RecordBuilder::section, "elements, substances or reactions")
        .def_property_readonly("symbol", &RecordBuilder::symbol)
        .def("set", [](RecordBuilder& self, const py::kwargs& kwargs) -> RecordBuilder& { return self.set(fields(kwargs)); },
             py::return_value_policy::reference_internal, "Sets other fields of the record")
        .def("standard_state", [](RecordBuilder& self, const py::kwargs& kwargs) -> RecordBuilder& {
                 for (const auto& [name, v] : fields(kwargs))
                     self.standardState(name, v);
                 return self;
             }, py::return_value_policy::reference_internal, "The standard-state properties G, H, S, Cp, V")
        .def("properties", [](RecordBuilder& self, const py::kwargs& kwargs) -> RecordBuilder& {
                 for (const auto& [name, v] : fields(kwargs))
                     self.properties(name, v);
                 return self;
             }, py::return_value_policy::reference_internal, "The reaction properties logK, dG, dH, dS, dCp, dV")
        .def("method", [](RecordBuilder& self, const std::string& name, const py::object& segments, const py::object& parameters,
                          const py::object& parameter_set, const py::object& applicability, const py::object& table,
                          const py::object& transitions, const py::object& implementation, const py::kwargs& values) -> RecordBuilder& {
                 RecordMethod m;
                 m.method = name;
                 if (truthy(implementation))
                     m.implementation = implementation.cast<std::string>();
                 py::dict all;
                 if (!parameters.is_none())
                     all = py::dict(parameters);
                 for (const auto& [k, v] : values)
                     all[k] = v;
                 m.parameters = fields(all);
                 if (!segments.is_none())
                 {
                     m.segments.emplace();
                     for (const auto& s : segments)
                         m.segments->push_back(segment(s));
                 }
                 if (truthy(parameter_set))
                     m.parameter_set = parameter_set.cast<std::string>();
                 if (truthy(transitions))
                     m.transitions = value(transitions);
                 if (truthy(table))
                     m.table = value(table);
                 if (truthy(applicability))
                     m.applicability = value(applicability);
                 return self.method(m);
             }, py::arg("name"), py::arg("segments") = py::none(), py::arg("parameters") = py::none(), py::arg("parameter_set") = py::none(),
             py::arg("applicability") = py::none(), py::arg("table") = py::none(), py::arg("transitions") = py::none(),
             py::arg("implementation") = py::none(), py::return_value_policy::reference_internal,
             "Adds a method. The keywords that are not named are the parameters of the method")
        .def("to_json", &RecordBuilder::toJson, "The record as JSON text, with its symbol")
        .def("to_dict", [](const RecordBuilder& self) { return py::module_::import("json").attr("loads")(self.toJson()); }, "The record as a dictionary")
        .def("check", [](const RecordBuilder& self) { return self.check().all(); },
             "The problems of this record, now: a list of Diagnostic from the reader and from validate(). The record is alone here, "
             "so a reference to another record is not a problem")
        .def("record", [](const RecordBuilder& self) -> py::object {
                 if (self.kind() == "element")
                     return py::cast(self.elementRecord());
                 if (self.kind() == "substance")
                     return py::cast(self.substanceRecord());
                 return py::cast(self.reactionRecord());
             }, "The record, read")
        .def("__repr__", [](const RecordBuilder& self) { return "RecordBuilder(" + self.toJson() + ")"; });
}

} // namespace ThermoFun
