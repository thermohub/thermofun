// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Tests that the derivatives with respect to temperature and pressure of the properties calculated by methods
// that are not covered by the databases of the tests agree with finite differences of their values.
#include <cmath>
#include <cstdio>
#include <functional>

#include "ThermoEngine.h"
#include "Database.h"
#include "ThermoProperties.h"
#include "Engine/Executor.h"
#include "Methods/Fluids/FluidMethods.h"
#include "Methods/Minerals/MineralMethods.h"
#include "Methods/Minerals/ReferenceMethods.h"
#include "Methods/Reactions/ReactionMethods.h"
#include "Methods/Solutes/SoluteMethods.h"
#include "Methods/Water/WaterMethods.h"

using namespace ThermoFun;
using namespace ThermoFun::v2;

static int failures = 0;

template <typename Function>
static void checkDerivatives(const char* what, const Function& f, double T, double P, double rel = 2e-4)
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

/// The reference values of all substances here: V is 2 J/bar.
static auto referenceValues() -> ReferenceValues
{
    ReferenceValues r;
    r.T = 298.15;
    r.P = 1e5;
    r.G = -1.0e6;
    r.H = -1.1e6;
    r.S = 100.0;
    r.Cp = 80.0;
    r.V = 2e-5;
    return r;
}

/// A method alone at T, P. The volume is in J/bar, to compare it with a tolerance of that size.
static auto run(std::shared_ptr<const SubstanceMethod> method, double T, double P, bool pressureCorrection = false, const SolventInputs* solvent = nullptr) -> MethodResult
{
    BoundSubstance bound;
    bound.order = {std::move(method)};
    bound.reference = referenceValues();
    SubstanceOptions options;
    options.apply_pressure_correction = pressureCorrection;
    auto out = executeSubstance(bound, T, P, options, nullptr, solvent).values;
    out.V.val *= 1e5;
    out.V.ddt *= 1e5;
    out.V.ddp *= 1e5;
    return out;
}

/// The five properties of a method, each against finite differences.
static void checkProperties(const char* what, const std::shared_ptr<const SubstanceMethod>& method, double T, double P, double rel = 2e-4, bool pressureCorrection = false)
{
    for (auto member : {&MethodResult::G, &MethodResult::H, &MethodResult::S, &MethodResult::V})
        checkDerivatives(what, [&](double t, double p) { return run(method, t, p, pressureCorrection).*member; }, T, P, rel);
}

static auto reactionInputs(std::vector<double> coefficients) -> ReactionInputs
{
    ReactionInputs in;
    in.Tr = 298.15;
    in.Pr = 1e5;
    in.coefficients.assign(coefficients.begin(), coefficients.end());
    return in;
}

int main(int argc, char** argv)
{
    const double T = 450.0, P = 5e7;

    // Berman (1988) volume model: b1 in 1/bar, b2 in 1/bar2
    checkProperties("Berman", mineralMethod("VolumeBerman88", {-1.5e-5, 2.0e-8, -1.0e-11, 5.0e-5 / 1e5, 2.0e-9 / 1e10}, 0.0, 0.0, {}), T, P);

    // Birch-Murnaghan model (Gottschalk): kt00 and dkdt in MPa, kpp in 1/MPa
    for (auto member : {&MethodResult::G, &MethodResult::S, &MethodResult::V})
    {
        const auto method = mineralMethod("BirchMurnaghan47", {2.6e-5, 1.0e-8, 0.0, 0.0, 0.0, 1.0e3 / 1e-6, -0.05 / 1e-6, 4.0, 0.0}, 0.0, 0.0, {});
        checkDerivatives("BMGottschalk", [&](double t, double p) { return run(method, t, p).*member; }, T, P, 2e-3);
    }

    // Constant molar volume
    checkDerivatives("ConstantVolume G", [&](double t, double p) { return run(constantVolumeMethod(), t, p).G; }, T, P);

    // Ideal gas law volume with the pressure correction
    checkProperties("IdealGas", idealGasMethod(), T, P, 2e-4, true);

    // Gas and fluid models (fugacity from the equations of state). Pc in Pa
    {
        const auto gas = [](const char* id, std::vector<double> parameters, const char* formula) {
            FluidRecord record;
            record.lower_T = 200.0;
            record.formula = formula;
            return fluidMethod(id, std::move(parameters), record);
        };
        const std::vector<double> cubic = {190.56, 45.99e5, 0.0115, 0.0}, interaction = {190.56, 45.99e5, 0.0115, 0.0, 0.0, 0.0};
        checkProperties("SRK", gas("SoaveRedlichKwong72", cubic, "CH4"), 450.0, 5e7, 2e-3, true);
        checkProperties("PR78", gas("PengRobinson78", cubic, "CH4"), 450.0, 5e7, 2e-3, true);
        checkProperties("PRSV", gas("PRSV86", interaction, "CH4"), 450.0, 5e7, 2e-3, true);
        checkProperties("STP", gas("SternerPitzer94", {190.56, 45.99e5}, "CO2"), 600.0, 5e7, 2e-3, true);
        checkProperties("CORK", gas("CorkHP91", interaction, "CO2"), 600.0, 5e7, 2e-3, true);

        // exact derivatives of the free energy (no finite differences): tight tolerance; polar fluid (dipole and induced dipole)
        const auto cgf = gas("ChurakovGottschalk03", {3.7327, 149.92, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, "CH4");
        checkProperties("CGF", cgf, 450.0, 5e7, 2e-5, true);
        checkProperties("CGF", cgf, 700.0, 2e8, 2e-5, true);
        checkProperties("CGF polar", gas("ChurakovGottschalk03", {3.0, 300.0, 1.2, 1.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, "CH4"), 600.0, 1e8, 2e-5, true);
    }

    // Reaction methods that depend on the properties of the solvent (Frantz-Marshall, Dolejs-Manning)
    if (argc > 1)
    {
        ThermoEngine engine(argv[1]);
        auto waterProps = [&](double t, double p) { double pp = p; return engine.propertiesSolvent(t, pp, "H2O@"); };

        const auto check = [&](const char* name, const std::shared_ptr<const ReactionMethod>& method) {
            for (auto member : {&ThermoPropertiesReaction::reaction_gibbs_energy, &ThermoPropertiesReaction::reaction_enthalpy,
                                &ThermoPropertiesReaction::reaction_entropy, &ThermoPropertiesReaction::reaction_volume})
                checkDerivatives(name, [&](double t, double p) { const auto w = waterProps(t, p); return method->calculate(t, p, {}, &w).*member; }, 573.15, 5e7);
        };
        check("Frantz-Marshall", makeMarshallFranck81(reactionInputs({-10.0, -2000.0, 1.0e5, 1.0e7, 3.0, -500.0, 1.0e4})));
        check("Dolejs-Manning", makeDolejsManning10(reactionInputs({1.0e5, -200.0, -20.0, 0.01, -5.0e3})));

        // Holland and Powell (1998) aqueous solute model: derivatives of its own values. b in J/(mol K2)
        {
            const auto method = soluteHP98Method({0.2 * 1e3}, "X");
            auto f = [&](auto member) {
                return [=, &engine](double t, double p) {
                    double Tr = 298.15, Pr = 1e5, pp = p;
                    SolventInputs solvent;
                    solvent.properties_at_reference = engine.propertiesSolvent(Tr, Pr, "H2O@");
                    solvent.properties = engine.propertiesSolvent(t, pp, "H2O@");
                    return run(method, t, p, false, &solvent).*member;
                };
            };
            for (double t : {400.0, 450.0, 520.0})
                for (auto m : {&MethodResult::G, &MethodResult::S, &MethodResult::V, &MethodResult::Cp})
                    checkDerivatives("HP98 solute", f(m), t, 5e7, 1e-3);
            // exact relations: dG/dT = -S, dG/dP = V, H = G + T S + constant (G is apparent), dH/dT = Cp
            for (double t : {400.0, 450.0, 520.0})
            {
                const auto x = f(&MethodResult::G)(t, 5e7);
                const auto S = f(&MethodResult::S)(t, 5e7);
                const auto V = f(&MethodResult::V)(t, 5e7);
                const auto H = f(&MethodResult::H)(t, 5e7);
                const auto Cp = f(&MethodResult::Cp)(t, 5e7);
                auto close = [](double a, double b) { return std::fabs(a - b) <= 1e-6 * (std::fabs(b) + 1.0); };
                if (!close(x.ddt, -S.val) || !close(x.ddp * 1e5, V.val) || !close(H.val, x.val + t * S.val + (-1.1e6 + 1.0e6 - 298.15 * 100.0)) ||
                    !close(H.ddt, Cp.val) || !close(t * S.ddt, Cp.val))
                {
                    std::printf("FAILED: HP98 solute thermodynamic relations at T=%g\n", t);
                    ++failures;
                }
            }
        }

        // Reaction volume as a function of T and P: derivatives, thermodynamic relations and the reference state
        {
            auto inputs = reactionInputs({1.0e-4, 2.0e-7, 1.0e-10, -4.0e-5, 1.0e-9});
            inputs.reference.reaction_volume = 3.0; // J/bar
            const auto method = makeReactionVolumePolynomial(inputs);

            ThermoPropertiesReaction in;
            in.reaction_gibbs_energy = -5.0e4; in.reaction_enthalpy = -6.0e4; in.reaction_entropy = 30.0;
            in.reaction_heat_capacity_cp = 120.0; in.reaction_heat_capacity_cv = 100.0;
            in.ln_equilibrium_constant = 20.0; in.log_equilibrium_constant = 8.7;
            in.reaction_internal_energy = -6.0e4; in.reaction_helmholtz_energy = -5.0e4;
            in.reaction_volume = 3.0;
            auto f = [&](auto member) { return [=](double t, double p) { return method->calculate(t, p, in, nullptr).*member; }; };
            const double tt = 450.0, pp = 5e7;
            for (auto m : {&ThermoPropertiesReaction::reaction_gibbs_energy, &ThermoPropertiesReaction::reaction_enthalpy,
                           &ThermoPropertiesReaction::reaction_entropy, &ThermoPropertiesReaction::reaction_volume,
                           &ThermoPropertiesReaction::reaction_heat_capacity_cp, &ThermoPropertiesReaction::ln_equilibrium_constant})
                checkDerivatives("Reaction_Vol_fT", f(m), tt, pp);

            const auto x = method->calculate(tt, pp, in, nullptr);
            auto rel = [](double a, double b) { return std::fabs(a - b) <= 1e-9 * (std::fabs(b) + 1.0); };
            // the input has no derivatives: dG/dP = V (J/bar per Pa), dG/dT + S = S(input), H = G + T S - T S(input)
            if (!rel(x.reaction_gibbs_energy.ddp * 1e5, x.reaction_volume.val) ||
                !rel(x.reaction_gibbs_energy.ddt + x.reaction_entropy.val, in.reaction_entropy.val) ||
                !rel(x.reaction_entropy.ddp, -x.reaction_volume.ddt * 1e-5))
            {
                std::printf("FAILED: Reaction_Vol_fT thermodynamic relations\n");
                ++failures;
            }
            const auto r = method->calculate(298.15, 1e5, in, nullptr); // at the reference state nothing changes
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
        const auto model = waterZhangDuan05Method();
        auto ps = [&](double t, double p) { return model->properties(t, p, 0); };
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
        const auto sverjensky = sverjenskyEtAl14Method();
        const auto fernandez = fernandezEtAl97Method();
        // the properties of the solvent at T and P, as the engine gives them to a method
        const auto electro = [&](const std::shared_ptr<const DielectricMethod>& model, double t, double p) {
            double pp = p;
            return model->properties(t, p, engine.propertiesSolvent(t, pp, "H2O@", 0), 0);
        };
        for (auto tp : {std::make_pair(450.0, 5e7), std::make_pair(650.0, 3e8), std::make_pair(300.0, 1e6)})
        {
            const double t = tp.first, p = tp.second;
            auto check = [&](const char* name, const std::shared_ptr<const DielectricMethod>& model) {
                auto e = [&](double tt, double pp) { return electro(model, tt, pp); };
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
            const auto s = electro(sverjensky, t, p);
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
