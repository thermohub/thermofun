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

#pragma once

#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

// pybind11 includes
#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace ThermoFun {
    // Common module
    void exportThermoScalar(py::module& m);
    void exportStatus(py::module& m);
    // Properties
    void exportThermoPropertiesSubstance(py::module& m);
    void exportThermoPropertiesReaction(py::module& m);
    void exportElectroPropertiesSolvent(py::module& m);
    void exportPropertiesSolvent(py::module& m);
    // Parameters
    void exportThermoParametersReaction(py::module& m);
    void exportThermoParametersSubstance(py::module& m);
    // Core module
    void exportThermoEngine(py::module& m);
    void exportThermoBatch(py::module& m);
    void exportBatchPreferences(py::module& m);
    void exportOutput(py::module& m);
    // Data
    void exportDatabase(py::module& m);
    void exportSubstance(py::module& m);
    void exportReaction(py::module& m);
    void exportElement(py::module& m);
} // namespace ThermoFun
