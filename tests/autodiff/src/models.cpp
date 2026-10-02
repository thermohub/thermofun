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
#include "ElectroModelsSolvent.h"
#include "Database.h"
#include "ThermoProperties.h"
#include "Substance.h"
#include "Substances/Solute/SoluteHollandPowell98.h"
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

        // Holland and Powell (1998) aqueous solute model: derivatives of its own values
        {
            Substance solute = makeSubstance();
            ThermoParametersSubstance parameters;
            parameters.solute_holland_powell98_coeff = {0.2};
            solute.setThermoParameters(parameters);
            SoluteHollandPowell98 model(solute);
            auto f = [&](auto member) {
                return [=, &model](double t, double p) {
                    double Tr = 298.15, Pr = 1e5, pp = p;
                    auto wpr = engine.propertiesSolvent(Tr, Pr, "H2O@");
                    auto wp = engine.propertiesSolvent(t, pp, "H2O@");
                    return (model.thermoProperties(t, p, wpr, wp)).*member;
                };
            };
            for (double t : {400.0, 450.0, 520.0})
                for (auto m : {&ThermoPropertiesSubstance::gibbs_energy, &ThermoPropertiesSubstance::entropy,
                               &ThermoPropertiesSubstance::volume, &ThermoPropertiesSubstance::heat_capacity_cp})
                    checkDerivatives("HP98 solute", f(m), t, 5e7, 1e-3);
            // exact relations: dG/dT = -S, dG/dP = V, H = G + T S, dH/dT = Cp
            for (double t : {400.0, 450.0, 520.0})
            {
                const auto x = f(&ThermoPropertiesSubstance::gibbs_energy)(t, 5e7);
                const auto S = f(&ThermoPropertiesSubstance::entropy)(t, 5e7);
                const auto V = f(&ThermoPropertiesSubstance::volume)(t, 5e7);
                const auto H = f(&ThermoPropertiesSubstance::enthalpy)(t, 5e7);
                const auto Cp = f(&ThermoPropertiesSubstance::heat_capacity_cp)(t, 5e7);
                auto close = [](double a, double b) { return std::fabs(a - b) <= 1e-6 * (std::fabs(b) + 1.0); };
                if (!close(x.ddt, -S.val) || !close(x.ddp * 1e5, V.val) || !close(H.val, x.val + t * S.val) ||
                    !close(H.ddt, Cp.val) || !close(t * S.ddt, Cp.val))
                {
                    std::printf("FAILED: HP98 solute thermodynamic relations at T=%g\n", t);
                    ++failures;
                }
            }
        }

        // Reaction volume as a function of T and P: derivatives, thermodynamic relations and the reference state
        {
            Reaction rv;
            rv.setReferenceT(298.15);
            rv.setReferenceP(1e5);
            ThermoPropertiesReaction ref;
            ref.reaction_volume = 3.0; // J/bar
            rv.setThermoReferenceProperties(ref);
            ThermoParametersReaction parameters;
            parameters.reaction_V_fT_coeff = {1.0e-4, 2.0e-7, 1.0e-10, -4.0e-5, 1.0e-9};
            rv.setThermoParameters(parameters);
            Reaction_Vol_fT model(rv);

            ThermoPropertiesReaction in;
            in.reaction_gibbs_energy = -5.0e4; in.reaction_enthalpy = -6.0e4; in.reaction_entropy = 30.0;
            in.reaction_heat_capacity_cp = 120.0; in.reaction_heat_capacity_cv = 100.0;
            in.ln_equilibrium_constant = 20.0; in.log_equilibrium_constant = 8.7;
            in.reaction_internal_energy = -6.0e4; in.reaction_helmholtz_energy = -5.0e4;
            in.reaction_volume = 3.0;
            auto f = [&](auto member) { return [=, &model](double t, double p) { return (model.thermoProperties(t, p, in)).*member; }; };
            const double tt = 450.0, pp = 5e7;
            for (auto m : {&ThermoPropertiesReaction::reaction_gibbs_energy, &ThermoPropertiesReaction::reaction_enthalpy,
                           &ThermoPropertiesReaction::reaction_entropy, &ThermoPropertiesReaction::reaction_volume,
                           &ThermoPropertiesReaction::reaction_heat_capacity_cp, &ThermoPropertiesReaction::ln_equilibrium_constant})
                checkDerivatives("Reaction_Vol_fT", f(m), tt, pp);

            const auto x = model.thermoProperties(tt, pp, in);
            auto rel = [](double a, double b) { return std::fabs(a - b) <= 1e-9 * (std::fabs(b) + 1.0); };
            // the input has no derivatives: dG/dP = V (J/bar per Pa), dG/dT + S = S(input), H = G + T S - T S(input)
            if (!rel(x.reaction_gibbs_energy.ddp * 1e5, x.reaction_volume.val) ||
                !rel(x.reaction_gibbs_energy.ddt + x.reaction_entropy.val, in.reaction_entropy.val) ||
                !rel(x.reaction_entropy.ddp, -x.reaction_volume.ddt * 1e-5))
            {
                std::printf("FAILED: Reaction_Vol_fT thermodynamic relations\n");
                ++failures;
            }
            const auto r = model.thermoProperties(298.15, 1e5, in); // at the reference state nothing changes
            if (!rel(r.reaction_gibbs_energy.val, in.reaction_gibbs_energy.val) || !rel(r.reaction_volume.val, 3.0))
            {
                std::printf("FAILED: Reaction_Vol_fT reference state\n");
                ++failures;
            }
        }
    }

    // Zhang and Duan (2005) water: the derivatives of the density up to the third order are exact (no finite differences)
    for (auto tp : {std::make_pair(400.0, 3e8), std::make_pair(600.0, 5e8), std::make_pair(800.0, 1e9)})
    {
        auto substance = makeSubstance();
        WaterZhangDuan2005 model(substance);
        auto ps = [&](double t, double p) { return model.propertiesSolvent(t, p, 0); };
        auto f = [&](auto member) { return [=](double t, double p) { return ps(t, p).*member; }; };
        const double t = tp.first, p = tp.second;
        for (auto m : {&PropertiesSolvent::density, &PropertiesSolvent::densityT, &PropertiesSolvent::densityP,
                       &PropertiesSolvent::densityTT, &PropertiesSolvent::densityTP, &PropertiesSolvent::densityPP})
            checkDerivatives("Zhang-Duan density derivative", f(m), t, p, 2e-4);
        // the first and second derivatives are the derivatives of the density
        const auto x = ps(t, p);
        auto close = [](double a, double b, double rel) { return std::fabs(a - b) <= rel * (std::fabs(b) + 1e-300); };
        if (!close(x.density.ddt, x.densityT.val, 1e-12) || !close(x.density.ddp, x.densityP.val, 1e-12) ||
            !close(x.densityT.ddt, x.densityTT.val, 1e-12) || !close(x.densityP.ddp, x.densityPP.val, 1e-12) ||
            // the mixed partial derivatives are symmetric: exact for exact derivatives
            !close(x.densityT.ddp, x.densityP.ddt, 1e-12) || !close(x.densityTP.val, x.densityT.ddp, 1e-12) ||
            !close(x.densityTT.ddp, x.densityTP.ddt, 1e-12) || !close(x.densityTP.ddp, x.densityPP.ddt, 1e-12))
        {
            std::printf("FAILED: Zhang-Duan derivatives of the density are not exact (T=%g P=%g)\n", t, p);
            ++failures;
        }
    }

    // Dielectric constant of Sverjensky et al. (2014) and Fernandez et al. (1997): exact derivatives of the closed form
    // function of T and of the density of the solvent, with the derivatives of the density of the solvent model
    if (argc > 1)
    {
        ThermoEngine engine(argv[1]);
        Database database(argv[1]);
        const auto water = database.getSubstance("H2O@");
        WaterElectroSverjensky2014 sverjensky(water);
        WaterElectroFernandez1997 fernandez(water);
        for (auto tp : {std::make_pair(450.0, 5e7), std::make_pair(650.0, 3e8), std::make_pair(300.0, 1e6)})
        {
            const double t = tp.first, p = tp.second;
            auto check = [&](const char* name, auto& model) {
                auto e = [&](double tt, double pp) { return model.electroPropertiesSolvent(tt, pp, 0); };
                auto f = [&](auto member) { return [=](double tt, double pp) { return e(tt, pp).*member; }; };
                for (auto m : {&ElectroPropertiesSolvent::epsilon, &ElectroPropertiesSolvent::epsilonT, &ElectroPropertiesSolvent::epsilonP,
                               &ElectroPropertiesSolvent::epsilonTT, &ElectroPropertiesSolvent::epsilonPP})
                    checkDerivatives(name, f(m), t, p, 2e-4);
                for (auto m : {&ElectroPropertiesSolvent::bornZ, &ElectroPropertiesSolvent::bornY, &ElectroPropertiesSolvent::bornQ, &ElectroPropertiesSolvent::bornX})
                    checkDerivatives(name, f(m), t, p, 2e-4);
                const auto x = e(t, p);
                auto close = [](double a, double b, double rel) { return std::fabs(a - b) <= rel * (std::fabs(b) + 1e-300); };
                // epsilonT and epsilonTT are the derivatives of epsilon, epsilonP (per bar) of epsilon (per Pa), the mixed derivatives are symmetric
                if (!close(x.epsilon.ddt, x.epsilonT.val, 1e-12) || !close(x.epsilon.ddp * 1e5, x.epsilonP.val, 1e-12) ||
                    !close(x.epsilonT.ddt, x.epsilonTT.val, 1e-12) || !close(x.epsilonP.ddp * 1e5, x.epsilonPP.val, 1e-12) ||
                    !close(x.epsilonT.ddp * 1e5, x.epsilonP.ddt, 1e-12) || !close(x.epsilonTP.val, x.epsilonP.ddt, 1e-12))
                {
                    std::printf("FAILED: %s derivatives of the dielectric constant are not exact (T=%g P=%g)\n", name, t, p);
                    ++failures;
                }
            };
            check("Sverjensky dielectric constant", sverjensky);
            check("Fernandez dielectric constant", fernandez);

            // Sverjensky: the closed form chain rule of ln(epsilon) = u(T) + w(T) ln(rho) gives epsilonT and epsilonP
            double pp = p;
            const auto rho = engine.propertiesSolvent(t, pp, "H2O@", 0);
            const double tc = t - 273.15, r = rho.density.val / 1000.0;
            const double u1 = -8.016651e-05 + 0.5 * -6.871618e-02 / std::sqrt(tc);
            const double w0 = -1.576377e-03 * tc + 6.810288e-02 * std::sqrt(tc) + 7.548755e-01;
            const double w1 = -1.576377e-03 + 0.5 * 6.810288e-02 / std::sqrt(tc);
            const auto s = sverjensky.electroPropertiesSolvent(t, p, 0);
            const double LT = u1 + w1 * std::log(r) + w0 * rho.densityT.val / rho.density.val;
            const double LP = w0 * rho.densityP.val / rho.density.val;
            if (std::fabs(s.epsilonT.val - s.epsilon.val * LT) > 1e-9 * std::fabs(s.epsilonT.val) ||
                std::fabs(s.epsilonP.val - s.epsilon.val * LP * 1e5) > 1e-9 * std::fabs(s.epsilonP.val))
            {
                std::printf("FAILED: Sverjensky epsilonT, epsilonP differ from the closed form chain rule (T=%g P=%g)\n", t, p);
                ++failures;
            }
        }
    }

    if (failures == 0) std::printf("All model derivative tests passed\n");
    return failures == 0 ? 0 : 1;
}
