// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

#include <optional>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <ThermoFun/IO/Legacy/LegacyMethodMap.h>
#include <ThermoFun/Methods/Evaluate.h>
#include <ThermoFun/Methods/Minerals/ConstantCp.h>
#include <ThermoFun/Methods/Minerals/ConstantProperties.h>
#include <ThermoFun/Methods/Minerals/CpPolynomial.h>
#include <ThermoFun/Methods/MethodRegistry.h>
#include <ThermoFun/Methods/Water/SolventMethod.h>

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

auto makeApplication(const std::string& id, const std::string& implementation, const py::dict& parameters) -> MethodApplication
{
    MethodApplication a;
    a.method = id;
    a.implementation = implementation;
    for (const auto& item : parameters)
    {
        Quantity q;
        q.value = item.second.cast<double>();
        a.parameters[item.first.cast<std::string>()] = q;
    }
    return a;
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

    py::class_<SolventInputs>(m, "SolventInputs", "The state of the solvent for a method that needs it")
        .def(py::init<>())
        .def_readwrite("properties", &SolventInputs::properties)
        .def_readwrite("properties_at_reference", &SolventInputs::properties_at_reference)
        .def_readwrite("electro", &SolventInputs::electro)
        .def_readwrite("water", &SolventInputs::water)
        .def_readwrite("water_at_reference", &SolventInputs::water_at_reference)
        .def_readwrite("ideal_gas_water", &SolventInputs::ideal_gas_water)
        .def_readwrite("ideal_gas_water_at_reference", &SolventInputs::ideal_gas_water_at_reference);

    py::class_<SolventMethod, std::shared_ptr<SolventMethod>>(m, "SolventMethod", "An equation of state of water")
        .def_property_readonly("id", [](const SolventMethod& s) { return s.spec().id; })
        .def("properties", [](const SolventMethod& s, double T, double P, int state) { return s.properties(T, P, state); },
             "The properties of the solvent at T [K], P [Pa]. State: 0 liquid, 1 gas", py::arg("T"), py::arg("P"), py::arg("state") = 0)
        .def("substanceProperties", [](const SolventMethod& s, double T, double P, int state, const std::string& triple) { return s.substanceProperties(T, P, state, triple); },
             "The properties of water as a substance at T [K], P [Pa]", py::arg("T"), py::arg("P"), py::arg("state") = 0, py::arg("triple") = "NEA_HGK");

    py::class_<DielectricMethod, std::shared_ptr<DielectricMethod>>(m, "DielectricMethod", "A model of the dielectric constant of water")
        .def_property_readonly("id", [](const DielectricMethod& s) { return s.spec().id; })
        .def("properties", &DielectricMethod::properties, "The electrochemical properties at T [K], P [Pa], from the properties of the solvent",
             py::arg("T"), py::arg("P"), py::arg("solvent"), py::arg("state") = 0);

    py::class_<ReactionMethod, std::shared_ptr<ReactionMethod>>(m, "ReactionMethod", "A method of a reaction, bound to its parameters")
        .def_property_readonly("id", [](const ReactionMethod& s) { return s.spec().id; })
        .def("calculate", &ReactionMethod::calculate, "The properties at T [K], P [Pa], from those of the method before it. solvent: for a method that needs it",
             py::arg("T"), py::arg("P"), py::arg("incoming") = ThermoPropertiesReaction{}, py::arg("solvent") = nullptr);

    m.def("catalog", [] {
        py::list out;
        for (const auto& spec : MethodCatalog::instance().specs())
        {
            py::dict row;
            row["id"] = spec.id; row["aliases"] = spec.aliases; row["title"] = spec.title; row["summary"] = spec.summary;
            row["citation"] = spec.citation; row["operates_on"] = toString(spec.operates_on);
            row["base"] = spec.operates_on == OperatesOn::Substance ? spec.effects.base : spec.reaction_effects.base;
            row["piecewise"] = spec.piecewise; row["implemented"] = spec.implemented;
            row["water"] = spec.role == MethodRole::SolventProperties; row["dielectric"] = spec.role == MethodRole::SolventElectro;
            row["implementations"] = spec.implementations;
            row["needs_solvent"] = spec.needs.solvent;
            std::vector<std::string> classes, legacy_names;
            for (const auto c : spec.applies_to_classes)
                classes.push_back(toString(c));
            for (const auto& l : spec.operates_on == OperatesOn::Substance ? v2::legacy::legacySubstanceMethods() : v2::legacy::legacyReactionMethods())
                if (spec.id == l.method)
                    legacy_names.push_back(l.name);
            row["classes"] = classes;
            row["legacy_names"] = legacy_names;
            row["native_convention"] = spec.native_convention ? toString(*spec.native_convention) : std::string();
            py::list parameters;
            for (const auto& p : spec.parameters)
            {
                py::dict one;
                one["name"] = p.name; one["si_unit"] = p.si_unit; one["source_unit"] = p.source_unit; one["required"] = p.required;
                one["description"] = p.description;
                one["default"] = p.default_si ? py::cast(*p.default_si) : py::none();
                parameters.append(one);
            }
            row["parameters"] = parameters;
            out.append(row);
        }
        return out;
    }, "All built-in methods: id, title, what they need and their parameters");

    m.def("reactionMethod", [](const std::string& id, const py::dict& parameters, py::object temperatures, py::object logK, const py::dict& reference, int lagrangeOrder) {
            auto a = makeApplication(id, "", parameters);
            if (lagrangeOrder > 0)
            {
                a.interpolation = Interpolation::Lagrange;
                a.interpolation_order = lagrangeOrder;
            }
            if (!temperatures.is_none())
            {
                TableData table;
                table.temperatures = temperatures.cast<std::vector<double>>();
                table.values["logK"] = logK.cast<std::vector<double>>();
                a.table = table;
            }
            ReactionRecord record;
            for (const auto& item : reference)
            {
                const auto name = fromString<ReactionPropertyName>(item.first.cast<std::string>());
                if (!name)
                    throw py::value_error("Not a property of a reaction: " + item.first.cast<std::string>());
                Quantity q;
                q.value = item.second.cast<double>();
                record.reaction_properties[*name] = q;
            }
            return std::const_pointer_cast<ReactionMethod>(MethodRegistry::instance().createReaction(a, record));
        }, "A method of a reaction, for example reactionMethod(\"LogKTable\", temperatures=[...], logK=[...]). "
           "parameters in SI; lagrange_order above 0 for the Lagrange polynomial of LogKTable; reference: logK, dG, dH, dS, dCp, dV in SI.",
        py::arg("id"), py::arg("parameters") = py::dict(), py::arg("temperatures") = py::none(), py::arg("logK") = py::none(), py::arg("reference") = py::dict(), py::arg("lagrange_order") = 0);
    m.def("substanceMethod", [](const std::string& id, const std::string& implementation, const py::dict& parameters, const std::string& symbol, int charge) {
            SubstanceRecord record;
            record.symbol = symbol;
            record.charge = charge;
            return std::const_pointer_cast<SubstanceMethod>(MethodRegistry::instance().create(makeApplication(id, implementation, parameters), record));
        }, "A substance method from its id and its parameters in SI, for example substanceMethod(\"HKF88\", parameters=dict(a1=...), charge=1)",
        py::arg("id"), py::arg("implementation") = "", py::arg("parameters") = py::dict(), py::arg("symbol") = "", py::arg("charge") = 0);
    m.def("solventMethod", [](const std::string& id, const std::string& implementation, const py::dict& parameters) {
            return std::const_pointer_cast<SolventMethod>(MethodRegistry::instance().createSolvent(makeApplication(id, implementation, parameters)));
        }, "An equation of state of water, for example solventMethod(\"WaterHGK84\", implementation=\"gems\")",
        py::arg("id"), py::arg("implementation") = "", py::arg("parameters") = py::dict());
    m.def("dielectricMethod", [](const std::string& id, const std::string& implementation) {
            return std::const_pointer_cast<DielectricMethod>(MethodRegistry::instance().createDielectric(makeApplication(id, implementation, py::dict())));
        }, "A model of the dielectric constant of water", py::arg("id"), py::arg("implementation") = "");

    py::class_<SubstanceMethod, std::shared_ptr<SubstanceMethod>>(m, "SubstanceMethod", "A method of a substance, bound to its parameters")
        .def_property_readonly("id", [](const SubstanceMethod& s) { return s.spec().id; })
        .def_property_readonly("summary", [](const SubstanceMethod& s) { return s.spec().summary; })
        .def("evaluate", [](const SubstanceMethod& s, double T, double P, const ReferenceValues& r, const SolventInputs* solvent) { return evaluate(s, T, P, r, solvent); },
             "The properties at T [K], P [Pa], with their derivatives. solvent: for a method that needs the solvent", py::arg("T"), py::arg("P"), py::arg("reference"), py::arg("solvent") = nullptr)
        .def("parameterDerivative", [](const SubstanceMethod& s, std::size_t i, double T, double P, const ReferenceValues& r, const SolventInputs* solvent) {
                 auto d = evaluateParameterDerivative(s, i, T, P, r, solvent);
                 py::dict out;
                 out["G"] = d.G.ddt; out["H"] = d.H.ddt; out["S"] = d.S.ddt; out["Cp"] = d.Cp.ddt; out["V"] = d.V.ddt;
                 return out;
             }, "The derivatives of the properties with respect to one parameter", py::arg("index"), py::arg("T"), py::arg("P"), py::arg("reference"), py::arg("solvent") = nullptr)
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
