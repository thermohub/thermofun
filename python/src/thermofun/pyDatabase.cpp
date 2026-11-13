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

#include <sstream>
#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

// pybind11 includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

// ThermoFun includes
#include <ThermoFun/Database.h>
#include <ThermoFun/Convert.h>
#include <ThermoFun/Element.h>
#include <ThermoFun/Substance.h>
#include <ThermoFun/Reaction.h>

namespace ThermoFun {

void exportDatabase(py::module& m)
{
    m.def("legacyFileDiagnostics", &legacyFileDiagnostics, py::arg("file"), "What the legacy reader finds in a legacy file: the quirks and the keys that it does not read");
    m.def("checkRecord", &checkRecord, py::arg("section"), py::arg("symbol"), py::arg("record"),
          "The problems of one v2 record given as JSON text: section is elements, substances or reactions");

    py::class_<ShortenedNumbers>(m, "ShortenedNumbers", "How many numbers the readable form writes otherwise than the exact one")
        .def_readonly("count", &ShortenedNumbers::count)
        .def_readonly("largest", &ShortenedNumbers::largest, "the largest relative change")
        .def("__iter__", [](const ShortenedNumbers& s) { return py::iter(py::make_tuple(s.count, s.largest)); })
        .def("__repr__", [](const ShortenedNumbers& s) { return "ShortenedNumbers(count=" + std::to_string(s.count) + ", largest=" + std::to_string(s.largest) + ")"; });
    m.def("shortenedNumbers", &shortenedNumbers, py::arg("database"),
          "The numbers that toJson(True) writes otherwise than toJson(False): count and largest relative change; None if they do not pair up");
    const auto options = [](model::DataFormat to, bool yaml, bool split, bool readable, const std::string& report) {
        ConvertOptions o;
        o.to = to;
        o.yaml = yaml;
        o.split = split;
        o.readable = readable;
        o.report = report;
        return o;
    };
    m.def("conversionReport", [options](const Database& database, const std::string& input, model::DataFormat to, bool readable) {
              return conversionReport(database, input, options(to, false, false, readable, ""));
          }, py::arg("database"), py::arg("input"), py::arg("to") = model::DataFormat::V2, py::arg("readable") = false,
          "The migration report as Markdown");
    m.def("convertFile", [options](const std::string& input, const std::string& output, model::DataFormat to, bool yaml, bool split, bool readable, const std::string& report) {
              return convertFile(input, output, options(to, yaml, split, readable, report));
          }, py::arg("input"), py::arg("output"), py::arg("to") = model::DataFormat::V2, py::arg("yaml") = false, py::arg("split") = false,
          py::arg("readable") = false, py::arg("report") = "",
          "Reads input (legacy or v2 file or folder) and writes output: v2 JSON or YAML, a dataset folder (split), or legacy JSON. "
          "Writes the migration report if report is a file name. Returns the diagnostics of the load");

    auto appendData1 = static_cast<void(Database::*)(std::string)>(&Database::appendData);
    auto appendData2 = static_cast<void(Database::*)(std::vector<std::string>, std::string)>(&Database::appendData);

    auto addElement1 = static_cast<void(Database::*)(const std::string&)>(&Database::addElement);
    auto addElement2 = static_cast<void(Database::*)(const Element&)>(&Database::addElement);
    auto setElement1 = static_cast<void(Database::*)(const std::string&)>(&Database::setElement);
    auto setElement2 = static_cast<void(Database::*)(const Element&)>(&Database::setElement);

    auto addSubstance1 = static_cast<void(Database::*)(const std::string&)>(&Database::addSubstance);
    auto addSubstance2 = static_cast<void(Database::*)(const Substance&)>(&Database::addSubstance);
    auto setSubstance1 = static_cast<void(Database::*)(const std::string&)>(&Database::setSubstance);
    auto setSubstance2 = static_cast<void(Database::*)(const Substance&)>(&Database::setSubstance);

    auto addReaction1 = static_cast<void(Database::*)(const std::string&)>(&Database::addReaction);
    auto addReaction2 = static_cast<void(Database::*)(const Reaction&)>(&Database::addReaction);
    auto setReaction1 = static_cast<void(Database::*)(const std::string&)>(&Database::setReaction);
    auto setReaction2 = static_cast<void(Database::*)(const Reaction&)>(&Database::setReaction);

    py::class_<Database>(m, "Database", "Stores maps of elements, substances and reactions. A database instance can be used to create a ThermoEngine instance which can be further used to calculate the standard thermodynamic properties of substances and reactions at T and P")
        .def(py::init<>())
        .def(py::init<const std::string>(), "constructor using a JSON string with the thermofun database format")
        .def(py::init<const Database&>())
        .def_property_readonly( "conventions", [](Database& db) -> const DatabaseConventions& { return db.conventions(); }, py::return_value_policy::reference_internal, "Access the thermodynamic conventions used by this database" )
        .def("appendData", appendData1, "Append records to the database from a file.")
        .def("appendData", appendData2, "Append records of given type (elements, substances, reactions) to the database from a list of JSON strings.")
        .def("addElement", addElement1, "Add an Element instance in the database. If the element with the symbol exists the record will not be added.")
        .def("setElement", setElement1, "Sets an Element in the database. If the element with the symbol exists the record will be overwritten.")
        .def("addElement", addElement2, "Add an Element instance in the database from a JSON string. If the element with the symbol exists the record will not be added.")
        .def("setElement", setElement2, "Sets an Element in the database from a JSON string. If the element with the symbol exists the record will be overwritten.")
        .def("addMapElements", &Database::addMapElements, "Add a map of Elements in the database.")
        .def("setMapElements", &Database::setMapElements, "Set the records of a map of Elements in the database (existing records are overwritten, the others are added).")
        .def("addSubstance", addSubstance1, "Add a Substance instance in the database. If the substance with the symbol exists the record will not be added.")
        .def("setSubstance", setSubstance1, "Sets a substance in the database. If the substance with the symbol exists the record will be overwritten.")
        .def("addSubstance", addSubstance2, "Add a Substance instance in the database from a JSON string. If the substance with the symbol exists the record will not be added")
        .def("setSubstance", setSubstance2, "Sets a substance in the database  from a JSON string. If the substance with the symbol exists the record will be overwritten.")
        .def("addMapSubstances", &Database::addMapSubstances, "Add a map of Substances in the database.")
        .def("setMapSubstances", &Database::setMapSubstances, "Set the records of a map of Substances in the database (existing records are overwritten, the others are added).")
        .def("addReaction", addReaction1, "Add a Reaction instance in the database. If the reaction with the symbol exists the record will not be added.")
        .def("setReaction", setReaction1, "Set a reaction in the database. If the reaction with the symbol exists the record will be overwritten.")
        .def("addReaction", addReaction2, "Add a Reaction instance in the database from a JSON string. If the reaction with the symbol exists the record will not be added.")
        .def("setReaction", setReaction2, "Set a reaction in the database from a JSON string. If the reaction with the symbol exists the record will be overwritten.")
        .def("addMapReactions", &Database::addMapReactions, "Add a map pf Reactions in the database.")
        .def("setMapReactions", &Database::setMapReactions, "Set the records of a map of Reactions in the database (existing records are overwritten, the others are added).")
        .def("getElements", &Database::getElements, "Return all elements in the database")
        .def("getSubstances", &Database::getSubstances, "Return all substances in the database")
        .def("getReactions", &Database::getReactions, "Return all reactions in the database")
        .def("getElementsList", &Database::getElementsList, "List of all elements in the database")
        .def("getSubstancesList", &Database::getSubstancesList, "List of all substances in the database")
        .def("getReactionsList", &Database::getReactionsList, "List of all reactions in the database")
        .def("mapElements", &Database::mapElements, "Returns the map of elements in the database")
        .def("mapSubstances", &Database::mapSubstances, "Returns the map of substances in the database")
        .def("mapReactions", &Database::mapReactions, "Returns the map of reactions in the database")
        .def("numberOfElements", &Database::numberOfElements, "Returns the number of elements in the database")
        .def("numberOfSubstances", &Database::numberOfSubstances, "Returns the number of substances in the database")
        .def("numberOfReactions", &Database::numberOfReactions, "Returns the number of reactions in the database")
        .def("getElement", &Database::getElement, "Return a element in the database")
        .def("getSubstance", &Database::getSubstance, "Return a substance in the database")
        .def("getReaction", &Database::getReaction, "Return a reactions in the database")
        .def("element", &Database::element, py::return_value_policy::reference, "Reference to the element in the database")
        .def("substance", &Database::substance, py::return_value_policy::reference, "Reference to the substance in the database")
        .def("reaction", &Database::reaction, py::return_value_policy::reference, "Reference to the reaction in the database")
        .def("containsElement", &Database::containsElement, "Check if the database contains a given element")
        .def("containsSubstance", &Database::containsSubstance, "Check if the database contains a given substance")
        .def("containsReaction", &Database::containsReaction, "Check if the database contains a given reaction")
        .def("elementalEntropyFormula", &Database::elementalEntropyFormula, "Returns the sum of the standard entropies of the elements of a formula")
        .def("parseSubstanceFormula", &Database::parseSubstanceFormula, "Parses a given substance formula present in the database")
        .def("toJson", &Database::toJson, py::arg("readable") = true, "All records in the v2 format as JSON text. readable: a value from a single-precision source is written as the shortest decimal of that float")
        .def("toYaml", &Database::toYaml, py::arg("readable") = true, "All records in the v2 format as YAML text")
        .def("toLegacyJson", &Database::toLegacyJson, py::arg("use_original_text") = true, "All records as legacy JSON text. What the format cannot hold is left out, with a warning")
        .def("legacyWriteProblems", &Database::legacyWriteProblems, "What toLegacyJson leaves out")
        .def("validate", &Database::validate, "All problems of the database, also those that a load does not refuse because a record can come from another file")
        .def("save", &Database::save, py::arg("path"), py::arg("format") = model::DataFormat::V2, py::arg("readable") = true, "Write all records. V2: YAML if the name ends with .yaml or .yml, else JSON. Legacy: legacy JSON")
        .def("saveDataset", &Database::saveDataset, py::arg("folder"), py::arg("yaml") = false, py::arg("readable") = true,
             "Write all records as a dataset folder: thermodataset.json (or .yaml) and one file for each record in elements/, substances/ and reactions/")
        .def("thermoDataSet", &Database::thermoDataSet, py::return_value_policy::copy, "Data about the loaded dataset (a copy)")
        .def("methods", &Database::methods, py::return_value_policy::copy, "The Method records that the database gives (a copy)")
        .def("parameterSets", &Database::parameterSets, py::return_value_policy::copy, "The parameter sets that the database gives (a copy)")
        .def("dataSources", &Database::dataSources, py::return_value_policy::copy, "The data sources that the database gives (a copy)")
        .def("loadDiagnostics", &Database::loadDiagnostics, py::return_value_policy::copy, "What the load of v2 files reported")
        .def("setSubstancesChargeFromFromula", &Database::setSubstancesChargeFromFromula, "Sets the charge of all substances based in their formulas, uses the formula parser")
        .def("__str__", [](const Database& self) { std::stringstream ss; ss << self.getElements(); ss << self.getSubstances(); ss << self.getReactions(); return ss.str(); })
        ;

    py::class_<DatabaseConventions>(m, "DatabaseConventions")
        .def(py::init<>())
        .def_readwrite("water_triple_point_reference", &DatabaseConventions::water_triple_point_reference);
}

}
