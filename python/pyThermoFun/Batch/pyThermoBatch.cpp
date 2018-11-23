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
#include <thermofun/Batch/ThermoBatch.h>
#include <thermofun/Batch/OutputBatch.h>
#include <thermofun/Database.h>
#include <thermofun/ThermoProperties.h>

namespace ThermoFun {

void exportThermoBatch(py::module& m)
{
    auto thermoPropertiesSubstance1 = static_cast<Output(ThermoBatch::*)(double, double, std::string, std::string)>
                                        (&ThermoBatch::thermoPropertiesSubstance);
    auto thermoPropertiesSubstance2 = static_cast<Output(ThermoBatch::*)(double, double, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesSubstance);
    auto thermoPropertiesSubstance3 = static_cast<Output(ThermoBatch::*)(std::array<double,3>, std::array<double,3>, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesSubstance);
    auto thermoPropertiesSubstance4 = static_cast<Output(ThermoBatch::*)(std::vector<std::vector<double>>, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesSubstance);

    auto thermoPropertiesReaction1 = static_cast<Output(ThermoBatch::*)(double, double, std::string, std::string)>
                                        (&ThermoBatch::thermoPropertiesReaction);
    auto thermoPropertiesReaction2 = static_cast<Output(ThermoBatch::*)(double, double, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesReaction);
    auto thermoPropertiesReaction3 = static_cast<Output(ThermoBatch::*)(std::array<double,3>, std::array<double,3>, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesReaction);
    auto thermoPropertiesReaction4 = static_cast<Output(ThermoBatch::*)(std::vector<std::vector<double>>, std::vector<std::string>, std::vector<std::string>)>
                                        (&ThermoBatch::thermoPropertiesReaction);

    py::class_<ThermoBatch>(m, "ThermoBatch")
        .def(py::init<const std::string>())
        .def(py::init<const Database&>())
        .def("setUnits", &ThermoBatch::setUnits)                   
        .def("setDigits", &ThermoBatch::setDigits)                
        .def("setPropertiesUnits", &ThermoBatch::setPropertiesUnits)
        .def("setPropertiesDigits", &ThermoBatch::setPropertiesDigits)         
        .def("setPropertyUnit", &ThermoBatch::setPropertyUnit)            
        .def("setPropertyDigit", &ThermoBatch::setPropertyDigit)           
        .def("setPropertyUnitDigit", &ThermoBatch::setPropertyUnitDigit)       
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance1) 
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance2)
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance3) 
        .def("thermoPropertiesSubstance", thermoPropertiesSubstance4)
        .def("thermoPropertiesReaction", thermoPropertiesReaction1)  
        .def("thermoPropertiesReaction", thermoPropertiesReaction2)  
        .def("thermoPropertiesReaction", thermoPropertiesReaction3)  
        .def("thermoPropertiesReaction", thermoPropertiesReaction4)  
        .def("setOutputSettings", &ThermoBatch::setOutputSettings)         
        .def("setSolventSymbol", &ThermoBatch::setSolventSymbol)          
        ;
}
}