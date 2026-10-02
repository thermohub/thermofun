// Tests that the derivatives with respect to temperature and pressure of the properties calculated by models
// that are not covered by the databases of the tests agree with finite differences of their values.
#include <cmath>
#include <cstdio>
#include <functional>

#include "ThermoEngine.h"
#include "ThermoModelsReaction.h"
#include "Reaction.h"
#include "ThermoModelsSubstance.h"
#include "ThermoModelsSolvent.h"
#include "ThermoProperties.h"
#include "Substance.h"
#include "ThermoParameters.h"

using namespace ThermoFun;

static int failures = 0;

static void checkDerivatives(const char* what, const std::function<Reaktoro_::ThermoProperty(double, double)>& f,
                             double T, double P, double rel = 2e-4)
{
    const double hT = 1e-3 * T, hP = 1e-3 * P;
    const auto x = f(T, P);
    const double fdT = (f(T + hT, P).val - f(T - hT, P).val) / (2 * hT);
    const double fdP = (f(T, P + hP).val - f(T, P - hP).val) / (2 * hP);
    const bool okT = std::fabs(x.ddt - fdT) <= rel * std::fabs(fdT) + 1e-12;
    const bool okP = std::fabs(x.ddp - fdP) <= rel * std::fabs(fdP) + 1e-14;
    if (!okT || !okP)
    {
        std::printf("FAILED: %s: ddt %g (fd %g) ddp %g (fd %g)\n", what, x.ddt, fdT, x.ddp, fdP);
        ++failures;
    }
}

static ThermoPropertiesSubstance inputProperties()
{
    ThermoPropertiesSubstance tps;
    tps.gibbs_energy = -1.0e6;
    tps.enthalpy = -1.1e6;
    tps.entropy = 100.0;
    tps.volume = 2.0;
    tps.heat_capacity_cp = 80.0;
    tps.heat_capacity_cv = 80.0;
    tps.internal_energy = -1.1e6;
    tps.helmholtz_energy = -1.0e6;
    return tps;
}

static Substance makeSubstance()
{
    Substance substance;
    substance.setSymbol("X");
    substance.setReferenceT(298.15);
    substance.setReferenceP(1e5);
    ThermoPropertiesSubstance ref = inputProperties();
    substance.setThermoReferenceProperties(ref);
    return substance;
}

int main(int argc, char** argv)
{
    const double T = 450.0, P = 5e7;

    // Berman (1988) volume model
    {
        auto substance = makeSubstance();
        ThermoParametersSubstance parameters;
        parameters.volume_coeff = {-1.5e-5, 2.0e-8, -1.0e-11, 5.0e-5, 2.0e-9};
        substance.setThermoParameters(parameters);
        const auto tps = inputProperties();
        MinBerman88 model(substance);
        auto f = [&](auto member) { return [=, &model](double t, double p) { return (model.thermoProperties(t, p, tps)).*member; }; };
        checkDerivatives("Berman gibbs_energy", f(&ThermoPropertiesSubstance::gibbs_energy), T, P);
        checkDerivatives("Berman enthalpy", f(&ThermoPropertiesSubstance::enthalpy), T, P);
        checkDerivatives("Berman entropy", f(&ThermoPropertiesSubstance::entropy), T, P);
        checkDerivatives("Berman volume", f(&ThermoPropertiesSubstance::volume), T, P);
        checkDerivatives("Berman internal_energy", f(&ThermoPropertiesSubstance::internal_energy), T, P);
        checkDerivatives("Berman helmholtz_energy", f(&ThermoPropertiesSubstance::helmholtz_energy), T, P);
    }

    // Birch-Murnaghan model (Gottschalk)
    {
        auto substance = makeSubstance();
        ThermoParametersSubstance parameters;
        parameters.volume_BirchM_coeff = {2.6e-5, 1.0e-8, 0.0, 0.0, 0.0, 1.0e3, -0.05, 4.0, 0.0};
        substance.setThermoParameters(parameters);
        const auto tps = inputProperties();
        MinBMGottschalk model(substance);
        auto f = [&](auto member) { return [=, &model](double t, double p) { return (model.thermoProperties(t, p, tps)).*member; }; };
        checkDerivatives("BMGottschalk gibbs_energy", f(&ThermoPropertiesSubstance::gibbs_energy), T, P, 2e-3);
        checkDerivatives("BMGottschalk entropy", f(&ThermoPropertiesSubstance::entropy), T, P, 2e-3);
        checkDerivatives("BMGottschalk volume", f(&ThermoPropertiesSubstance::volume), T, P, 2e-3);
    }

    // Constant molar volume
    {
        auto substance = makeSubstance();
        const auto tps = inputProperties();
        ConMolVol model(substance);
        auto f = [&](auto member) { return [=, &model](double t, double p) { return (model.thermoProperties(t, p, tps)).*member; }; };
        checkDerivatives("ConMolVol gibbs_energy", f(&ThermoPropertiesSubstance::gibbs_energy), T, P);
        checkDerivatives("ConMolVol helmholtz_energy", f(&ThermoPropertiesSubstance::helmholtz_energy), T, P);
    }

    // Ideal gas law volume with the pressure correction
    {
        auto substance = makeSubstance();
        substance.setSubstanceClass(SubstanceClass::type::GASFLUID);
        const auto tps = inputProperties();
        IdealGasLawVol model(substance);
        auto f = [&](auto member) { return [=, &model](double t, double p) { return (model.thermoProperties(t, p, tps, true)).*member; }; };
        checkDerivatives("IdealGasLawVol volume", f(&ThermoPropertiesSubstance::volume), T, P);
        checkDerivatives("IdealGasLawVol gibbs_energy", f(&ThermoPropertiesSubstance::gibbs_energy), T, P);
        checkDerivatives("IdealGasLawVol enthalpy", f(&ThermoPropertiesSubstance::enthalpy), T, P);
    }

    // Gas and fluid models (fugacity from the equations of state)
    {
        auto substance = makeSubstance();
        substance.setSubstanceClass(SubstanceClass::type::GASFLUID);
        substance.setFormula("CH4");
        ThermoParametersSubstance parameters;
        parameters.temperature_intervals = {{200.0, 2000.0}};
        parameters.critical_parameters = {190.56, 45.99, 0.0115, 0.0, 0.0, 0.0, 0.0};  // Tcr, Pcr, omega, ...
        substance.setThermoParameters(parameters);
        const auto tps = inputProperties();

        auto gas = [&](const char* name, auto&& model, double Tgas, double Pgas, double rel = 2e-3) {
            for (auto member : {&ThermoPropertiesSubstance::gibbs_energy, &ThermoPropertiesSubstance::enthalpy,
                                &ThermoPropertiesSubstance::entropy, &ThermoPropertiesSubstance::volume})
            {
                auto f = [&](double t, double p) { return (model.thermoProperties(t, p, tps, true)).*member; };
                checkDerivatives(name, f, Tgas, Pgas, rel);
            }
        };

        gas("SRK", GasSRK(substance), 450.0, 5e7);
        gas("PR78", GasPR78(substance), 450.0, 5e7);
        gas("PRSV", GasPRSV(substance), 450.0, 5e7);

        auto co2 = substance;
        co2.setFormula("CO2");
        gas("STP", GasSTP(co2), 600.0, 5e7);
        gas("CORK", GasCORK(co2), 600.0, 5e7);

        auto cgf = substance;
        ThermoParametersSubstance cgfParameters = parameters;
        cgfParameters.critical_parameters = {3.7327, 149.92, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        cgf.setThermoParameters(cgfParameters);
        gas("CGF", GasCGF(cgf), 450.0, 5e7);
    }

    // Reaction models that depend on the properties of the solvent (Frantz-Marshall, Dolejs-Manning)
    if (argc > 1)
    {
        ThermoEngine engine(argv[1]);
        auto waterProps = [&](double t, double p) { double pp = p; return engine.propertiesSolvent(t, pp, "H2O@"); };

        Reaction reaction;
        reaction.setReferenceT(298.15);
        reaction.setReferenceP(1e5);

        {
            ThermoParametersReaction parameters;
            parameters.reaction_FM_coeff = {-10.0, -2000.0, 1.0e5, 1.0e7, 3.0, -500.0, 1.0e4};
            reaction.setThermoParameters(parameters);
            ReactionFrantzMarshall model(reaction);
            auto f = [&](auto member) { return [=, &model](double t, double p) { return (model.thermoProperties(t, p, waterProps(t, p))).*member; }; };
            checkDerivatives("Frantz-Marshall gibbs_energy", f(&ThermoPropertiesReaction::reaction_gibbs_energy), 573.15, 5e7);
            checkDerivatives("Frantz-Marshall enthalpy", f(&ThermoPropertiesReaction::reaction_enthalpy), 573.15, 5e7);
            checkDerivatives("Frantz-Marshall entropy", f(&ThermoPropertiesReaction::reaction_entropy), 573.15, 5e7);
            checkDerivatives("Frantz-Marshall volume", f(&ThermoPropertiesReaction::reaction_volume), 573.15, 5e7);
        }
        {
            ThermoParametersReaction parameters;
            parameters.reaction_DM10_coeff = {1.0e5, -200.0, -20.0, 0.01, -5.0e3};
            reaction.setThermoParameters(parameters);
            ReactionDolejsManning10 model(reaction);
            auto f = [&](auto member) { return [=, &model](double t, double p) { return (model.thermoProperties(t, p, waterProps(t, p))).*member; }; };
            checkDerivatives("Dolejs-Manning gibbs_energy", f(&ThermoPropertiesReaction::reaction_gibbs_energy), 573.15, 5e7);
            checkDerivatives("Dolejs-Manning enthalpy", f(&ThermoPropertiesReaction::reaction_enthalpy), 573.15, 5e7);
            checkDerivatives("Dolejs-Manning entropy", f(&ThermoPropertiesReaction::reaction_entropy), 573.15, 5e7);
            checkDerivatives("Dolejs-Manning volume", f(&ThermoPropertiesReaction::reaction_volume), 573.15, 5e7);
        }
    }

    // Zhang and Duan (2005) water density
    {
        auto substance = makeSubstance();
        WaterZhangDuan2005 model(substance);
        auto f = [&](double t, double p) { return model.propertiesSolvent(t, p, 0).density; };
        checkDerivatives("Zhang-Duan density", f, 600.0, 5e8, 2e-3);
    }

    if (failures == 0) std::printf("All model derivative tests passed\n");
    return failures == 0 ? 0 : 1;
}
