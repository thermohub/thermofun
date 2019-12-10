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

// pybind11 includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

// ThermoFun includes
#include <ThermoFun/Database.h>
#include <ThermoFun/Element.h>
#include <ThermoFun/Substance.h>
#include <ThermoFun/Reaction.h>

namespace ThermoFun {

void exportDatabase(py::module& m)
{
    auto appendData1 = static_cast<void(Database::*)(std::string)>(&Database::appendData);
    auto appendData2 = static_cast<void(Database::*)(std::vector<std::string>, std::string)>(&Database::appendData);

    py::class_<Database>(m, "Database")
        .def(py::init<>())
        .def(py::init<const std::string>())
        .def(py::init<const Database&>())
        .def("appendData", appendData1, "Append records to the database from a file.")
        .def("appendData", appendData2, "Append records of given type (elements, substances, reactions) to the database from a list of JSON strings.")
        .def("addElement", &Database::addElement, "Add an Element instance in the database.")
        .def("setElement", &Database::setElement, "Sets a Element in the database. If substance exists the record will be overwritten")
        .def("addMapElements", &Database::addMapElements, "Add a map of Elements in the database.")
        .def("addSubstance", &Database::addSubstance, "Add an Substance instance in the database.")
        .def("setSubstance", &Database::setSubstance, "Sets a substance in the database. If substance exists the record will be overwritten")
        .def("addMapSubstances", &Database::addMapSubstances, "Add a map of Substances in the database.")
        .def("addReaction", &Database::addReaction, "Add an Reaction instance in the database.")
        .def("setReaction", &Database::setReaction, "Sets a reaction in the database. If reaction exists the record will be overwritten")
        .def("addMapReactions", &Database::addMapReactions, "Add a map pf Reactions in the database.")
        .def("getElements", &Database::getElements, "Return all elements in the database")
        .def("getSubstances", &Database::getSubstances, "Return all substances in the database")
        .def("getReactions", &Database::getReactions, "Return all reactions in the database")
        .def("mapElements", &Database::mapElements, "Returns the map of elements in the database")
        .def("mapSubstances", &Database::mapSubstances, "Returns the map of substances in the database")
        .def("mapReactions", &Database::mapReactions, "Returns the map of reactions in the database")
        .def("numberOfElements", &Database::numberOfElements, "Returns the number of elements in the database")
        .def("numberOfSubstances", &Database::numberOfSubstances, "Returns the number of substances in the database")
        .def("numberOfReactions", &Database::numberOfReactions, "Returns the number of reactions in the database")
        .def("getElement", &Database::getElement, "Return a element in the database")
        .def("getSubstance", &Database::getSubstance, "Return a substance in the database")
        .def("getReaction", &Database::getReaction, "Return a reactions in the database")
        .def("containsElement", &Database::containsElement, "Check if the database contains a given element")
        .def("containsSubstance", &Database::containsSubstance, "Check if the database contains a given substance")
        .def("containsReaction", &Database::containsReaction, "Check if the database contains a given reaction")
        .def("parseSubstanceFormula", &Database::parseSubstanceFormula, "Parses a given substance formula present in the database")
        ;
}

}
