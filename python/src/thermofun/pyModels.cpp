// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Python bindings of the calculation models of ThermoFun (the models of the substances, reactions, solvents and of
// the electro-chemical properties of the solvent). The ThermoEngine selects and calls them from the records of the
// database; here they can be used directly, with a Substance or a Reaction as the source of the parameters.

#if _MSC_VER >= 1929
#include <corecrt.h>
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include <ThermoFun/Substance.h>
#include <ThermoFun/Reaction.h>
#include <ThermoFun/ThermoProperties.h>
#include <ThermoFun/ThermoModelsSubstance.h>
#include <ThermoFun/ThermoModelsSolvent.h>
#include <ThermoFun/ElectroModelsSolvent.h>
#include <ThermoFun/ThermoModelsReaction.h>

namespace ThermoFun {

namespace {

/// A model that is constructed from a Substance (or a Reaction)
template<class Model, class Source>
auto bindModel(py::module& m, const char* name, const char* doc) -> py::class_<Model>
{
    return py::class_<Model>(m, name, doc)
        .def(py::init<const Source&>(), py::arg("source"));
}

const char* tpDoc = "T in K, P in Pa";

/// substance models: thermoProperties(T, P)
template<class Model>
void substanceModelTP(py::module& m, const char* name, const char* doc)
{
    bindModel<Model, Substance>(m, name, doc)
        .def("thermoProperties", [](Model& self, double T, double P) { return self.thermoProperties(T, P); },
             py::arg("T"), py::arg("P"), tpDoc);
}

/// substance models that increment the properties given as input: thermoProperties(T, P, tps)
template<class Model>
void substanceModelTPtps(py::module& m, const char* name, const char* doc)
{
    bindModel<Model, Substance>(m, name, doc)
        .def("thermoProperties", [](Model& self, double T, double P, ThermoPropertiesSubstance tps) { return self.thermoProperties(T, P, tps); },
             py::arg("T"), py::arg("P"), py::arg("tps"), "tps: the properties of the substance (ideal gas, or at the reference pressure) that are corrected; T in K, P in Pa");
}

/// gases and fluids: thermoProperties(T, P, tps, apply_pressure_correction)
template<class Model>
void substanceModelGas(py::module& m, const char* name, const char* doc)
{
    bindModel<Model, Substance>(m, name, doc)
        .def("thermoProperties", [](Model& self, double T, double P, ThermoPropertiesSubstance tps, bool apply_p) { return self.thermoProperties(T, P, tps, apply_p); },
             py::arg("T"), py::arg("P"), py::arg("tps"), py::arg("apply_pressure_correction") = false,
             "tps: the properties of the ideal gas; apply_pressure_correction: correct the Gibbs energy and enthalpy for the pressure; T in K, P in Pa");
}

/// reaction models that depend on the properties of the solvent
template<class Model>
void reactionModelSolvent(py::module& m, const char* name, const char* doc)
{
    bindModel<Model, Reaction>(m, name, doc)
        .def("thermoProperties", [](Model& self, double T, double P, PropertiesSolvent wp) { return self.thermoProperties(T, P, wp); },
             py::arg("T"), py::arg("P"), py::arg("solvent_properties"), "T in K, P in Pa");
}

} // namespace

void exportModels(py::module& m)
{
    // models of substances ------------------------------------------------------------------------------------
    substanceModelTP<ThermoModelsSubstance>(m, "ThermoModelsSubstance", "Properties of a substance calculated with the models of its record");
    substanceModelTP<WaterIdealGasWoolley>(m, "WaterIdealGasWoolley", "Ideal gas properties of water (Woolley 1980)");
    substanceModelTP<EmpiricalCpIntegration>(m, "EmpiricalCpIntegration", "Integration of the empirical heat capacity equation Cp = f(T)");
    substanceModelTP<EntropyCpIntegration>(m, "EntropyCpIntegration", "Standard entropy and heat capacity integration");

    substanceModelTPtps<MinMurnaghanEOSHP98>(m, "MinMurnaghanEOSHP98", "Murnaghan equation of state of Holland and Powell (1998)");
    substanceModelTPtps<MinBerman88>(m, "MinBerman88", "Volume of minerals, Berman (1988)");
    substanceModelTPtps<MinBMGottschalk>(m, "MinBMGottschalk", "Birch-Murnaghan equation of state, Gottschalk (1997)");
    substanceModelTPtps<HPLandau>(m, "HPLandau", "Landau phase transition of Holland and Powell (1998)");
    substanceModelTPtps<ConMolVol>(m, "ConMolVol", "Constant molar volume");

    substanceModelGas<GasCORK>(m, "GasCORK", "Compensated Redlich-Kwong equation of state (Holland and Powell 1991, 1998)");
    substanceModelGas<GasPRSV>(m, "GasPRSV", "Peng-Robinson-Stryjek-Vera equation of state");
    substanceModelGas<GasCGF>(m, "GasCGF", "Churakov-Gottschalk equation of state");
    substanceModelGas<GasSRK>(m, "GasSRK", "Soave-Redlich-Kwong equation of state");
    substanceModelGas<GasPR78>(m, "GasPR78", "Peng-Robinson (1978) equation of state");
    substanceModelGas<GasSTP>(m, "GasSTP", "Sterner-Pitzer equation of state");
    substanceModelGas<IdealGasLawVol>(m, "IdealGasLawVol", "Molar volume from the ideal gas law");

    bindModel<SoluteHKFgems, Substance>(m, "SoluteHKFgems", "Helgeson-Kirkham-Flowers equation of state of aqueous solutes (GEMS implementation)")
        .def("thermoProperties", [](SoluteHKFgems& self, double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) { return self.thermoProperties(T, P, wp, wes); },
             py::arg("T"), py::arg("P"), py::arg("solvent_properties"), py::arg("electro_properties"), tpDoc);
    bindModel<SoluteHKFreaktoro, Substance>(m, "SoluteHKFreaktoro", "Helgeson-Kirkham-Flowers equation of state of aqueous solutes (Reaktoro implementation)")
        .def("thermoProperties", [](SoluteHKFreaktoro& self, double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) { return self.thermoProperties(T, P, wp, wes); },
             py::arg("T"), py::arg("P"), py::arg("solvent_properties"), py::arg("electro_properties"), tpDoc);
    bindModel<SoluteHollandPowell98, Substance>(m, "SoluteHollandPowell98", "Aqueous solute model of Holland and Powell (1998)")
        .def("thermoProperties", [](SoluteHollandPowell98& self, double T, double P, PropertiesSolvent wpr, PropertiesSolvent wp) { return self.thermoProperties(T, P, wpr, wp); },
             py::arg("T"), py::arg("P"), py::arg("reference_solvent_properties"), py::arg("solvent_properties"), tpDoc);
    bindModel<SoluteAnderson91, Substance>(m, "SoluteAnderson91", "Modified Ryzhenko-Bryzgalin / Anderson (1991) aqueous solute model")
        .def("thermoProperties", [](SoluteAnderson91& self, double T, double P, PropertiesSolvent wpr, PropertiesSolvent wp) { return self.thermoProperties(T, P, wpr, wp); },
             py::arg("T"), py::arg("P"), py::arg("reference_solvent_properties"), py::arg("solvent_properties"), tpDoc);
    bindModel<SoluteAkinfievDiamondEOS, Substance>(m, "SoluteAkinfievDiamondEOS", "Akinfiev and Diamond (2003) equation of state of aqueous nonelectrolytes")
        .def("thermoProperties",
             [](SoluteAkinfievDiamondEOS& self, double T, double P, ThermoPropertiesSubstance tps, const ThermoPropertiesSubstance& wtp, const ThermoPropertiesSubstance& wigp,
                const PropertiesSolvent& wp, const ThermoPropertiesSubstance& wtpr, const ThermoPropertiesSubstance& wigpr, const PropertiesSolvent& wpr) {
                 return self.thermoProperties(T, P, tps, wtp, wigp, wp, wtpr, wigpr, wpr);
             },
             py::arg("T"), py::arg("P"), py::arg("tps"), py::arg("water_tp"), py::arg("water_ideal_gas_tp"), py::arg("solvent_properties"),
             py::arg("reference_water_tp"), py::arg("reference_water_ideal_gas_tp"), py::arg("reference_solvent_properties"),
             "tps: the properties of the ideal gas; the water properties at T, P and at the reference state; T in K, P in Pa");

    // models of the solvent -----------------------------------------------------------------------------------
    // the pressure is an input here; for the saturation line (P = 0) use the ThermoEngine
    bindModel<WaterHGK, Substance>(m, "WaterHGK", "Haar-Gallagher-Kell (1984) equation of state of water (GEMS implementation)")
        .def("propertiesSolvent", [](WaterHGK& self, double T, double P, int state, std::string triple) { return self.propertiesSolvent(T, P, state, triple); },
             py::arg("T"), py::arg("P"), py::arg("state") = 0, py::arg("triple") = "NEA_HGK", "state: liquid 0, vapor 1; T in K, P in Pa")
        .def("thermoPropertiesSubstance", [](WaterHGK& self, double T, double P, int state, std::string triple) { return self.thermoPropertiesSubstance(T, P, state, triple); },
             py::arg("T"), py::arg("P"), py::arg("state") = 0, py::arg("triple") = "NEA_HGK");
    bindModel<WaterHGKreaktoro, Substance>(m, "WaterHGKreaktoro", "Haar-Gallagher-Kell (1984) equation of state of water (Reaktoro implementation)")
        .def("propertiesSolvent", [](WaterHGKreaktoro& self, double T, double P, int state) { return self.propertiesSolvent(T, P, state); },
             py::arg("T"), py::arg("P"), py::arg("state") = 0, "state: liquid 0, vapor 1; T in K, P in Pa")
        .def("thermoPropertiesSubstance", [](WaterHGKreaktoro& self, double T, double P, int state, std::string triple) { return self.thermoPropertiesSubstance(T, P, state, triple); },
             py::arg("T"), py::arg("P"), py::arg("state") = 0, py::arg("triple") = "NEA_HGK");
    bindModel<WaterWP95reaktoro, Substance>(m, "WaterWP95reaktoro", "Wagner and Pruss (1995) IAPWS equation of state of water (Reaktoro implementation)")
        .def("propertiesSolvent", [](WaterWP95reaktoro& self, double T, double P, int state) { return self.propertiesSolvent(T, P, state); },
             py::arg("T"), py::arg("P"), py::arg("state") = 0, "state: liquid 0, vapor 1; T in K, P in Pa")
        .def("thermoPropertiesSubstance", [](WaterWP95reaktoro& self, double T, double P, int state, std::string triple) { return self.thermoPropertiesSubstance(T, P, state, triple); },
             py::arg("T"), py::arg("P"), py::arg("state") = 0, py::arg("triple") = "NEA_HGK");
    bindModel<WaterZhangDuan2005, Substance>(m, "WaterZhangDuan2005", "Zhang and Duan (2005) equation of state of water")
        .def("propertiesSolvent", [](WaterZhangDuan2005& self, double T, double P, int state) { return self.propertiesSolvent(T, P, state); },
             py::arg("T"), py::arg("P"), py::arg("state") = 0, tpDoc)
        .def("thermoPropertiesSubstance", [](WaterZhangDuan2005& self, double T, double P, int state) { return self.thermoPropertiesSubstance(T, P, state); },
             py::arg("T"), py::arg("P"), py::arg("state") = 0, tpDoc);

    // electro-chemical properties of the solvent ----------------------------------------------------------------
    bindModel<WaterJNreaktoro, Substance>(m, "WaterJNreaktoro", "Johnson and Norton (1991) dielectric constant of water (Reaktoro implementation)")
        .def("electroPropertiesSolvent", [](WaterJNreaktoro& self, double T, double P, PropertiesSolvent ps, int state) { return self.electroPropertiesSolvent(T, P, ps, state); },
             py::arg("T"), py::arg("P"), py::arg("solvent_properties"), py::arg("state") = 0, tpDoc);
    bindModel<WaterJNgems, Substance>(m, "WaterJNgems", "Johnson and Norton (1991) dielectric constant of water (GEMS implementation)")
        .def("electroPropertiesSolvent", [](WaterJNgems& self, double T, double P, int state) { return self.electroPropertiesSolvent(T, P, state); },
             py::arg("T"), py::arg("P"), py::arg("state") = 0, tpDoc);
    bindModel<WaterElectroSverjensky2014, Substance>(m, "WaterElectroSverjensky2014", "Sverjensky et al. (2014) dielectric constant of water")
        .def("electroPropertiesSolvent", [](WaterElectroSverjensky2014& self, double T, double P, int state) { return self.electroPropertiesSolvent(T, P, state); },
             py::arg("T"), py::arg("P"), py::arg("state") = -1, tpDoc);
    bindModel<WaterElectroFernandez1997, Substance>(m, "WaterElectroFernandez1997", "Fernandez et al. (1997) dielectric constant of water")
        .def("electroPropertiesSolvent", [](WaterElectroFernandez1997& self, double T, double P, int state) { return self.electroPropertiesSolvent(T, P, state); },
             py::arg("T"), py::arg("P"), py::arg("state") = -1, tpDoc);

    // models of reactions -----------------------------------------------------------------------------------------
    reactionModelSolvent<ReactionDolejsManning10>(m, "ReactionDolejsManning10", "Dolejs and Manning (2010) density model of the reaction equilibrium constant");
    reactionModelSolvent<ReactionFrantzMarshall>(m, "ReactionFrantzMarshall", "Marshall and Franck / Frantz and Marshall density model of the reaction equilibrium constant");
    reactionModelSolvent<ReactionRyzhenkoBryzgalin>(m, "ReactionRyzhenkoBryzgalin", "Modified Ryzhenko-Bryzgalin model of the reaction equilibrium constant");
    bindModel<Reaction_LogK_fT, Reaction>(m, "Reaction_LogK_fT", "Reaction properties from logK as a function of T (1, 2, 3 term extrapolations, Cp(T), logK(T) coefficients)")
        .def("thermoProperties", [](Reaction_LogK_fT& self, double T, double P, MethodCorrT_Thrift::type method) { return self.thermoProperties(T, P, method); },
             py::arg("T"), py::arg("P"), py::arg("method_T"), "method_T: the temperature correction method (MethodCorrT_Thrift); T in K, P in Pa");
    bindModel<Reaction_Vol_fT, Reaction>(m, "Reaction_Vol_fT", "Reaction volume as a polynomial of T and P: corrects the properties of the reaction at the reference pressure")
        .def("thermoProperties", [](Reaction_Vol_fT& self, double T, double P, const ThermoPropertiesReaction& tpr) { return self.thermoProperties(T, P, tpr); },
             py::arg("T"), py::arg("P"), py::arg("tpr"), "tpr: the properties of the reaction at the reference pressure; T in K, P in Pa");
    bindModel<ReactionFromReactantsProperties, Reaction>(m, "ReactionFromReactantsProperties", "Properties of a reaction calculated from the properties of its reactants")
        .def("thermoProperties",
             [](ReactionFromReactantsProperties& self, double T, double P, const std::vector<std::pair<ThermoPropertiesSubstance, double>>& components, const std::vector<std::string>& symbols) {
                 return self.thermoProperties(T, P, components, symbols);
             },
             py::arg("T"), py::arg("P"), py::arg("components"), py::arg("symbols"),
             "components: list of (properties of the reactant, stoichiometric coefficient); symbols: the symbols of the reactants; T in K, P in Pa");
}

} // namespace ThermoFun
