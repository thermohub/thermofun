// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

#include <sstream>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include <ThermoFun/Model/Records.h>

namespace ThermoFun {

void exportQuantity(py::module& m)
{
    using namespace v2;

    py::enum_<PropertyName>(m, "PropertyName", "Standard-state property of a substance")
        .value("G", PropertyName::G).value("H", PropertyName::H).value("S", PropertyName::S)
        .value("Cp", PropertyName::Cp).value("V", PropertyName::V).value("U", PropertyName::U)
        .value("A", PropertyName::A).value("Cv", PropertyName::Cv);

    py::enum_<ReactionPropertyName>(m, "ReactionPropertyName", "Standard-state property of a reaction")
        .value("logK", ReactionPropertyName::logK).value("lnK", ReactionPropertyName::lnK)
        .value("dG", ReactionPropertyName::dG).value("dH", ReactionPropertyName::dH)
        .value("dS", ReactionPropertyName::dS).value("dCp", ReactionPropertyName::dCp)
        .value("dV", ReactionPropertyName::dV).value("dU", ReactionPropertyName::dU)
        .value("dA", ReactionPropertyName::dA);

    py::enum_<DataFormat>(m, "DataFormat", "Format of a data file")
        .value("Auto", DataFormat::Auto).value("Legacy", DataFormat::Legacy).value("V2", DataFormat::V2);

    py::enum_<Uncertainty::Kind>(m, "UncertaintyKind", "Origin of an uncertainty")
        .value("Primary", Uncertainty::Kind::Primary).value("Derived", Uncertainty::Kind::Derived);

    py::class_<Uncertainty>(m, "Uncertainty", "Standard uncertainty of a value, in the SI unit of the value")
        .def(py::init<>())
        .def_readwrite("standard", &Uncertainty::standard)
        .def_readwrite("coverage_factor_as_written", &Uncertainty::coverage_factor_as_written)
        .def_readwrite("kind", &Uncertainty::kind)
        .def_readwrite("distribution", &Uncertainty::distribution)
        .def_readwrite("degrees_of_freedom", &Uncertainty::degrees_of_freedom)
        .def_readwrite("sources", &Uncertainty::sources)
        .def_readwrite("snapshot", &Uncertainty::snapshot)
        .def_readwrite("convention_unknown", &Uncertainty::convention_unknown)
        .def(py::self == py::self);

    py::class_<ValueMeta>(m, "ValueMeta", "Quality, origin and source of a value")
        .def(py::init<>())
        .def_property("quality", [](const ValueMeta& x) { return x.quality ? toString(*x.quality) : std::string(); },
                      [](ValueMeta& x, const std::string& word) { x.quality = word.empty() ? std::nullopt : fromString<Quality>(word); })
        .def_property("origin", [](const ValueMeta& x) { return x.origin ? toString(*x.origin) : std::string(); },
                      [](ValueMeta& x, const std::string& word) { x.origin = word.empty() ? std::nullopt : fromString<Origin>(word); })
        .def_readwrite("calculation_path", &ValueMeta::calculation_path)
        .def_readwrite("source", &ValueMeta::source)
        .def(py::self == py::self);

    py::class_<Quantity>(m, "Quantity", "A value in the SI unit of its slot, with its unit as written and its uncertainty")
        .def(py::init<>())
        .def_readwrite("value", &Quantity::value)
        .def_readwrite("unit_as_written", &Quantity::unit_as_written)
        .def_readwrite("value_as_written", &Quantity::value_as_written)
        .def_readwrite("uncertainty", &Quantity::uncertainty)
        .def_readwrite("meta", &Quantity::meta)
        .def("isBare", &Quantity::isBare, "True if the quantity has only a value in the default unit")
        .def(py::self == py::self)
        .def("__repr__", [](const Quantity& q) {
            std::ostringstream s;
            s << "Quantity(value=" << q.value;
            if (!q.unit_as_written.empty())
                s << ", unit_as_written='" << q.unit_as_written << "'";
            if (q.uncertainty)
                s << ", standard_uncertainty=" << q.uncertainty->standard;
            s << ")";
            return s.str();
        });
}

} // namespace ThermoFun
