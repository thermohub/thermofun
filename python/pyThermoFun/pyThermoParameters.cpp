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

#include <ThermoFun/ThermoParameters.h>

namespace ThermoFun {

void exportThermoParametersSubstance(py::module& m)
{
    py::class_<ThermoParametersSubstance>(m, "ThermoParametersSubstance")
        .def(py::init<>())
        .def_readwrite("isothermal_compresibility", &ThermoParametersSubstance::isothermal_compresibility)
        .def_readwrite("isobaric_expansivity", &ThermoParametersSubstance::isobaric_expansivity)
        .def_readwrite("temperature_intervals", &ThermoParametersSubstance::temperature_intervals)
        .def_readwrite("pressure_intervals", &ThermoParametersSubstance::pressure_intervals)
        .def_readwrite("Cp_coeff", &ThermoParametersSubstance::Cp_coeff)
        .def_readwrite("Cp_nonElectrolyte_coeff", &ThermoParametersSubstance::Cp_nonElectrolyte_coeff)
        .def_readwrite("phase_transition_prop", &ThermoParametersSubstance::phase_transition_prop)
        .def_readwrite("phase_transition_prop_Berman", &ThermoParametersSubstance::phase_transition_prop_Berman)
        .def_readwrite("m_landau_phase_trans_props", &ThermoParametersSubstance::m_landau_phase_trans_props)
        .def_readwrite("HKF_parameters", &ThermoParametersSubstance::HKF_parameters)
        .def_readwrite("volume_coeff", &ThermoParametersSubstance::volume_coeff)
        .def_readwrite("critical_parameters", &ThermoParametersSubstance::critical_parameters)
        .def_readwrite("volume_BirchM_coeff", &ThermoParametersSubstance::volume_BirchM_coeff)
        .def_readwrite("empirical_coeff", &ThermoParametersSubstance::empirical_coeff)
        .def_readwrite("solute_holland_powell98_coeff", &ThermoParametersSubstance::solute_holland_powell98_coeff)        
        ;
}

void exportThermoParametersReaction(py::module& m)
{
    py::class_<ThermoParametersReaction>(m, "ThermoParametersReaction")
        .def(py::init<>())
        .def_readwrite("temperature_intervals", &ThermoParametersReaction::temperature_intervals)
        .def_readwrite("pressure_intervals", &ThermoParametersReaction::pressure_intervals)
        .def_readwrite("reaction_logK_fT_coeff", &ThermoParametersReaction::reaction_logK_fT_coeff)
        .def_readwrite("logK_TP_array", &ThermoParametersReaction::logK_TP_array)
        .def_readwrite("reaction_Cp_fT_coeff", &ThermoParametersReaction::reaction_Cp_fT_coeff)
        .def_readwrite("reaction_V_fT_coeff", &ThermoParametersReaction::reaction_V_fT_coeff)
        .def_readwrite("reaction_RB_coeff", &ThermoParametersReaction::reaction_RB_coeff)
        .def_readwrite("reaction_FM_coeff", &ThermoParametersReaction::reaction_FM_coeff)
        .def_readwrite("reaction_DM10_coeff", &ThermoParametersReaction::reaction_DM10_coeff)
        ;      
}

} // namespace ThermoFun