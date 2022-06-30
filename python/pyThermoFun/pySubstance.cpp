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
namespace py = pybind11;

// ThermoFun includes
#include <ThermoFun/Substance.h>
#include <ThermoFun/ThermoProperties.h>
#include <ThermoFun/ThermoParameters.h>

namespace ThermoFun {

void exportSubstance(py::module& m)
{
    py::class_<Substance>(m, "Substance")
        .def(py::init<>())
        .def(py::init<const Substance&>())
        .def(py::init<std::string>())
        .def("setName", &Substance::setName, "Set the name of the Substance.")
        .def("setSymbol", &Substance::setSymbol, "Set the symbol of the Substance.")
        .def("setFormula", &Substance::setFormula, "Set the formula of the Substance.")
        .def("setReactionSymbol", &Substance::setReactionSymbol, "Set the symbol of the reaction that can be used to define the substance properties.")
        .def("setMolarMass", &Substance::setMolarMass, "Set the molar mass of the chemical species (in units of kg/mol)")
        .def("setThermoReferenceProperties", &Substance::setThermoReferenceProperties, "Set the reference thermodynamic data of the substance.")
        .def("setThermoParameters", &Substance::setThermoParameters, "Set the reference thermodynamic data of the substance.")
        .def("setMethodGenEoS", &Substance::setMethodGenEoS, "Set the code for general EOS calculation method")
        .def("setMethod_T", &Substance::setMethod_T, "Set the code for the temperature correction method")
        .def("setMethod_P", &Substance::setMethod_P, "Set the code for the pressure correction method")
        .def("setLowerT", &Substance::setLowerT, "Set upper temperature limit of the correction method (K)")
        .def("setLowerP", &Substance::setLowerP, "Set upper presure limit of the correction method (Pa)")
        .def("setUpperT", &Substance::setUpperT, "Set lower temperature limit of the correction method (K)")
        .def("setUpperP", &Substance::setUpperP, "Set lower pressure limit for the correction method (Pa)")
        .def("setSubstanceClass", &Substance::setSubstanceClass, "Set the code for the substance class")
        .def("setAggregateState", &Substance::setAggregateState, "Set the code for the aggregate state of the substance")
        .def("setThermoCalculationType", &Substance::setThermoCalculationType, "Set the code for the substance calculation type")
        .def("setCharge", &Substance::setCharge, "Set the charge of the substance")
        .def("setReferenceT", &Substance::setReferenceT, "Set the reference temperature (K)")
        .def("setReferenceP", &Substance::setReferenceP, "Set the references pressure (Pa)")
        .def("setJsonString", &Substance::setJsonString, "Set the record as a json string")
        .def("name", &Substance::name, "Return the name of the chemical Substance")
        .def("symbol", &Substance::symbol, "Return the symbol of the chemical Substance")
        .def("formula", &Substance::formula, "Return the formula of the chemical Substance")
        .def("reactionSymbol", &Substance::reactionSymbol, "Return the reaction symbol that can be used to define the properties of the chemical Substance")
        .def("molarMass", &Substance::molarMass, "Return the molar mass of the chemical species (in units of kg/mol)")
        .def("referenceT", &Substance::referenceT, "Return the reference temperature (K)")
        .def("referenceP", &Substance::referenceP, "Return the reference pressure (Pa)")
        .def("lowerT", &Substance::lowerT, "Return upper temperature limit of the correction method (K)")
        .def("lowerP", &Substance::lowerP, "Return upper presure limit of the correction method (Pa)")
        .def("upperT", &Substance::upperT, "Return lower temperature limit of the correction method (K)")
        .def("upperP", &Substance::upperP, "Return lower pressure limit for the correction method (Pa)")
        .def("thermoProperties", &Substance::thermoProperties, "Return the thermodynamic data of the substance.")
        .def("thermoParameters", &Substance::thermoParameters, "Return the thermodynamic parameters for calculating thermodynamic properties of the substance.")
        .def("thermoReferenceProperties", &Substance::thermoReferenceProperties, "Return the reference thermodynamic data of the substance.")
        .def("methodGenEOS", &Substance::methodGenEOS, "Return the general EOS method code")
        .def("method_T", &Substance::method_T, "Return the temperature correction method code")
        .def("method_P", &Substance::method_P, "Return the pressure correction method code")
        .def("substanceClass", &Substance::substanceClass, "Return the class type of the substance")
        .def("thermoCalculationType", &Substance::thermoCalculationType, "Return the code of the substance proeprties calculation type")
        .def("aggregateState", &Substance::aggregateState, "Return the aggregate state of a substance")
        .def("charge", &Substance::charge, "Return the charge of a substance")
        .def("jsonString", &Substance::jsonString, "Return the record as a json string")
        ;
}

}
