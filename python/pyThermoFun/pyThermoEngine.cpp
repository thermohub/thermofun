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

#if (_MSC_VER > 1929)
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

    auto thermoPropertiesSubstance1 = static_cast<ThermoPropertiesSubstance(ThermoEngine::*)(double, double&, std::string) const>(&ThermoEngine::thermoPropertiesSubstance);
    auto electroPropertiesSolvent1 = static_cast<ElectroPropertiesSolvent(ThermoEngine::*)(double, double&, std::string) const>(&ThermoEngine::electroPropertiesSolvent);
    auto propertiesSolvent1 = static_cast<PropertiesSolvent(ThermoEngine::*)(double, double&, std::string) const>(&ThermoEngine::propertiesSolvent);
    auto thermoPropertiesReaction1 = static_cast<ThermoPropertiesReaction(ThermoEngine::*)(double, double&, std::string) const>(&ThermoEngine::thermoPropertiesReaction);
    auto thermoPropertiesReactionFromReactants1 = static_cast<ThermoPropertiesReaction(ThermoEngine::*)(double, double&, std::string) const>(&ThermoEngine::thermoPropertiesReactionFromReactants);

    auto thermoPropertiesSubstance2 = static_cast<ThermoPropertiesSubstance(ThermoEngine::*)(double, double&, const Substance&) const>(&ThermoEngine::thermoPropertiesSubstance);
    auto electroPropertiesSolvent2 = static_cast<ElectroPropertiesSolvent(ThermoEngine::*)(double, double&, const Substance&) const>(&ThermoEngine::electroPropertiesSolvent);
    auto propertiesSolvent2 = static_cast<PropertiesSolvent(ThermoEngine::*)(double, double&, const Substance&) const>(&ThermoEngine::propertiesSolvent);
    auto thermoPropertiesReaction2 = static_cast<ThermoPropertiesReaction(ThermoEngine::*)(double, double&, const Reaction&) const>(&ThermoEngine::thermoPropertiesReaction);
    auto thermoPropertiesReactionFromReactants2 = static_cast<ThermoPropertiesReaction(ThermoEngine::*)(double, double&, const Reaction&) const>(&ThermoEngine::thermoPropertiesReactionFromReactants);

    py::class_<ThermoEngine>(m, "ThermoEngine")
//        .def(py::init<>())
        .def(py::init<const std::string>())
        .def(py::init<const Database&>())
        .def(py::init<const ThermoEngine&>())
        .def("appendData", appendData1, "Append records to the database from a file.")
        .def("appendData", appendData2, "Append records of given type (elements, substances, reactions) to the database from a list of JSON strings.")
        .def("setSolventSymbol", &ThermoEngine::setSolventSymbol)
        .def("solventSymbol", &ThermoEngine::solventSymbol)
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance1, "Calculate the thermodynamic properties of a substance with a given symbol.")
        .def("electroPropertiesSolvent", electroPropertiesSolvent1, "Calculate the electro-chemical properties of a substance solvent with a given symbol.")
        .def("propertiesSolvent", propertiesSolvent1, "Calculate the properties of a substance solvent with a given symbol.")
        .def("thermoPropertiesReaction", thermoPropertiesReaction1, "Calculate the thermodynamic properties of a reaction with a given symbol, or for a given reaction equation.")
        .def("thermoPropertiesReactionFromReactants", thermoPropertiesReactionFromReactants1, "Calculate the thermodynamic properties of a reaction from reactants with a given symbol.")
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance2, "Calculate the thermodynamic properties of a given substance object.")
        .def("electroPropertiesSolvent", electroPropertiesSolvent2, "Calculate the electro-chemical properties of a given substance solvent object.")
        .def("propertiesSolvent", propertiesSolvent2, "Calculate the properties of a given substance solvent object.")
        .def("thermoPropertiesReaction", thermoPropertiesReaction2, "Calculate the thermodynamic properties of a given reaction object.")
        .def("thermoPropertiesReactionFromReactants", thermoPropertiesReactionFromReactants2, "Calculate the thermodynamic properties from the reactants of a given reaction object.")
        ;
}

}
