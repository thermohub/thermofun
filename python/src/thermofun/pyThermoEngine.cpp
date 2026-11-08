// This file is part of ThermoFun https://bitbucket.org/gems4/thermofun/
// ThermoFun is a framework for delivering standard state thermodynamic data.
//
// Copyright (c) 2016-2018 G.D.Miron, D.A.Kulik, A.Leal
//
// ThermoFun is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.

// ThermoFun is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with ThermoFun code. If not, see <http://www.gnu.org/licenses/>.

#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

// pybind11 includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <tuple>
namespace py = pybind11;

// C++ includes
#include <algorithm>
#include <cctype>
#include <string>

// ThermoFun includes
#include <ThermoFun/ThermoEngine.h>
#include <ThermoFun/Common/Rounding.hpp>
#include <ThermoFun/Database.h>
#include <ThermoFun/Substance.h>
#include <ThermoFun/Reaction.h>
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun {

namespace {

// The value of a checked word: an unknown word gives the words allowed and a hint
template <class E>
auto wordOf(const std::string& word) -> E
{
    const auto value = model::fromString<E>(word);
    if (!value)
        throw py::value_error(model::unknownWordMessage<E>(word));
    return *value;
}

// A setting given as the enum or as a word
template <class E>
auto valueOf(const py::object& given) -> E
{
    if (py::isinstance<py::str>(given))
        return wordOf<E>(given.cast<std::string>());
    return given.cast<E>();
}

// The word of a value in capitals, as the old fields had it
template <class E>
auto oldWord(E value) -> std::string
{
    auto word = model::toString(value);
    std::transform(word.begin(), word.end(), word.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return word;
}

} // namespace

namespace {

auto pathToDict(const ThermoFun::CalculationPath& path) -> py::dict
{
    py::dict out;
    out["kind"] = path.kind; out["symbol"] = path.symbol; out["source"] = path.source; out["methods"] = path.methods;
    out["coefficient"] = path.coefficient; out["role"] = path.role; out["problem"] = path.problem;
    py::list steps;
    for (const auto& step : path.steps)
        steps.append(pathToDict(step));
    out["steps"] = steps;
    return out;
}

} // namespace

void exportThermoEngine(py::module& m)
{
    py::enum_<model::PropertySource>(m, "PropertySource", "Where the properties of a substance or a reaction come from")
        .value("Direct", model::PropertySource::Direct).value("Reaction", model::PropertySource::Reaction)
        .value("Reactants", model::PropertySource::Reactants).value("Record", model::PropertySource::Record);
    py::enum_<model::EnergyReference>(m, "EnergyReference", "The reference of the energies of a substance")
        .value("BensonHelgeson", model::EnergyReference::BensonHelgeson).value("BermanBrown", model::EnergyReference::BermanBrown)
        .value("SER", model::EnergyReference::SER).value("AssignedEnthalpy", model::EnergyReference::AssignedEnthalpy)
        .value("GibbsHelmholtzFormation", model::EnergyReference::GibbsHelmholtzFormation).value("DFTFormation0K", model::EnergyReference::DFTFormation0K);
    py::enum_<model::StandardPressure>(m, "StandardPressure", "The standard-state pressure")
        .value("OneBar", model::StandardPressure::OneBar).value("OneAtm", model::StandardPressure::OneAtm);
    py::enum_<model::ConcentrationScale>(m, "ConcentrationScale", "The concentration scale of aqueous solutes")
        .value("Molality", model::ConcentrationScale::Molality).value("MoleFraction", model::ConcentrationScale::MoleFraction)
        .value("Molarity", model::ConcentrationScale::Molarity);
    py::enum_<model::WaterReference>(m, "WaterReference", "The reference of the properties of water")
        .value("Geochemical", model::WaterReference::Geochemical).value("SteamTables", model::WaterReference::SteamTables);

    auto appendData1 = static_cast<void(ThermoEngine::*)(std::string)>(&ThermoEngine::appendData);
    auto appendData2 = static_cast<void(ThermoEngine::*)(std::vector<std::string>, std::string)>(&ThermoEngine::appendData);

        // Using lambda functions to provide default value for `state`
    auto electroPropertiesSolvent1 = [](const ThermoEngine& self, double param1, double& param2, const std::string& param3, int state = -1) {
        return self.electroPropertiesSolvent(param1, param2, param3, state);
    };

    auto electroPropertiesSolvent1_default = [](const ThermoEngine& self, double param1, double& param2, const std::string& param3) {
        return self.electroPropertiesSolvent(param1, param2, param3, -1);
    };
    
    auto electroPropertiesSolvent2 = [](const ThermoEngine& self, double param1, double& param2, const Substance& param3, int state = -1) {
        return self.electroPropertiesSolvent(param1, param2, param3, state);
    };

    auto electroPropertiesSolvent2_default = [](const ThermoEngine& self, double param1, double& param2, const Substance& param3) {
        return self.electroPropertiesSolvent(param1, param2, param3, -1);
    };

    auto propertiesSolvent1 = [](const ThermoEngine& self, double param1, double& param2, const std::string& param3, int state = -1) {
        return self.propertiesSolvent(param1, param2, param3, state);
    };

    auto propertiesSolvent1_default = [](const ThermoEngine& self, double param1, double& param2, const std::string& param3) {
        return self.propertiesSolvent(param1, param2, param3, -1);
    };
    
    auto propertiesSolvent2 = [](const ThermoEngine& self, double param1, double& param2, const Substance& param3, int state = -1) {
        return self.propertiesSolvent(param1, param2, param3, state);
    };

    auto propertiesSolvent2_default = [](const ThermoEngine& self, double param1, double& param2, const Substance& param3) {
        return self.propertiesSolvent(param1, param2, param3, -1);
    };

    auto thermoPropertiesSubstance1 = static_cast<ThermoPropertiesSubstance(ThermoEngine::*)(double, double&, std::string) const>(&ThermoEngine::thermoPropertiesSubstance);
    //auto electroPropertiesSolvent1 = static_cast<ElectroPropertiesSolvent(ThermoEngine::*)(double, double&, std::string) const>(&ThermoEngine::electroPropertiesSolvent);
    //auto propertiesSolvent1 = static_cast<PropertiesSolvent(ThermoEngine::*)(double, double&, std::string) const>(&ThermoEngine::propertiesSolvent);
    auto thermoPropertiesReaction1 = static_cast<ThermoPropertiesReaction(ThermoEngine::*)(double, double&, std::string) const>(&ThermoEngine::thermoPropertiesReaction);
    auto thermoPropertiesReactionFromReactants1 = static_cast<ThermoPropertiesReaction(ThermoEngine::*)(double, double&, std::string) const>(&ThermoEngine::thermoPropertiesReactionFromReactants);

    auto thermoPropertiesSubstance2 = static_cast<ThermoPropertiesSubstance(ThermoEngine::*)(double, double&, const Substance&) const>(&ThermoEngine::thermoPropertiesSubstance);
    //auto electroPropertiesSolvent2 = static_cast<ElectroPropertiesSolvent(ThermoEngine::*)(double, double&, const Substance&) const>(&ThermoEngine::electroPropertiesSolvent);
    //auto propertiesSolvent2 = static_cast<PropertiesSolvent(ThermoEngine::*)(double, double&, const Substance&) const>(&ThermoEngine::propertiesSolvent);
    auto thermoPropertiesReaction2 = static_cast<ThermoPropertiesReaction(ThermoEngine::*)(double, double&, const Reaction&) const>(&ThermoEngine::thermoPropertiesReaction);
    auto thermoPropertiesReactionFromReactants2 = static_cast<ThermoPropertiesReaction(ThermoEngine::*)(double, double&, const Reaction&) const>(&ThermoEngine::thermoPropertiesReactionFromReactants);

    m.def("optionWords", [] {
        std::map<std::string, std::vector<std::string>> out;
        out["property_source"] = model::words<model::PropertySource>();
        out["energy_reference"] = model::words<model::EnergyReference>();
        out["standard_pressure"] = model::words<model::StandardPressure>();
        out["concentration_scale"] = model::words<model::ConcentrationScale>();
        out["water_reference"] = model::words<model::WaterReference>();
        return out;
    }, "The valid words of the options of the engine");

    py::class_<CalculationSnapshot>(m, "CalculationSnapshot", "What a calculation depends on")
        .def_readonly("thermofun_version", &CalculationSnapshot::thermofun_version)
        .def_readonly("format_version", &CalculationSnapshot::format_version)
        .def_readonly("dataset", &CalculationSnapshot::dataset)
        .def_readonly("dataset_version", &CalculationSnapshot::dataset_version)
        .def_readonly("content_hash", &CalculationSnapshot::content_hash)
        .def_readonly("implementations", &CalculationSnapshot::implementations)
        .def_readonly("physical_constants", &CalculationSnapshot::physical_constants)
        .def_readonly("water_symbol", &CalculationSnapshot::water_symbol)
        .def_readonly("water_methods", &CalculationSnapshot::water_methods)
        .def_readonly("options", &CalculationSnapshot::options)
        .def_readonly("conventions", &CalculationSnapshot::conventions)
        .def("lines", &snapshotLines, "The snapshot as short lines of text");

    const auto sensitivities = [](const std::vector<std::tuple<std::string, std::string, std::string>>& parameters) {
        std::vector<ParameterRef> out;
        for (const auto& [symbol, method, parameter] : parameters)
            out.push_back({symbol, method, parameter});
        return out;
    };

    py::class_<ParameterHandle>(m, "ParameterHandle", "One parameter of a record of an engine, for a fit")
        .def("name", &ParameterHandle::name)
        .def("value", &ParameterHandle::value, "The value in SI")
        .def("setValue", &ParameterHandle::setValue, py::arg("value"), "Changes the value (SI); only the results that depend on the record are cleared")
        .def("uncertainty", &ParameterHandle::uncertainty, "The standard uncertainty in SI, or None")
        .def("key", [](const ParameterHandle& h) { return std::make_tuple(h.ref().symbol, h.ref().method, h.ref().parameter); }, "(symbol, method, parameter)");

    py::class_<ConsistencyOptions>(m, "ConsistencyOptions", "The options of ThermoEngine.consistencyReport")
        .def(py::init<>())
        .def_readwrite("delta_G_threshold", &ConsistencyOptions::delta_G_threshold, "J/mol; a larger difference is inconsistent");

    py::class_<ConsistencyItem>(m, "ConsistencyItem", "One record at one T and P that has two ways to its properties")
        .def_readonly("symbol", &ConsistencyItem::symbol)
        .def_readonly("kind", &ConsistencyItem::kind)
        .def_readonly("T", &ConsistencyItem::T)
        .def_readonly("P", &ConsistencyItem::P)
        .def_readonly("delta_G", &ConsistencyItem::delta_G)
        .def_readonly("delta_logK", &ConsistencyItem::delta_logK)
        .def_readonly("flag", &ConsistencyItem::flag)
        .def_readonly("message", &ConsistencyItem::message);

    py::class_<ThermoEngine>(m, "ThermoEngine", "Contains methods for calculating the thermodynamic properties of the substances and reactions")
//        .def(py::init<>())
        .def(py::init<const std::string>(), "Constructor using a thermofun database as a JSON string")
        .def(py::init<const Database&>(), "Constructor using a thermofun database")
        .def(py::init<const ThermoEngine&>())
        .def("appendData", appendData1, "Append a json string record to the database from a file", py::arg("record_json_string"))
        .def("appendData", appendData2, "Append records of given label (element, substance, reaction) to the database from a list of JSON strings", py::arg("records_json"), py::arg("label"))
        .def("setPreferences", &ThermoEngine::setPreferences, "Set the preferences used in the engine calculations", py::arg("preferences"))
        .def("database", &ThermoEngine::database, py::return_value_policy::reference_internal, "The database of the engine")
        .def("setSolventSymbol", &ThermoEngine::setSolventSymbol, "Sets the symbol of the solvent record present in the thermodynamic dataset. Will be used to calculate the solvent properties ", py::arg("symbol"))
        .def("setPropertySource", [](ThermoEngine& en, const std::string& symbol, py::object source) { en.setPropertySource(symbol, valueOf<model::PropertySource>(source)); },
             "Where the properties of one substance or reaction come from: PropertySource or the words record, direct, reaction, reactants. It replaces the preference property_source for that symbol", py::arg("symbol"), py::arg("source"))
        .def("propertySource", &ThermoEngine::propertySource, "The source of a symbol: its own, else the preference of the engine", py::arg("symbol"))
        .def("solventSymbol", &ThermoEngine::solventSymbol, "Returns the symbol of the solvent record used to calculate the solvent properties")
        .def("saturationPressure", &ThermoEngine::saturationPressure, py::arg("T"),
             "The saturation pressure of water at T (K), in Pa: what P = 0 means in a calculation. From the water record of the engine, else from the Wagner-Pruss curve; an error at or above the critical temperature of water")
        .def("setConventions", &ThermoEngine::setConventions, "Set the thermodynamic conventions used in the engine calculations", py::arg("conventions"))
        .def_property( "preferences", [](ThermoEngine& en) -> EnginePreferences& { return en.preferences(); }, &ThermoEngine::setPreferences, py::return_value_policy::reference_internal, "The preferences used in engine calculations. A change (also of one field) applies from the next calculation" )
        .def_property( "conventions", [](ThermoEngine& en) -> EngineConventions& { return en.conventions(); }, &ThermoEngine::setConventions, py::return_value_policy::reference_internal, "The thermodynamic conventions used in engine calculations. A change (also of one field) applies from the next calculation" )
        .def("explain", [](const ThermoEngine& en, double T, double P, std::string symbol) {
                 py::list out;
                 for (const auto& step : en.explain(T, P, symbol))
                 {
                     py::dict row;
                     row["method"] = step.method; row["G"] = step.G; row["H"] = step.H; row["S"] = step.S; row["Cp"] = step.Cp; row["V"] = step.V;
                     out.append(row);
                 }
                 return out;
             }, "The methods that calculate a substance, in the order they run, with the state after each one (SI). Empty if no such methods run: water, a substance defined by a reaction, a record without a base method.", py::arg("temperature"), py::arg("pressure"), py::arg("symbol"))
        .def("calculationPath", [](const ThermoEngine& en, const std::string& symbol) { return pathToDict(en.calculationPath(symbol)); }, py::arg("symbol"),
             "How a substance or a reaction is calculated with the sources of this engine, as a dict: kind, symbol, source, methods, coefficient, role, steps, problem. It calculates nothing.")
        .def("thermoPropertiesSubstanceDetailed", [](const ThermoEngine& en, double T, double P, std::string symbol) {
                 const auto d = en.thermoPropertiesSubstanceDetailed(T, P, symbol);
                 py::list steps;
                 for (const auto& step : d.steps)
                 {
                     py::dict row;
                     row["method"] = step.method; row["G"] = step.G; row["H"] = step.H; row["S"] = step.S; row["Cp"] = step.Cp; row["V"] = step.V;
                     steps.append(row);
                 }
                 py::dict out;
                 out["properties"] = d.properties; out["source"] = d.source; out["reaction"] = d.reaction; out["steps"] = steps; out["conversions"] = d.conversions;
                 return out;
             }, "The result of a calculation and how it came: properties, source, reaction, steps (the methods in order, SI) and conversions.", py::arg("temperature"), py::arg("pressure"), py::arg("symbol"))
        .def("parameterSensitivities", [sensitivities](const ThermoEngine& self, double T, double P, const std::string& symbol, const std::vector<std::tuple<std::string, std::string, std::string>>& parameters) {
                 return self.parameterSensitivities(T, P, symbol, sensitivities(parameters));
             }, py::arg("T"), py::arg("P"), py::arg("symbol"), py::arg("parameters"),
             "The exact derivatives of the properties of a substance by parameters, each (symbol, method, parameter): method \"\" for a reference value. A dict: property, then the name of the parameter. SI")
        .def("reactionParameterSensitivities", [sensitivities](const ThermoEngine& self, double T, double P, const std::string& symbol, const std::vector<std::tuple<std::string, std::string, std::string>>& parameters) {
                 return self.reactionParameterSensitivities(T, P, symbol, sensitivities(parameters));
             }, py::arg("T"), py::arg("P"), py::arg("symbol"), py::arg("parameters"),
             "The exact derivatives of the properties of a reaction by parameters, each (symbol, method, parameter): method \"\" for a reference value. A dict: property, then the name of the parameter. SI")
        .def("parameter", &ThermoEngine::parameter, py::keep_alive<0, 1>(), py::arg("symbol"), py::arg("method"), py::arg("parameter"),
             "A handle on one parameter of a record: method \"\" for a reference value")
        .def("parameters", [](py::object self, const std::string& symbol) {
                 py::list out;
                 for (auto& handle : self.cast<ThermoEngine&>().parameters(symbol))
                 {
                     py::object item = py::cast(handle);
                     py::detail::keep_alive_impl(item, self);  // a handle needs its engine
                     out.append(item);
                 }
                 return out;
             }, py::arg("symbol"), "The handles of all the values of a record that can be fitted")
        .def("commitParameters", &ThermoEngine::commitParameters, "A database with the current values of the handles")
        .def("setParameterCovariance", [sensitivities](ThermoEngine& self, const std::vector<std::tuple<std::string, std::string, std::string>>& parameters, const std::vector<double>& covariance) {
                 return self.setParameterCovariance(sensitivities(parameters), covariance);
             }, py::arg("parameters"), py::arg("covariance"),
             "Gives parameters a covariance, SI, row after row: one block for each record. Returns the number of the values dropped, between different records")
        .def("numberOfCachedResults", &ThermoEngine::numberOfCachedResults, "The number of results kept in the caches")
        .def("snapshot", &ThermoEngine::snapshot, "What the results depend on: versions, dataset, SHA-256 of the records, implementations")
        .def("consistencyReport", &ThermoEngine::consistencyReport, py::arg("TP"), py::arg("options") = ConsistencyOptions{},
             "The records that can be calculated in two ways, and the difference at each pair (T [K], P [Pa])")
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance1, "Calculate the thermodynamic properties of a substance with a given symbol.", py::arg("temperature"), py::arg("pressure"), py::arg("symbol"))
        .def("electroPropertiesSolvent", electroPropertiesSolvent1, "Calculate the electro-chemical properties of a substance solvent with a given symbol.", py::arg("temperature"), py::arg("pressure"), py::arg("symbol"), py::arg("state"))
        .def("propertiesSolvent", propertiesSolvent1, "Calculate the properties of a substance solvent with a given symbol.", py::arg("temperature"), py::arg("pressure"), py::arg("symbol"), py::arg("state"))
        .def("electroPropertiesSolvent", electroPropertiesSolvent1_default, "Calculate the electro-chemical properties of a substance solvent with a given symbol.", py::arg("temperature"), py::arg("pressure"), py::arg("symbol"))
        .def("propertiesSolvent", propertiesSolvent1_default, "Calculate the properties of a substance solvent with a given symbol.", py::arg("temperature"), py::arg("pressure"), py::arg("symbol"))
        .def("thermoPropertiesReaction", thermoPropertiesReaction1, "Calculate the thermodynamic properties of a reaction with a given symbol, or for a given reaction equation.", py::arg("temperature"), py::arg("pressure"), py::arg("symbol"))
        .def("thermoPropertiesReactionFromReactants", thermoPropertiesReactionFromReactants1, "Calculate the thermodynamic properties of a reaction from reactants with a given symbol.", py::arg("temperature"), py::arg("pressure"), py::arg("symbol"))
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance2, "Calculate the thermodynamic properties of a given substance object.", py::arg("temperature"), py::arg("pressure"), py::arg("substance"))
        .def("electroPropertiesSolvent", electroPropertiesSolvent2, "Calculate the electro-chemical properties of a given substance solvent object.", py::arg("temperature"), py::arg("pressure"), py::arg("solvent"), py::arg("state"))
        .def("propertiesSolvent", propertiesSolvent2, "Calculate the properties of a given substance solvent object.", py::arg("temperature"), py::arg("pressure"), py::arg("solvent"), py::arg("state"))
        .def("electroPropertiesSolvent", electroPropertiesSolvent2_default, "Calculate the electro-chemical properties of a given substance solvent object.", py::arg("temperature"), py::arg("pressure"), py::arg("solvent"))
        .def("propertiesSolvent", propertiesSolvent2_default, "Calculate the properties of a given substance solvent object.", py::arg("temperature"), py::arg("pressure"), py::arg("solvent"))
        .def("thermoPropertiesReaction", thermoPropertiesReaction2, "Calculate the thermodynamic properties of a given reaction object.", py::arg("temperature"), py::arg("pressure"), py::arg("reaction"))
        .def("thermoPropertiesReactionFromReactants", thermoPropertiesReactionFromReactants2, "Calculate the thermodynamic properties from the reactants of a given reaction object.", py::arg("temperature"), py::arg("pressure"), py::arg("reaction"))
        ;

   py::class_<EnginePreferences>(m, "EnginePreferences")
    .def(py::init<>())  // default constructor
    .def_property("property_source", [](const EnginePreferences& p) { return p.property_source; },
                  [](EnginePreferences& p, py::object source) { p.property_source = valueOf<model::PropertySource>(source); },
                  "where the properties of a record come from: PropertySource or the words record (default), direct, reaction, reactants")
    .def_readwrite("method_implementation", &EnginePreferences::method_implementation,
                   "method id -> implementation, for example {'HKF88': 'reaktoro'}; it replaces the implementation of the records")
    .def_readwrite("enable_memoize", &EnginePreferences::enable_memoize,
                   "enable/disable memoization of calculated properties (default: True)")
    .def_readwrite("max_cache_size", &EnginePreferences::max_cache_size,
                   "results kept in all the caches together, default 100000 (about 0.7 kB each); 0: no limit")
    .def_readwrite("solventState", &EnginePreferences::solvent_state,
                   "not read by the calculations (old API): the state of water comes from its record or from the state argument of propertiesSolvent, default 0")
    .def_readwrite("solventSymbol", &EnginePreferences::solvent_symbol,
                   "symbol of solvent used in EoS of solute properties (e.g., H2O@ for HKF model), default H2O@")
    .def_readwrite("fallback_to_reference_properties",
                   &EnginePreferences::fallback_to_reference_properties,
                   "use reference properties if no calculation functions are available True/False (default: False)")
    .def_readwrite("apply_pressure_correction_to_gas_props",
                   &EnginePreferences::apply_pressure_correction_to_gas_props,
                   "a gas with a fluid method or IdealGas has RT ln(P/Pr) in G and -R ln(P/Pr) in S True/False (default: True); False gives the state at Pr")
    .def_readwrite("cancel_fugacity_coefficient_in_gas_props",
                   &EnginePreferences::cancel_fugacity_coefficient_in_gas_props,
                   "remove the fugacity coefficient term RT ln(phi) of the fluids from G True/False (default: False); the residual terms of H and S stay")
    .def("useStandardStateOfGases", &EnginePreferences::useStandardStateOfGases, py::return_value_policy::reference_internal,
         "G(T, Pr) for a gas: sets apply_pressure_correction_to_gas_props = False and cancel_fugacity_coefficient_in_gas_props = True "
         "(the defaults before version 1.0), for a code that adds these terms itself; returns the preferences")
    .def_readwrite("round_to_uncertainty",
                   &EnginePreferences::round_to_uncertainty,
                   "round the values of the calculated properties to the decimals of their uncertainties (err) with the NEA TDB rules (default: False); derivatives are not rounded")
    .def_readwrite("uncertainty_significant_digits",
                   &EnginePreferences::uncertainty_significant_digits,
                   "number of significant digits of the (rounded up) uncertainties if round_to_uncertainty is True (default: 2)")
    .def_readwrite("propagate_parameter_errors",
                   &EnginePreferences::propagate_parameter_errors,
                   "propagate the errors of the reference properties and of the coefficients of the models to the err of the calculated properties True/False (default: False)");

    m.def("round_to_uncertainty",
          [](double value, double error, int significant_digits) {
              rounding::toUncertainty(value, error, significant_digits);
              return py::make_tuple(value, error);
          },
          py::arg("value"), py::arg("error"), py::arg("significant_digits") = 2,
          "Round an uncertainty up to a number of significant digits and the value (half to even) to the same number of decimals (NEA TDB rules); returns (value, error)");
    m.def("round_half_even", &rounding::roundHalfEven, py::arg("x"), py::arg("decimals"),
          "Round to a number of decimals with the NEA TDB rule (a 5 not followed by other non-zero digits rounds to the even digit)");

    py::class_<EngineConventions>(m, "EngineConventions")
        .def(py::init<>())
        .def_property("energy_reference", [](const EngineConventions& c) { return c.energy_reference; },
                      [](EngineConventions& c, py::object v) { c.energy_reference = valueOf<model::EnergyReference>(v); c.apparent_properties.clear(); })
        .def_property("standard_pressure", [](const EngineConventions& c) { return c.standard_pressure; },
                      [](EngineConventions& c, py::object v) { c.standard_pressure = valueOf<model::StandardPressure>(v); })
        .def_property("concentration_scale", [](const EngineConventions& c) { return c.concentration_scale; },
                      [](EngineConventions& c, py::object v) { c.concentration_scale = valueOf<model::ConcentrationScale>(v); })
        .def_property("water_reference", [](const EngineConventions& c) { return c.water_reference; },
                      [](EngineConventions& c, py::object v) { c.water_reference = valueOf<model::WaterReference>(v); c.water_properties.clear(); })
        // the old names: the same values as checked words in capitals
        .def_property("apparent_properties", [](const EngineConventions& c) { return oldWord(c.energy_reference); },
                      [](EngineConventions& c, const std::string& word) { c.energy_reference = wordOf<model::EnergyReference>(word); c.apparent_properties.clear(); })
        .def_property("water_properties", [](const EngineConventions& c) { return oldWord(c.water_reference); },
                      [](EngineConventions& c, const std::string& word) { c.water_reference = wordOf<model::WaterReference>(word); c.water_properties.clear(); });
}

}