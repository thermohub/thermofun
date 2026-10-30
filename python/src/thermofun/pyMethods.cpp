// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

#include <optional>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <ThermoFun/Methods/Evaluate.h>
#include <ThermoFun/Methods/Minerals/ConstantCp.h>
#include <ThermoFun/Methods/Minerals/ConstantProperties.h>
#include <ThermoFun/Methods/Minerals/CpPolynomial.h>

namespace py = pybind11;

namespace ThermoFun {

namespace {

using namespace v2;

auto makeReference(py::object T, py::object P, py::object G, py::object H, py::object S, py::object Cp, py::object V) -> ReferenceValues
{
    auto number = [](const py::object& x) -> std::optional<double> {
        if (x.is_none())
            return std::nullopt;
        return x.cast<double>();
    };
    ReferenceValues r;
    if (!T.is_none())
        r.T = T.cast<double>();
    if (!P.is_none())
        r.P = P.cast<double>();
    r.G = number(G);
    r.H = number(H);
    r.S = number(S);
    r.Cp = number(Cp);
    r.V = number(V);
    return r;
}

} // namespace

void exportMethods(py::module& parent)
{
    auto m = parent.def_submodule("methods", "The method objects: each one runs alone, with no database and no engine. Values are in SI.");

    py::class_<PropertyValue>(m, "PropertyValue", "A property with its derivatives with respect to T and P")
        .def_readonly("val", &PropertyValue::val)
        .def_readonly("ddt", &PropertyValue::ddt)
        .def_readonly("ddp", &PropertyValue::ddp);

    py::class_<MethodResult>(m, "MethodResult", "The properties of one method applied alone: G, H [J/mol], S, Cp [J/(mol K)], V [m3/mol]")
        .def_readonly("G", &MethodResult::G)
        .def_readonly("H", &MethodResult::H)
        .def_readonly("S", &MethodResult::S)
        .def_readonly("Cp", &MethodResult::Cp)
        .def_readonly("V", &MethodResult::V)
        .def_readonly("outside_range", &MethodResult::outside_range);

    py::class_<ReferenceValues>(m, "Reference", "The reference state of a substance, in SI; a value that is None is not given")
        .def(py::init(&makeReference), py::arg("T") = py::none(), py::arg("P") = py::none(), py::arg("G") = py::none(),
             py::arg("H") = py::none(), py::arg("S") = py::none(), py::arg("Cp") = py::none(), py::arg("V") = py::none())
        .def_readwrite("T", &ReferenceValues::T)
        .def_readwrite("P", &ReferenceValues::P)
        .def_readwrite("G", &ReferenceValues::G)
        .def_readwrite("H", &ReferenceValues::H)
        .def_readwrite("S", &ReferenceValues::S)
        .def_readwrite("Cp", &ReferenceValues::Cp)
        .def_readwrite("V", &ReferenceValues::V);

    py::class_<PhaseTransition>(m, "PhaseTransition", "A phase transition: T [K], dS [J/(mol K)], dH [J/mol], dV [m3/mol]")
        .def(py::init([](double T, double dS, double dH, double dV) { return PhaseTransition{T, dS, dH, dV}; }), py::arg("T"), py::arg("dS") = 0.0, py::arg("dH") = 0.0, py::arg("dV") = 0.0);

    py::class_<SubstanceMethod, std::shared_ptr<SubstanceMethod>>(m, "SubstanceMethod", "A method of a substance, bound to its parameters")
        .def_property_readonly("id", [](const SubstanceMethod& s) { return s.spec().id; })
        .def_property_readonly("summary", [](const SubstanceMethod& s) { return s.spec().summary; })
        .def("evaluate", [](const SubstanceMethod& s, double T, double P, const ReferenceValues& r) { return evaluate(s, T, P, r); },
             "The properties at T [K], P [Pa], with their derivatives", py::arg("T"), py::arg("P"), py::arg("reference"))
        .def("parameterDerivative", [](const SubstanceMethod& s, std::size_t i, double T, double P, const ReferenceValues& r) {
                 auto d = evaluateParameterDerivative(s, i, T, P, r);
                 py::dict out;
                 out["G"] = d.G.ddt; out["H"] = d.H.ddt; out["S"] = d.S.ddt; out["Cp"] = d.Cp.ddt; out["V"] = d.V.ddt;
                 return out;
             }, "The derivatives of the properties with respect to one parameter", py::arg("index"), py::arg("T"), py::arg("P"), py::arg("reference"))
        .def("parameterValues", &SubstanceMethod::parameterValues, "The parameters in SI")
        .def("withParameter", [](const SubstanceMethod& s, std::size_t i, double v) { return std::const_pointer_cast<SubstanceMethod>(s.withParameter(i, v)); },
             "A copy with one parameter changed (SI)", py::arg("index"), py::arg("value"));

    py::class_<ConstantProperties, SubstanceMethod, std::shared_ptr<ConstantProperties>>(m, "ConstantProperties", "The reference properties at every T and P")
        .def(py::init<>());
    py::class_<ConstantCp, SubstanceMethod, std::shared_ptr<ConstantCp>>(m, "ConstantCp", "Integrates the constant reference Cp")
        .def(py::init<>());
    py::class_<CpPolynomial, SubstanceMethod, std::shared_ptr<CpPolynomial>>(m, "CpPolynomial", "Integrates a Cp polynomial over T segments")
        .def(py::init([](std::vector<std::pair<double, double>> intervals, std::vector<double> coefficients, std::vector<PhaseTransition> transitions) {
                 return CpPolynomial(std::move(intervals), std::move(coefficients), std::move(transitions));
             }),
             "intervals: (lower, upper) in K for each segment; coefficients: a0 to a9 of the segments, one after the other",
             py::arg("intervals") = std::vector<std::pair<double, double>>{}, py::arg("coefficients") = std::vector<double>{}, py::arg("transitions") = std::vector<PhaseTransition>{});
}

} // namespace ThermoFun
