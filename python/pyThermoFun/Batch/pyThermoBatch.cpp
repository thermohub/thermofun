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

#ifdef _WIN32
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

namespace ThermoFun {

void exportThermoBatch(py::module& m)
{
    auto thermoPropertiesSubstance1 = static_cast<Output(ThermoBatch::*)(double, double, std::string, std::string)>
                                        (&ThermoBatch::thermoPropertiesSubstance);
    auto thermoPropertiesSubstance2 = static_cast<Output(ThermoBatch::*)(double, double, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesSubstance);
    auto thermoPropertiesSubstance3 = static_cast<Output(ThermoBatch::*)(std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesSubstance);
    auto thermoPropertiesSubstance4 = static_cast<Output(ThermoBatch::*)(std::vector<std::vector<double>>, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesSubstance);
    auto thermoPropertiesSubstance5 = static_cast<Output(ThermoBatch::*)(std::vector<std::vector<double>>, std::vector<std::string>, std::vector<std::string>, std::vector<ThermoPropertiesSubstance>)>
                                        (&ThermoBatch::thermoPropertiesSubstance);
    auto thermoPropertiesSubstance6 = static_cast<Output(ThermoBatch::*)(std::vector<double>, std::vector<double>, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesSubstance);
    auto thermoPropertiesSubstance7 = static_cast<Output(ThermoBatch::*)(std::vector<double>, std::vector<double>, std::vector<std::string>, std::vector<std::string>, std::vector<ThermoPropertiesSubstance>)>
                                        (&ThermoBatch::thermoPropertiesSubstance);                  

    auto thermoPropertiesReaction1 = static_cast<Output(ThermoBatch::*)(double, double, std::string, std::string)>
                                        (&ThermoBatch::thermoPropertiesReaction);
    auto thermoPropertiesReaction2 = static_cast<Output(ThermoBatch::*)(double, double, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesReaction);
    auto thermoPropertiesReaction3 = static_cast<Output(ThermoBatch::*)(std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesReaction);
    auto thermoPropertiesReaction4 = static_cast<Output(ThermoBatch::*)(std::vector<std::vector<double>>, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesReaction);
    auto thermoPropertiesReaction5 = static_cast<Output(ThermoBatch::*)(std::vector<std::vector<double>>, std::vector<std::string>, std::vector<std::string>, std::vector<ThermoPropertiesReaction>)>
                                        (&ThermoBatch::thermoPropertiesReaction);
    auto thermoPropertiesReaction6 = static_cast<Output(ThermoBatch::*)(std::vector<double>, std::vector<double>, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesReaction);
    auto thermoPropertiesReaction7 = static_cast<Output(ThermoBatch::*)(std::vector<double>, std::vector<double>, std::vector<std::string>, std::vector<std::string>, std::vector<ThermoPropertiesReaction>)>
                                        (&ThermoBatch::thermoPropertiesReaction);

    py::class_<ThermoBatch>(m, "ThermoBatch")
        .def(py::init<const std::string>())
        .def(py::init<const Database&>())
        .def(py::init<const ThermoEngine&>())
        .def("setUnits", &ThermoBatch::setUnits)
        .def("setDigits", &ThermoBatch::setDigits)
        .def("setPropertiesUnits", &ThermoBatch::setPropertiesUnits)
        .def("setPropertiesDigits", &ThermoBatch::setPropertiesDigits)
        .def("setPropertyUnit", &ThermoBatch::setPropertyUnit)
        .def("setPropertyDigit", &ThermoBatch::setPropertyDigit)
        .def("setPropertyUnitDigit", &ThermoBatch::setPropertyUnitDigit)
        .def("setTemperatureIncrement", &ThermoBatch::setTemperatureIncrement)
        .def("setPressureIncrement", &ThermoBatch::setPressureIncrement)        
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance1)
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance2)
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance3)
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance4)
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance5)
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance6)
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance7)
        .def("thermoPropertiesReaction", thermoPropertiesReaction1)
        .def("thermoPropertiesReaction", thermoPropertiesReaction2)
        .def("thermoPropertiesReaction", thermoPropertiesReaction3)
        .def("thermoPropertiesReaction", thermoPropertiesReaction4)
        .def("thermoPropertiesReaction", thermoPropertiesReaction5)
        .def("thermoPropertiesReaction", thermoPropertiesReaction6)
        .def("thermoPropertiesReaction", thermoPropertiesReaction7)
        .def("setBatchPreferences", &ThermoBatch::setBatchPreferences)
        .def("setSolventSymbol", &ThermoBatch::setSolventSymbol)
        ;
}

void exportBatchPreferences(py::module& m)
{
    py::class_<BatchPreferences>(m, "BatchPreferences")
        .def(py::init<>())
        .def_readwrite("isFixed", &BatchPreferences::isFixed)
        .def_readwrite("isFloating", &BatchPreferences::isFloating)
        .def_readwrite("isScientific", &BatchPreferences::isScientific)
        .def_readwrite("separator", &BatchPreferences::separator)
        .def_readwrite("fileName", &BatchPreferences::fileName)
        .def_readwrite("solventFileName", &BatchPreferences::solventFileName)
        .def_readwrite("outputSolventProperties", &BatchPreferences::outputSolventProperties)
        .def_readwrite("substancePropertiesFromReaction", &BatchPreferences::substancePropertiesFromReaction)
        .def_readwrite("reactionPropertiesFromReactants", &BatchPreferences::reactionPropertiesFromReactants)
        .def_readwrite("loopOverTPpairsFirst", &BatchPreferences::loopOverTPpairsFirst)
        .def_readwrite("loopTemperatureThenPressure", &BatchPreferences::loopTemperatureThenPressure)
        ;
}

}
