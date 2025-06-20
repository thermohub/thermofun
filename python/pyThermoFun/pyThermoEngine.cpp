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

// You should have received a copy of the GNU General Public License
// along with ThermoFun code. If not, see <http://www.gnu.org/licenses/>.

#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

// pybind11 includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

// ThermoFun includes
#include <ThermoFun/ThermoEngine.h>
#include <ThermoFun/Database.h>
#include <ThermoFun/Substance.h>
#include <ThermoFun/Reaction.h>
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun {

void exportThermoEngine(py::module& m)
{
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

    py::class_<ThermoEngine>(m, "ThermoEngine", "Contains methods for calculating the thermodynamic properties of the substances and reactions")
//        .def(py::init<>())
        .def(py::init<const std::string>(), "Constructor using a thermofun database as a JSON string")
        .def(py::init<const Database&>(), "Constructor using a thermofun database")
        .def(py::init<const ThermoEngine&>())
        .def("appendData", appendData1, "Append a json string record to the database from a file", py::arg("record_json_string"))
        .def("appendData", appendData2, "Append records of given label (element, substance, reaction) to the database from a list of JSON strings", py::arg("records_json"), py::arg("label"))
        .def("setSolventSymbol", &ThermoEngine::setSolventSymbol, "Sets the symbol of the solvent record present in the thermodynamic dataset. Will be used to calculate the solvent properties ", py::arg("symbol"))
        .def("solventSymbol", &ThermoEngine::solventSymbol, "Returns the symbol of the solvent record used to calculate the solvent properties")
        .def("preferences", 
         static_cast<EnginePreferences& (ThermoEngine::*)()>(&ThermoEngine::preferences),
         py::return_value_policy::reference_internal)
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
    .def_readwrite("solventState", &EnginePreferences::solventState)
    .def_readwrite("waterTripleProperties", &EnginePreferences::waterTripleProperties)
    .def_readwrite("solventSymbol", &EnginePreferences::solventSymbol);
}

}
