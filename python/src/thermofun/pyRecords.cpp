// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include <ThermoFun/Model/Records.h>

namespace ThermoFun {

void exportRecords(py::module& m)
{
    using namespace v2;

    // words: the names SubstanceClass and AggregateState are the enums of the old API
    auto wordOf = [](auto value) { return toString(value); };

    py::class_<CovarianceBlock>(m, "CovarianceBlock", "The covariance of some values of a record, in their SI units")
        .def(py::init<>())
        .def_readwrite("paths", &CovarianceBlock::paths)
        .def_readwrite("matrix", &CovarianceBlock::matrix)
        .def(py::self == py::self);

    py::class_<Convention>(m, "Convention", "Conventions of a dataset or a record")
        .def(py::init<>())
        .def_readwrite("energy_reference", &Convention::energy_reference)
        .def_readwrite("standard_pressure", &Convention::standard_pressure)
        .def_readwrite("concentration_scale", &Convention::concentration_scale)
        .def_readwrite("water_reference", &Convention::water_reference)
        .def(py::self == py::self);

    py::class_<ReferenceConditions>(m, "ReferenceConditions", "Reference temperature (K) and pressure (Pa)")
        .def(py::init<>())
        .def_readwrite("T", &ReferenceConditions::T)
        .def_readwrite("P", &ReferenceConditions::P)
        .def(py::self == py::self);

    py::class_<Applicability>(m, "Applicability", "Range of T (K) and P (Pa) of a record or a method")
        .def(py::init<>())
        .def_readwrite("t_min", &Applicability::t_min)
        .def_readwrite("t_max", &Applicability::t_max)
        .def_readwrite("p_min", &Applicability::p_min)
        .def_readwrite("p_max", &Applicability::p_max)
        .def_readwrite("is_hard", &Applicability::is_hard)
        .def(py::self == py::self);

    py::class_<Segment>(m, "Segment", "A piece of a method that is piecewise in T")
        .def(py::init<>())
        .def_readwrite("t_min", &Segment::t_min)
        .def_readwrite("t_max", &Segment::t_max)
        .def_readwrite("parameters", &Segment::parameters)
        .def(py::self == py::self);

    py::class_<TableData>(m, "TableData", "A table of property values on a grid of T and P (SI)")
        .def(py::init<>())
        .def_readwrite("temperatures", &TableData::temperatures)
        .def_readwrite("pressures", &TableData::pressures)
        .def_readwrite("values", &TableData::values)
        .def_readwrite("reference_pressure", &TableData::reference_pressure)
        .def(py::self == py::self);

    py::class_<MethodApplication>(m, "MethodApplication", "The use of one method on one record")
        .def(py::init<>())
        .def_readwrite("method", &MethodApplication::method)
        .def_readwrite("method_as_written", &MethodApplication::method_as_written)
        .def_readwrite("implementation", &MethodApplication::implementation)
        .def_readwrite("interpolation_order", &MethodApplication::interpolation_order)
        .def_readwrite("parameters", &MethodApplication::parameters)
        .def_readwrite("parameter_set", &MethodApplication::parameter_set)
        .def_readwrite("applicability", &MethodApplication::applicability)
        .def_readwrite("segments", &MethodApplication::segments)
        .def_readwrite("transitions", &MethodApplication::transitions)
        .def_readwrite("table", &MethodApplication::table)
        .def_readwrite("extra_json", &MethodApplication::extra_json)
        .def(py::self == py::self)
        .def("__repr__", [](const MethodApplication& a) { return "MethodApplication(method='" + a.method + "')"; });

    py::class_<SubstanceRecord>(m, "SubstanceRecord", "What a data file says about a substance")
        .def(py::init<>())
        .def_readwrite("symbol", &SubstanceRecord::symbol)
        .def_property("class_", [wordOf](const SubstanceRecord& r) { return wordOf(r.class_); }, [](SubstanceRecord& r, const std::string& w) { r.class_ = fromString<v2::SubstanceClass>(w).value_or(v2::SubstanceClass::Other); })
        .def_readwrite("sourcetdb", &SubstanceRecord::sourcetdb)
        .def_readwrite("name", &SubstanceRecord::name)
        .def_readwrite("formula", &SubstanceRecord::formula)
        .def_readwrite("charge", &SubstanceRecord::charge)
        .def_property("aggregate_state", [wordOf](const SubstanceRecord& r) { return wordOf(r.aggregate_state); }, [](SubstanceRecord& r, const std::string& w) { r.aggregate_state = fromString<v2::AggregateState>(w).value_or(v2::AggregateState::Other); })
        .def_readwrite("molar_mass", &SubstanceRecord::molar_mass)
        .def_readwrite("standard_state_convention", &SubstanceRecord::standard_state_convention)
        .def_readwrite("convention", &SubstanceRecord::convention)
        .def_readwrite("reference_conditions", &SubstanceRecord::reference_conditions)
        .def_readwrite("standard_state_properties", &SubstanceRecord::standard_state_properties)
        .def_readwrite("methods", &SubstanceRecord::methods)
        .def_readwrite("defining_reaction", &SubstanceRecord::defining_reaction)
        .def_readwrite("applicability", &SubstanceRecord::applicability)
        .def_readwrite("preferred_property_source", &SubstanceRecord::preferred_property_source)
        .def_readwrite("covariance", &SubstanceRecord::covariance)
        .def_readwrite("structural", &SubstanceRecord::structural)
        .def_readwrite("data_source", &SubstanceRecord::data_source)
        .def_readwrite("data_sources", &SubstanceRecord::data_sources)
        .def_readwrite("deprecated", &SubstanceRecord::deprecated)
        .def_readwrite("deprecated_in_favour_of", &SubstanceRecord::deprecated_in_favour_of)
        .def_readwrite("extra_json", &SubstanceRecord::extra_json)
        .def_readwrite("source_format", &SubstanceRecord::source_format)
        .def_readwrite("original_text", &SubstanceRecord::original_text)
        .def(py::self == py::self)
        .def("__repr__", [](const SubstanceRecord& r) { return "SubstanceRecord(symbol='" + r.symbol + "', class='" + toString(r.class_) + "')"; });

    py::class_<ReactionRecord>(m, "ReactionRecord", "What a data file says about a reaction")
        .def(py::init<>())
        .def_readwrite("symbol", &ReactionRecord::symbol)
        .def_readwrite("sourcetdb", &ReactionRecord::sourcetdb)
        .def_readwrite("level", &ReactionRecord::level)
        .def_readwrite("name", &ReactionRecord::name)
        .def_readwrite("equation", &ReactionRecord::equation)
        .def_readwrite("stoichiometry", &ReactionRecord::stoichiometry)
        .def_readwrite("reference_conditions", &ReactionRecord::reference_conditions)
        .def_readwrite("reaction_properties", &ReactionRecord::reaction_properties)
        .def_readwrite("methods", &ReactionRecord::methods)
        .def_readwrite("applicability", &ReactionRecord::applicability)
        .def_readwrite("convention", &ReactionRecord::convention)
        .def_readwrite("preferred_property_source", &ReactionRecord::preferred_property_source)
        .def_readwrite("covariance", &ReactionRecord::covariance)
        .def_readwrite("data_source", &ReactionRecord::data_source)
        .def_readwrite("data_sources", &ReactionRecord::data_sources)
        .def_readwrite("deprecated", &ReactionRecord::deprecated)
        .def_readwrite("extra_json", &ReactionRecord::extra_json)
        .def_readwrite("source_format", &ReactionRecord::source_format)
        .def_readwrite("original_text", &ReactionRecord::original_text)
        .def(py::self == py::self)
        .def("__repr__", [](const ReactionRecord& r) { return "ReactionRecord(symbol='" + r.symbol + "')"; });

    py::class_<ElementRecord>(m, "ElementRecord", "What a data file says about an element")
        .def(py::init<>())
        .def_readwrite("symbol", &ElementRecord::symbol)
        .def_readwrite("name", &ElementRecord::name)
        .def_property("class_", [wordOf](const ElementRecord& r) { return wordOf(r.class_); }, [](ElementRecord& r, const std::string& w) { r.class_ = fromString<ElementClass>(w).value_or(ElementClass::Other); })
        .def_readwrite("isotope_mass", &ElementRecord::isotope_mass)
        .def_readwrite("number", &ElementRecord::number)
        .def_readwrite("valence", &ElementRecord::valence)
        .def_readwrite("molar_mass", &ElementRecord::molar_mass)
        .def_readwrite("standard_entropy", &ElementRecord::standard_entropy)
        .def_readwrite("heat_capacity", &ElementRecord::heat_capacity)
        .def_readwrite("volume", &ElementRecord::volume)
        .def_readwrite("reference_form", &ElementRecord::reference_form)
        .def_readwrite("structural", &ElementRecord::structural)
        .def_readwrite("data_sources", &ElementRecord::data_sources)
        .def_readwrite("sourcetdb", &ElementRecord::sourcetdb)
        .def_readwrite("extra_json", &ElementRecord::extra_json)
        .def_readwrite("source_format", &ElementRecord::source_format)
        .def_readwrite("original_text", &ElementRecord::original_text)
        .def(py::self == py::self)
        .def("__repr__", [](const ElementRecord& r) { return "ElementRecord(symbol='" + r.symbol + "')"; });

    py::class_<DataSource>(m, "DataSource", "A literature source")
        .def(py::init<>())
        .def_readwrite("symbol", &DataSource::symbol)
        .def_readwrite("title", &DataSource::title)
        .def_readwrite("authors", &DataSource::authors)
        .def_readwrite("journal", &DataSource::journal)
        .def_readwrite("doi", &DataSource::doi)
        .def_readwrite("bibtex", &DataSource::bibtex)
        .def_readwrite("year", &DataSource::year)
        .def_readwrite("extra_json", &DataSource::extra_json)
        .def(py::self == py::self);

    py::class_<ParameterDefinition>(m, "ParameterDefinition", "A parameter of a defined method")
        .def(py::init<>())
        .def_readwrite("name", &ParameterDefinition::name)
        .def_readwrite("unit", &ParameterDefinition::unit)
        .def_readwrite("description", &ParameterDefinition::description)
        .def_readwrite("required", &ParameterDefinition::required)
        .def_readwrite("default_value", &ParameterDefinition::default_value)
        .def(py::self == py::self);

    py::class_<ReferenceTestCase>(m, "ReferenceTestCase", "A test case of a defined method")
        .def(py::init<>())
        .def_readwrite("T", &ReferenceTestCase::T)
        .def_readwrite("P", &ReferenceTestCase::P)
        .def_readwrite("parameters", &ReferenceTestCase::parameters)
        .def_readwrite("expected", &ReferenceTestCase::expected)
        .def_readwrite("relative_tolerance", &ReferenceTestCase::relative_tolerance)
        .def(py::self == py::self);

    py::class_<MethodDefinition>(m, "MethodDefinition", "A method that a dataset defines")
        .def(py::init<>())
        .def_readwrite("symbol", &MethodDefinition::symbol)
        .def_property_readonly("kind", [wordOf](const MethodDefinition& x) { return wordOf(x.kind); })
        .def_readwrite("tier", &MethodDefinition::tier)
        .def_property_readonly("operates_on", [wordOf](const MethodDefinition& x) { return wordOf(x.operates_on); })
        .def_readwrite("contributes_to", &MethodDefinition::contributes_to)
        .def_readwrite("parameter_definitions", &MethodDefinition::parameter_definitions)
        .def_readwrite("implementation_id", &MethodDefinition::implementation_id)
        .def_readwrite("source_form", &MethodDefinition::source_form)
        .def_readwrite("source_expression", &MethodDefinition::source_expression)
        .def_readwrite("source_segments", &MethodDefinition::source_segments)
        .def_readwrite("expression", &MethodDefinition::expression)
        .def_readwrite("expression_segments", &MethodDefinition::expression_segments)
        .def_readwrite("generator", &MethodDefinition::generator)
        .def_readwrite("source_hash", &MethodDefinition::source_hash)
        .def_readwrite("tests", &MethodDefinition::tests)
        .def_readwrite("effects_json", &MethodDefinition::effects_json)
        .def_readwrite("citation", &MethodDefinition::citation)
        .def_readwrite("extra_json", &MethodDefinition::extra_json)
        .def(py::self == py::self);

    py::class_<MethodParameterSet>(m, "MethodParameterSet", "Named values of the parameters of a method")
        .def(py::init<>())
        .def_readwrite("symbol", &MethodParameterSet::symbol)
        .def_readwrite("method", &MethodParameterSet::method)
        .def_readwrite("parameters", &MethodParameterSet::parameters)
        .def_readwrite("auxiliary", &MethodParameterSet::auxiliary)
        .def_readwrite("reference", &MethodParameterSet::reference)
        .def_readwrite("extra_json", &MethodParameterSet::extra_json)
        .def(py::self == py::self);

    py::class_<ThermoDataSetInfo>(m, "ThermoDataSetInfo", "Data about the loaded dataset")
        .def(py::init<>())
        .def_readwrite("symbol", &ThermoDataSetInfo::symbol)
        .def_readwrite("name", &ThermoDataSetInfo::name)
        .def_readwrite("version", &ThermoDataSetInfo::version)
        .def_property_readonly("status", [wordOf](const ThermoDataSetInfo& x) { return wordOf(x.status); })
        .def_readwrite("schema_version", &ThermoDataSetInfo::schema_version)
        .def_readwrite("standard_state_convention", &ThermoDataSetInfo::standard_state_convention)
        .def_readwrite("convention", &ThermoDataSetInfo::convention)
        .def_readwrite("reference_conditions", &ThermoDataSetInfo::reference_conditions)
        .def_readwrite("sourcetdb", &ThermoDataSetInfo::sourcetdb)
        .def_readwrite("solvent", &ThermoDataSetInfo::solvent)
        .def_readwrite("defaults", &ThermoDataSetInfo::defaults)
        .def_readwrite("default_data_source", &ThermoDataSetInfo::default_data_source)
        .def_readwrite("water_triple_point_reference", &ThermoDataSetInfo::water_triple_point_reference)
        .def_readwrite("legacy_datasources", &ThermoDataSetInfo::legacy_datasources)
        .def_readwrite("legacy_date", &ThermoDataSetInfo::legacy_date)
        .def_readwrite("extra_json", &ThermoDataSetInfo::extra_json)
        .def(py::self == py::self);
}

} // namespace ThermoFun
