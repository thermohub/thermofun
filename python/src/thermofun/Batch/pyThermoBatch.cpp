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

// C++ includes
#include <array>

// pybind11 includes
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

// ThermoFun includes
#include <ThermoFun/Batch/ThermoBatch.h>
#include <ThermoFun/Batch/OutputBatch.h>
#include <ThermoFun/Database.h>
#include <ThermoFun/ThermoEngine.h>
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun
{

    void exportThermoBatch(py::module &m)
    {
        auto thermoPropertiesSubstance1 = static_cast<Output (ThermoBatch::*)(double, double, std::string, std::string)>(&ThermoBatch::thermoPropertiesSubstance);
        auto thermoPropertiesSubstance2 = static_cast<Output (ThermoBatch::*)(double, double, std::vector<std::string>, std::vector<std::string>)>(&ThermoBatch::thermoPropertiesSubstance);
        auto thermoPropertiesSubstance3 = static_cast<Output (ThermoBatch::*)(std::vector<std::string>, std::vector<std::string>)>(&ThermoBatch::thermoPropertiesSubstance);
        auto thermoPropertiesSubstance4 = static_cast<Output (ThermoBatch::*)(std::vector<std::vector<double>>, std::vector<std::string>, std::vector<std::string>)>(&ThermoBatch::thermoPropertiesSubstance);
        auto thermoPropertiesSubstance5 = static_cast<Output (ThermoBatch::*)(std::vector<std::vector<double>>, std::vector<std::string>, std::vector<std::string>, std::vector<ThermoPropertiesSubstance>)>(&ThermoBatch::thermoPropertiesSubstance);
        auto thermoPropertiesSubstance6 = static_cast<Output (ThermoBatch::*)(std::vector<double>, std::vector<double>, std::vector<std::string>, std::vector<std::string>)>(&ThermoBatch::thermoPropertiesSubstance);
        auto thermoPropertiesSubstance7 = static_cast<Output (ThermoBatch::*)(std::vector<double>, std::vector<double>, std::vector<std::string>, std::vector<std::string>, std::vector<ThermoPropertiesSubstance>)>(&ThermoBatch::thermoPropertiesSubstance);

        auto thermoPropertiesReaction1 = static_cast<Output (ThermoBatch::*)(double, double, std::string, std::string)>(&ThermoBatch::thermoPropertiesReaction);
        auto thermoPropertiesReaction2 = static_cast<Output (ThermoBatch::*)(double, double, std::vector<std::string>, std::vector<std::string>)>(&ThermoBatch::thermoPropertiesReaction);
        auto thermoPropertiesReaction3 = static_cast<Output (ThermoBatch::*)(std::vector<std::string>, std::vector<std::string>)>(&ThermoBatch::thermoPropertiesReaction);
        auto thermoPropertiesReaction4 = static_cast<Output (ThermoBatch::*)(std::vector<std::vector<double>>, std::vector<std::string>, std::vector<std::string>)>(&ThermoBatch::thermoPropertiesReaction);
        auto thermoPropertiesReaction5 = static_cast<Output (ThermoBatch::*)(std::vector<std::vector<double>>, std::vector<std::string>, std::vector<std::string>, std::vector<ThermoPropertiesReaction>)>(&ThermoBatch::thermoPropertiesReaction);
        auto thermoPropertiesReaction6 = static_cast<Output (ThermoBatch::*)(std::vector<double>, std::vector<double>, std::vector<std::string>, std::vector<std::string>)>(&ThermoBatch::thermoPropertiesReaction);
        auto thermoPropertiesReaction7 = static_cast<Output (ThermoBatch::*)(std::vector<double>, std::vector<double>, std::vector<std::string>, std::vector<std::string>, std::vector<ThermoPropertiesReaction>)>(&ThermoBatch::thermoPropertiesReaction);

        py::class_<ThermoBatch>(m, "ThermoBatch", "Useful for performing batch calculations for multiple substance/reaction records for multiple T-P")
            .def(py::init<const std::string>())
            .def(py::init<const Database &>())
            .def(py::init<const ThermoEngine &>())
            // .def("setUnits", &ThermoBatch::setUnits)
            // .def("setDigits", &ThermoBatch::setDigits)
            .def("setPropertiesUnits", &ThermoBatch::setPropertiesUnits)
            .def("setPropertiesDigits", &ThermoBatch::setPropertiesDigits)
            .def("setPropertyUnit", &ThermoBatch::setPropertyUnit)
            .def("setPropertyDigit", &ThermoBatch::setPropertyDigit)
            .def("setPropertyUnitDigit", &ThermoBatch::setPropertyUnitDigit)
            .def("setTemperatureIncrement", &ThermoBatch::setTemperatureIncrement)
            .def("setPressureIncrement", &ThermoBatch::setPressureIncrement)
            .def("thermoPropertiesSubstance", thermoPropertiesSubstance1, "calculate for given property, substance symbol, temperature and pressure", py::arg("temperature"), py::arg("pressure"), py::arg("symbol"), py::arg("property"))
            .def("thermoPropertiesSubstance", thermoPropertiesSubstance2, "calculate for given properties, substance symbols, temperature and pressure", py::arg("temperature"), py::arg("pressure"), py::arg("symbols"), py::arg("properties"))
            .def("thermoPropertiesSubstance", thermoPropertiesSubstance3, "calculate for given properties, substance symbols, temperatures and pressures are set with increment functions", py::arg("symbols"), py::arg("properties"))
            .def("thermoPropertiesSubstance", thermoPropertiesSubstance4, "calculate for given properties, substance symbols, and temperature and pressure pairs", py::arg("TP_pairs"), py::arg("symbols"), py::arg("properties"))
            .def("thermoPropertiesSubstance", thermoPropertiesSubstance5, "calculate for given properties, substance symbols, and temperature and pressure pairs", py::arg("TP_pairs"), py::arg("symbols"), py::arg("properties"), py::arg("provided_results"))
            .def("thermoPropertiesSubstance", thermoPropertiesSubstance6, "calculate for given properties, substance symbols, temperatures and pressures", py::arg("temperatures"), py::arg("pressures"), py::arg("symbols"), py::arg("properties"))
            .def("thermoPropertiesSubstance", thermoPropertiesSubstance7, "calculate for given properties, substance symbols, temperatures and pressures", py::arg("temperatures"), py::arg("pressures"), py::arg("symbols"), py::arg("properties"), py::arg("provided_results"))
            .def("thermoPropertiesReaction", thermoPropertiesReaction1, "calculate for given property, reaction symbol, temperature and pressure", py::arg("temperature"), py::arg("pressure"), py::arg("symbol"), py::arg("property"))
            .def("thermoPropertiesReaction", thermoPropertiesReaction2, "calculate for given properties, reaction symbols, temperature and pressure", py::arg("temperature"), py::arg("pressure"), py::arg("symbols"), py::arg("properties"))
            .def("thermoPropertiesReaction", thermoPropertiesReaction3, "calculate for given properties, reaction symbols, temperatures and pressures are set with increment functions", py::arg("symbols"), py::arg("properties"))
            .def("thermoPropertiesReaction", thermoPropertiesReaction4, "calculate for given properties, reaction symbols, and temperature and pressure pairs", py::arg("TP_pairs"), py::arg("symbols"), py::arg("properties"))
            .def("thermoPropertiesReaction", thermoPropertiesReaction5, "calculate for given properties, reaction symbols, and temperature and pressure pairs", py::arg("TP_pairs"), py::arg("symbols"), py::arg("properties"), py::arg("provided_results"))
            .def("thermoPropertiesReaction", thermoPropertiesReaction6, "calculate for given properties, reaction symbols, temperatures and pressures", py::arg("temperatures"), py::arg("pressures"), py::arg("symbols"), py::arg("properties"))
            .def("thermoPropertiesReaction", thermoPropertiesReaction7, "calculate for given properties, reaction symbols, temperatures and pressures", py::arg("temperatures"), py::arg("pressures"), py::arg("symbols"), py::arg("properties"), py::arg("provided_results"))
            .def("setBatchPreferences", &ThermoBatch::setBatchPreferences)
            .def("setSolventSymbol", &ThermoBatch::setSolventSymbol)
            .def("temperatureIncrement", &ThermoBatch::temperatureIncrement, "returns the default temperature increment")
            .def("pressureIncrement", &ThermoBatch::pressureIncrement, "returns the default pressure increment")
            .def("propertyUnits", &ThermoBatch::propertyUnits, "returns the default properties units")
            .def("propertyDigits", &ThermoBatch::propertyDigits, "returns the default properties number of round digits");
    }

    // void exportDefault(py::module& m)
    //{
    //     py::class_<defaultPropertyNames>(m, "defs", "fdefs")
    //         .def_property_readonly("defaultPropertyNames", defaultPropertyNames)
    //         .def_property_readonly("defaultSolventPropertyUnits", defaultSolventPropertyUnits)
    //         .def_property_readonly("defaultSolventPropertyDigits", defaultSolventPropertyDigits)
    //         .def_property_readonly("defaultPropertyUnits", defaultPropertyUnits)
    //         .def_property_readonly("defaultPropertyDigits", defaultPropertyDigits)
    //         .def_property_readonly("defaultTemperatureIncrement", defaultTemperatureIncrement)
    //         .def_property_readonly("defaultPressureIncrement", defaultPressureIncrement)
    //         ;
    // }

    void exportBatchPreferences(py::module &m)
    {
        py::class_<BatchPreferences>(m, "BatchPreferences", "holds the options for outputting results")
            .def(py::init<>())
            .def_readwrite("isFixed", &BatchPreferences::isFixed, "default=True, using fixed-point notation: the value is represented with exactly as many digits in the decimal part as specified by the precision field")
            .def_readwrite("isFloating", &BatchPreferences::isFloating, "default=False, using floating point notation")
            .def_readwrite("isScientific", &BatchPreferences::isScientific, "default=False, if True write values in scientific notation")
            .def_readwrite("separator", &BatchPreferences::separator, "default=, separator used in output csv file")
            .def_readwrite("fileName", &BatchPreferences::fileName, "output file name")
            .def_readwrite("solventFileName", &BatchPreferences::solventFileName, "output file for solvent properties")
            .def_readwrite("outputSolventProperties", &BatchPreferences::outputSolventProperties, "default=False, output calculated solvent properties to file")
            .def_readwrite("substancePropertiesFromReaction", &BatchPreferences::substancePropertiesFromReaction, "default=False, calculate substance properties from reaction, if provided in the substance record with kez \"reaction\"")
            .def_readwrite("reactionPropertiesFromReactants", &BatchPreferences::reactionPropertiesFromReactants, "default=False, calculate reaction properties from reactants")
            .def_readwrite("loopOverTPpairsFirst", &BatchPreferences::loopOverTPpairsFirst, "default=True, when calculating properties the loop is going for T-P points first and then for substances/reactions list")
            .def_readwrite("loopTemperatureThenPressure", &BatchPreferences::loopTemperatureThenPressure, "default=True, when calculating properties the loop is going for T and the P");
    }

}
