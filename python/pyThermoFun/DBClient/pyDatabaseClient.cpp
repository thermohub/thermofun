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
#include <thermofun/DBClient/DatabaseClient.h>
#include <thermofun/Common/ParseBsonTraversalData.h>
#include <thermofun/Database.h>


namespace ThermoFun {

void exportDatabaseClient(py::module& m)
{
//    auto databaseFromRecordList1 = static_cast<Database(Database::*)(DatabaseClient, List_VertexId_VertexType)>
//                                        (&databaseFromRecordList);
    py::class_<DatabaseClient>(m, "DatabaseClient")
        .def(py::init<>())
        .def("recordsFromThermoDataSet", &DatabaseClient::recordsFromThermoDataSet,"Get database records belonging to given ThermoDataSet symbol")
        ;

    m.def("setDatabaseConnectionFilePath", &setDatabaseConnectionFilePath,"Set path to the database connection preferences file.");
//    m.def("databaseFromRecordList", &databaseFromRecordList,"Returns a ThermoFun database given a record list and database client");

    m.def("databaseFromRecordList", (Database (*)(DatabaseClient, List_VertexId_VertexType) ) &databaseFromRecordList,
    "Returns a ThermoFun database given a record list and database client", pybind11::arg("databaseclient"), pybind11::arg("records"));
}
}
