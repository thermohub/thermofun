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
namespace py = pybind11;

// ThermoFun includes
#include <ThermoFun/Element.h>

namespace ThermoFun {

void exportElement(py::module& m)
{
    py::class_<Element>(m, "Element")
        .def(py::init<>())
        .def(py::init<const Element&>())
        .def(py::init<std::string>())
        .def("setName", &Element::setName, "Set the name of the element")
        .def("setSymbol", &Element::setSymbol, "Set the symbol of the element")
        .def("setMolarMass", &Element::setMolarMass, "Set the molar mass of the element (in units of g/mol)")
        .def("setEntropy", &Element::setEntropy, "Set the entropy of the element (in units of J/mol/K)")
        .def("setHeatCapacity", &Element::setHeatCapacity, "Set the heat capacity of the element (in units of J/mol/K)")
        .def("setVolume", &Element::setVolume, "Set the volume of the element (in units of cm3/mol)")
        .def("setValence", &Element::setValence, "Set the valence of the element")
        .def("setClass", &Element::setClass, "Set the class_ of the element (as in element.thrift)")
        .def("setIsotopeMass", &Element::setIsotopeMass, "Set the rounded isotopic mass of the element")
        .def("setNumber", &Element::setNumber, "Return the Mendeleev table number of the element")
        .def("setJsonString", &Element::setJsonString, "Set the record as a json string")
        .def("name", &Element::name, "Return the name of the element")
        .def("symbol", &Element::symbol, "Return the symbol of the element")
        .def("molarMass", &Element::molarMass, "Return the molar mass of the element (in units of g/mol)")
        .def("entropy", &Element::entropy, "Return the entropy of the element (in units of J/mol/K)")
        .def("heatCapacity", &Element::heatCapacity, "Return the heat capacity of the element (in units of J/mol/K)")
        .def("volume", &Element::volume, "Return the volume of the element (in units of cm3/mol)")
        .def("valence", &Element::valence, "Return the valence of the element")
        .def("class_", &Element::class_, "Return the class of the element (as in element.thrift)")
        .def("isotopeMass", &Element::isotopeMass, "Return the rounded isotopic mass of the element")
        .def("number", &Element::number, "Return the Mendeleev table number of the element")
        .def("jsonString", &Element::jsonString, "Return the record as a json string")
        ;
}

}
