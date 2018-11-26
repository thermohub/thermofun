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

#include "PyThermoFun.hpp"
using namespace ThermoFun;

PYBIND11_MODULE(PyThermoFun, m)
{
    // Common module
    exportThermoScalar(m);
    exportStatus(m);
    // Properties 
    exportThermoPropertiesSubstance(m);
    exportThermoPropertiesReaction(m);
    exportElectroPropertiesSolvent(m);
    exportPropertiesSolvent(m);
    // Core module
    exportThermoEngine(m);
    exportThermoBatch(m);
    exportOutput(m);
    // Data
    exportDatabase(m);
    exportSubstance(m);
    exportReaction(m);
    exportElement(m);

    // Database Client
    exportDatabaseClient(m);
}