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

// pybind11 includes
#include <pybind11/pybind11.h>
namespace py = pybind11;

#include <ThermoFun/Common/ThermoScalar.hpp>
#include <ThermoFun/ThermoProperties.h>

namespace ThermoFun {

void exportThermoPropertiesSubstance(py::module& m)
{
    py::class_<ThermoPropertiesSubstance>(m, "ThermoPropertiesSubstance", "Standard molar properties of a substance")
        .def(py::init<>())
        .def_readwrite("gibbs_energy", &ThermoPropertiesSubstance::gibbs_energy)
        .def_readwrite("helmholtz_energy", &ThermoPropertiesSubstance::helmholtz_energy)
        .def_readwrite("internal_energy", &ThermoPropertiesSubstance::internal_energy)
        .def_readwrite("enthalpy", &ThermoPropertiesSubstance::enthalpy)
        .def_readwrite("entropy", &ThermoPropertiesSubstance::entropy)
        .def_readwrite("volume", &ThermoPropertiesSubstance::volume)
        .def_readwrite("heat_capacity_cp", &ThermoPropertiesSubstance::heat_capacity_cp)
        .def_readwrite("heat_capacity_cv", &ThermoPropertiesSubstance::heat_capacity_cv)
        ;
}

void exportThermoPropertiesReaction(py::module& m)
{
    py::class_<ThermoPropertiesReaction>(m, "ThermoPropertiesReaction", "Standard molar properties of a reaction")
        .def(py::init<>())
        .def_readwrite("ln_equilibrium_constant",   &ThermoPropertiesReaction::ln_equilibrium_constant)
        .def_readwrite("log_equilibrium_constant",  &ThermoPropertiesReaction::log_equilibrium_constant)
        .def_readwrite("reaction_gibbs_energy",     &ThermoPropertiesReaction::reaction_gibbs_energy)
        .def_readwrite("reaction_helmholtz_energy", &ThermoPropertiesReaction::reaction_helmholtz_energy)
        .def_readwrite("reaction_internal_energy",  &ThermoPropertiesReaction::reaction_internal_energy)
        .def_readwrite("reaction_enthalpy",         &ThermoPropertiesReaction::reaction_enthalpy)
        .def_readwrite("reaction_entropy",          &ThermoPropertiesReaction::reaction_entropy)
        .def_readwrite("reaction_volume",           &ThermoPropertiesReaction::reaction_volume)
        .def_readwrite("reaction_heat_capacity_cp", &ThermoPropertiesReaction::reaction_heat_capacity_cp)
        .def_readwrite("reaction_heat_capacity_cv", &ThermoPropertiesReaction::reaction_heat_capacity_cv)
        ;
}

void exportElectroPropertiesSolvent(py::module& m)
{
    py::class_<ElectroPropertiesSolvent>(m, "ElectroPropertiesSolvent", "Dielectric constant, its derivatives  with T and P, and Born functions")
        .def(py::init<>())
        .def_readwrite("epsilon", &ElectroPropertiesSolvent::epsilon)
        .def_readwrite("epsilonT", &ElectroPropertiesSolvent::epsilonT)
        .def_readwrite("epsilonP", &ElectroPropertiesSolvent::epsilonP)
        .def_readwrite("epsilonTT", &ElectroPropertiesSolvent::epsilonTT)
        .def_readwrite("epsilonTP", &ElectroPropertiesSolvent::epsilonTP)
        .def_readwrite("epsilonPP", &ElectroPropertiesSolvent::epsilonPP)
        .def_readwrite("bornZ", &ElectroPropertiesSolvent::bornZ)
        .def_readwrite("bornY", &ElectroPropertiesSolvent::bornY)
        .def_readwrite("bornQ", &ElectroPropertiesSolvent::bornQ)
        .def_readwrite("bornN", &ElectroPropertiesSolvent::bornN)
        .def_readwrite("bornU", &ElectroPropertiesSolvent::bornU)
        .def_readwrite("bornX", &ElectroPropertiesSolvent::bornX)
        ;
}

void exportPropertiesSolvent(py::module& m)
{
    py::class_<PropertiesSolvent>(m, "PropertiesSolvent", "Physical properties of water solvent")
        .def(py::init<>())
        .def_readwrite("speed_of_sound", &PropertiesSolvent::speed_of_sound)
        .def_readwrite("dynamic_viscosity", &PropertiesSolvent::dynamic_viscosity)
        .def_readwrite("thermal_conductivity", &PropertiesSolvent::thermal_conductivity)
        .def_readwrite("surface_tension", &PropertiesSolvent::surface_tension)
        .def_readwrite("Tdiff", &PropertiesSolvent::Tdiff)
        .def_readwrite("Prndtl", &PropertiesSolvent::Prndtl)
        .def_readwrite("Visck", &PropertiesSolvent::Visck)
        .def_readwrite("Alpha", &PropertiesSolvent::Alpha)
        .def_readwrite("dAldT", &PropertiesSolvent::dAldT)
        .def_readwrite("Beta", &PropertiesSolvent::Beta)
        .def_readwrite("Albe", &PropertiesSolvent::Albe)
        .def_readwrite("density", &PropertiesSolvent::density)
        .def_readwrite("densityT", &PropertiesSolvent::densityT)
        .def_readwrite("densityP", &PropertiesSolvent::densityP)
        .def_readwrite("densityTT", &PropertiesSolvent::densityTT)
        .def_readwrite("densityTP", &PropertiesSolvent::densityTP)
        .def_readwrite("densityPP", &PropertiesSolvent::densityPP)
        .def_readwrite("pressure", &PropertiesSolvent::pressure)
        .def_readwrite("pressureT", &PropertiesSolvent::pressureT)
        .def_readwrite("pressureD", &PropertiesSolvent::pressureD)
        .def_readwrite("pressureTT", &PropertiesSolvent::pressureTT)
        .def_readwrite("pressureTD", &PropertiesSolvent::pressureTD)
        .def_readwrite("pressureDD", &PropertiesSolvent::pressureDD)
        ;
}

} // namespace ThermoFun