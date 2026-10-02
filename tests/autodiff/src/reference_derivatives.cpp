// The analytical derivatives of the Zhang-Duan water and of the Sverjensky and Fernandez dielectric constants against independent
// references: exact derivatives with autodiff higher-order dual numbers (reference/Jets.hpp), which do not share any formula with the
// analytical ones. Also against central finite differences of the values.
#include <cmath>
#include <cstdio>
#include <functional>

#include "Database.h"
#include "ElectroModelsSolvent.h"
#include "ThermoEngine.h"
#include "ThermoModelsSolvent.h"
#include "ThermoProperties.h"
#include "Substance.h"
#include "reference/Jets.hpp"

using namespace ThermoFun;

static int failures = 0;

static void expectClose(const char* what, double value, double reference, double rel)
{
    if (std::fabs(value - reference) > rel * std::fabs(reference) + 1e-300)
    {
        std::printf("FAILED: %s: %.15g, reference %.15g (relative difference %.3g)\n", what, value, reference,
                    std::fabs(value - reference) / std::fabs(reference));
        ++failures;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// Zhang and Duan (2005): density from the EOS solved with dual numbers of the third order in (T, P)
namespace zd {
const double Tc = 647.25, Vc = 55.9480373, R = 83.14467, gamma = 1.05999998e-02, MH2O = 18.015268;
const double a[] = {0.0, 3.49824207e-01, -2.91046273e+00, 2.00914688e+00, 1.12819964e-01, 7.48997714e-01, -8.73207040e-01,
                    1.70609505e-02, -1.46355822e-02, 5.79768283e-02, -8.41246372e-04, 4.95186474e-03, -9.16248538e-03,
                    -1.00358152e-01, -1.82674744e-03};

using jets::Dual;

Dual density(Dual T, Dual P)   // kg/m3 as a function of T (K) and P (Pa)
{
    const Dual Pbar = P / 1e5;
    const Dual tau = T / Tc;
    const Dual B = a[1] + a[2]/pow(tau, 2) + a[3]/pow(tau, 3);
    const Dual C = a[4] + a[5]/pow(tau, 2) + a[6]/pow(tau, 3);
    const Dual D = a[7] + a[8]/pow(tau, 2) + a[9]/pow(tau, 3);
    const Dual E = a[10] + a[11]/pow(tau, 2) + a[12]/pow(tau, 3);
    const Dual F = a[13]/tau;
    const Dual G = a[14]*tau;
    auto step = [&](const Dual& x) -> Dual {
        const Dual q = exp(-gamma/pow(x, 2));
        const Dual Z = 1 + B/x + C/pow(x, 2) + D/pow(x, 4) + E/pow(x, 5) + (F/pow(x, 2) + G/pow(x, 4))*q;
        const Dual f = Z*R*T/Pbar - x*Vc;
        const Dual df = (-B/pow(x, 2) - 2*C/pow(x, 3) - 4*D/pow(x, 5) - 5*E/pow(x, 6) -
                         (2*q*(-F*gamma*pow(x, 2) + F*pow(x, 4) - gamma*G + 2*G*pow(x, 2)))/pow(x, 7))*R*T/Pbar - Vc;
        return x - f/df;
    };
    Dual x = 0.3;
    for (int i = 0; i < 100; ++i)
    {
        Dual xn = step(x);
        const bool done = std::fabs(autodiff::val(xn) - autodiff::val(x)) < 1e-12;
        x = xn;
        if (done) break;
    }
    for (int k = 0; k < 4; ++k) x = step(x);    // the derivatives converge one order per step
    return MH2O*1000.0/Vc/x;
}
} // namespace zd

// ---------------------------------------------------------------------------------------------------------------------
// the dielectric constants as functions of T and of the density (kg/m3)
static jets::Dual epsilonSverjensky(jets::Dual T, jets::Dual rho)
{
    using jets::Dual;
    const double a1 = -1.576377e-03, a2 = 6.810288e-02, a3 = 7.548755e-01, b1 = -8.016651e-05, b2 = -6.871618e-02, b3 = 4.747973;
    const Dual tc = T - 273.15;
    const Dual r = rho / 1000.0;
    return exp(b1*tc + b2*pow(tc, 0.5) + b3)*pow(r, a1*tc + a2*pow(tc, 0.5) + a3);
}

static jets::Dual epsilonFernandez(jets::Dual T, jets::Dual rho)
{
    using jets::Dual;
    const double I[] = {1, 1, 1, 2, 3, 3, 4, 5, 6, 7, 10};
    const double J[] = {0.25, 1, 2.5, 1.5, 1.5, 2.5, 2, 2, 5, 0.5, 10};
    const double n[] = {0.978224486826, -0.957771379375, 0.237511794148, 0.714692244396, -0.298217036956, -0.108863472196,
                        .949327488264e-1, -.980469816509e-2, .165167634970e-4, .937359795772e-4, -.12317921872e-9, .196096504426e-2};
    const double Tc = 647.096, rhoc = 322.;
    const double k = 1.380658e-23, Na = 6.0221367e23, alfa = 1.636e-40, epsilon0 = 8.854187817e-12, mu = 6.138e-30, M = 0.018015268;
    Dual g = 1 + n[11]*(rho/rhoc)/pow(Tc/228/(Tc/T) - 1, 1.2);
    for (unsigned i = 0; i < 11; i++) g += n[i]*pow(rho/rhoc, I[i])*pow(Tc/T, J[i]);
    const Dual A = Na*(mu*mu)*rho*g/M/epsilon0/k/T;
    const Dual B = Na*alfa*rho/3/M/epsilon0;
    return (1 + A + 5*B + pow(9 + 2*A + 18*B + A*A + 10*A*B + 9*B*B, 0.5))/4/(1 - B);
}

int main(int argc, char** argv)
{
    // --- Zhang and Duan -------------------------------------------------------------------------------------------------
    {
        Substance water; water.setSymbol("H2O@");
        WaterZhangDuan2005 model(water);
        for (auto tp : {std::make_pair(400.0, 3e8), std::make_pair(600.0, 5e8), std::make_pair(800.0, 1e9)})
        {
            const double T = tp.first, P = tp.second;
            const auto x = model.propertiesSolvent(T, P, 0);
            const auto j = jets::jet(zd::density, T, P);
            const double tol = 1e-9;
            expectClose("ZD density", x.density.val, j.f, tol);
            expectClose("ZD densityT", x.densityT.val, j.fT, tol);
            expectClose("ZD densityP", x.densityP.val, j.fP, tol);
            expectClose("ZD densityTT", x.densityTT.val, j.fTT, tol);
            expectClose("ZD densityTP", x.densityTP.val, j.fTP, tol);
            expectClose("ZD densityPP", x.densityPP.val, j.fPP, tol);
            expectClose("ZD densityTT.ddt", x.densityTT.ddt, j.fTTT, tol);
            expectClose("ZD densityTT.ddp", x.densityTT.ddp, j.fTTP, tol);
            expectClose("ZD densityTP.ddp", x.densityTP.ddp, j.fTPP, tol);
            expectClose("ZD densityPP.ddp", x.densityPP.ddp, j.fPPP, tol);
        }
    }

    // --- GEMS HGK water: densityPP and densityTP (were not set) against central finite differences of the first derivatives ---------
    {
        Substance water; water.setSymbol("H2O@");
        WaterHGK model(water);
        auto ps = [&](double t, double p) { double pp = p; return model.propertiesSolvent(t, pp, 0, "NEA_HGK"); };
        for (auto tp : {std::make_pair(450.0, 5e7), std::make_pair(700.0, 3e8), std::make_pair(900.0, 1e9)})
        {
            const double T = tp.first, P = tp.second, hT = 1e-3 * T, hP = 1e-3 * P;
            const auto x = ps(T, P);
            const double fdPP = (ps(T, P + hP).densityP.val - ps(T, P - hP).densityP.val) / (2 * hP);
            const double fdTP = (ps(T, P + hP).densityT.val - ps(T, P - hP).densityT.val) / (2 * hP);
            const double fdTP2 = (ps(T + hT, P).densityP.val - ps(T - hT, P).densityP.val) / (2 * hT);
            expectClose("HGK gems densityPP", x.densityPP.val, fdPP, 1e-4);
            expectClose("HGK gems densityTP", x.densityTP.val, fdTP, 1e-4);
            expectClose("HGK gems densityTP (dT of densityP)", x.densityTP.val, fdTP2, 1e-4);
            expectClose("HGK gems densityTP.ddt = densityTT.ddp", x.densityTP.ddt, x.densityTT.ddp, 1e-12);
            if (x.densityPP.val == 0.0) { std::printf("FAILED: HGK gems densityPP is not set\n"); ++failures; }
        }
    }

    // --- dielectric constants of Sverjensky and Fernandez --------------------------------------------------------------------
    if (argc > 1)
    {
        ThermoEngine engine(argv[1]);
        Database database(argv[1]);
        const auto water = database.getSubstance("H2O@");
        WaterElectroSverjensky2014 sverjensky(water);
        WaterElectroFernandez1997 fernandez(water);

        auto check = [&](const char* name, auto& model, jets::Dual (*epsilon)(jets::Dual, jets::Dual), double T, double P) {
            double p = P;
            const auto psol = engine.propertiesSolvent(T, p, "H2O@", 0);
            const auto rho = jets::densityJet(psol);
            auto f = [&](jets::Dual t, jets::Dual pp) -> jets::Dual { return epsilon(t, jets::taylor(rho, t - T, pp - P)); };
            const auto e = jets::jet(f, T, P);
            const auto x = model.electroPropertiesSolvent(T, P, 0);
            const double bar = 1e5, tol = 1e-9;
            auto label = [&](const char* what) { static char buf[200]; std::snprintf(buf, sizeof buf, "%s %s (T=%g P=%g)", name, what, T, P); return buf; };
            expectClose(label("epsilon"), x.epsilon.val, e.f, tol);
            expectClose(label("epsilonT"), x.epsilonT.val, e.fT, tol);
            expectClose(label("epsilonP"), x.epsilonP.val, e.fP*bar, tol);
            expectClose(label("epsilonTT"), x.epsilonTT.val, e.fTT, tol);
            expectClose(label("epsilonTP"), x.epsilonTP.val, e.fTP*bar, tol);
            expectClose(label("epsilonPP"), x.epsilonPP.val, e.fPP*bar*bar, tol);
            expectClose(label("epsilonT.ddt"), x.epsilonT.ddt, e.fTT, tol);
            expectClose(label("epsilonT.ddp"), x.epsilonT.ddp, e.fTP, tol);
            expectClose(label("epsilonTT.ddt"), x.epsilonTT.ddt, e.fTTT, tol);
            expectClose(label("epsilonTT.ddp"), x.epsilonTT.ddp, e.fTTP, tol);
            expectClose(label("epsilonP.ddp"), x.epsilonP.ddp, e.fPP*bar, tol);
        };
        for (auto tp : {std::make_pair(450.0, 5e7), std::make_pair(650.0, 3e8), std::make_pair(300.0, 1e6)})
        {
            check("Sverjensky", sverjensky, epsilonSverjensky, tp.first, tp.second);
            check("Fernandez", fernandez, epsilonFernandez, tp.first, tp.second);
        }
    }

    if (failures == 0) std::printf("All analytical derivatives agree with the references\n");
    return failures == 0 ? 0 : 1;
}
