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
#include <ThermoFun/Reaction.h>
#include <ThermoFun/ThermoProperties.h>
#include <ThermoFun/ThermoParameters.h>

namespace ThermoFun {

void exportReaction(py::module& m)
{
    py::class_<Reaction>(m, "Reaction")
        .def(py::init<>())
        .def(py::init<const Reaction&>())
        .def(py::init<std::string>())
        .def("setName", &Reaction::setName,"Set the name of the Reaction.")
        .def("setSymbol", &Reaction::setSymbol,"Set the symbol of the Reaction.")
        .def("setEquation", &Reaction::setEquation,"Set the equation of the Reaction")
        .def("setReactants", &Reaction::setReactants,"Set map of reactant symbols")
        .def("setReferenceT", &Reaction::setReferenceT,"Set reference temperature fo the standard properties of reaction (K)")
        .def("setReferenceP", &Reaction::setReferenceP,"Set references pressure for the standard properties of reaction (Pa)")
        .def("setLowerT", &Reaction::setLowerT,"Set upper temperature limit of the correction method (K)")
        .def("setLowerP", &Reaction::setLowerP,"Set upper presure limit of the correction method (Pa)")
        .def("setUpperT", &Reaction::setUpperT,"Set lower temperature limit of the correction method (K)")
        .def("setUpperP", &Reaction::setUpperP,"Set lower pressure limit for the correction method (Pa)")
        .def("setMethodGenEoS", &Reaction::setMethodGenEoS,"Set the code for general EOS calculation method")
        .def("setMethod_T", &Reaction::setMethod_T,"Set the code for the temperature correction method")
        .def("setMethod_P", &Reaction::setMethod_P,"Set the code for the pressure correction method")
        .def("setThermoReferenceProperties", &Reaction::setThermoReferenceProperties,"Set the references properties of reaction")
        .def("setThermoParameters", &Reaction::setThermoParameters,"Set the parameters used for calculating the properties of reaction at T and P")
        .def("setJsonString", &Reaction::setJsonString, "Set the record as a json string")
        .def("fromEquation", &Reaction::fromEquation, "Initialize the reaction using a reaction equation")
        .def("name", &Reaction::name,"Returns the name of the chemical Reaction")
        .def("symbol", &Reaction::symbol,"Returns the symbol of the Reaction")
        .def("equation", &Reaction::equation,"Returns the equation of the Reaction")
        .def("reactants", &Reaction::reactants,"Returns the map of reactants symbols and their coefficients")
        .def("thermoReferenceProperties", &Reaction::thermoReferenceProperties,"Returns the references properties of the reaction")
        .def("thermoParameters", &Reaction::thermoParameters,"Returns an instance of the defined parameters of the reaction")
        .def("referenceT", &Reaction::referenceT,"Returns the references temperature (K)")
        .def("referenceP", &Reaction::referenceP,"Returns the references pressure (Pa)")
        .def("lowerT", &Reaction::lowerT,"Return upper temperature limit of the correction method (K)")
        .def("lowerP", &Reaction::lowerP,"Return upper presure limit of the correction method (Pa)")
        .def("upperT", &Reaction::upperT,"Return lower temperature limit of the correction method (K)")
        .def("upperP", &Reaction::upperP,"Return lower pressure limit for the correction method (Pa)")
        .def("methodGenEOS", &Reaction::methodGenEOS,"Return the general EOS method code")
        .def("method_T", &Reaction::method_T,"Return the temperature correction method code")
        .def("method_P", &Reaction::method_P,"Return the pressure correction method code")
        .def("jsonString", &Reaction::jsonString, "Return the record as a json string")
        ;
}

}
