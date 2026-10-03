// Tests that ThermoScalar (built on autodiff::real) propagates the derivatives
// with respect to temperature and pressure like a direct autodiff evaluation,
// and that the error and status propagation is retained.
#include <cmath>
#include <cstdio>

#include "Common/ThermoScalar.hpp"

using namespace Reaktoro_;

static int failures = 0;

static void check(bool ok, const char* what)
{
    if (!ok) { std::printf("FAILED: %s\n", what); ++failures; }
}

static bool close(double a, double b, double tol = 1e-12)
{
    return std::fabs(a - b) <= tol * (1.0 + std::fabs(b));
}

// f(T, P) = exp(T/300) * log(P) / sqrt(T) + pow(P, 1.5) - 2/T
template<typename S, typename Tt, typename Pt>
static S f(const Tt& T, const Pt& P)
{
    return exp(T/300.0) * log(P) / sqrt(T) + pow(P, 1.5) - 2.0/T;
}

int main()
{
    const double T0 = 350.0, P0 = 2.5;
    Temperature T(T0);
    Pressure P(P0);

    const ThermoScalar r = f<ThermoScalar>(T, P);

    // Reference: a direct autodiff::real evaluation seeding one variable at a time
    autodiff::real t = T0, p = P0;
    autodiff::seed(t);
    const autodiff::real rt = f<autodiff::real>(t, p);
    autodiff::unseed(t);
    autodiff::seed(p);
    const autodiff::real rp = f<autodiff::real>(t, p);
    autodiff::unseed(p);

    check(close(r.val(), autodiff::val(rt)), "value");
    check(close(r.ddt(), autodiff::derivative(rt)), "ddt against autodiff");
    check(close(r.ddp(), autodiff::derivative(rp)), "ddp against autodiff");

    // Analytic derivatives
    const double e = std::exp(T0/300.0);
    const double ddt = e*std::log(P0)/std::sqrt(T0)*(1.0/300.0 - 0.5/T0) + 2.0/(T0*T0);
    const double ddp = e/(P0*std::sqrt(T0)) + 1.5*std::sqrt(P0);
    check(close(r.ddt(), ddt), "ddt analytic");
    check(close(r.ddp(), ddp), "ddp analytic");

    // The passes can be used directly with autodiff
    check(close(autodiff::derivative(r.wrtT()), r.ddt()), "wrtT pass");
    check(close(autodiff::derivative(r.wrtP()), r.ddp()), "wrtP pass");

    // Division and multiplication of two ThermoScalars (quotient rule)
    const ThermoScalar q = T / P;
    check(close(q.ddt(), 1.0/P0) && close(q.ddp(), -T0/(P0*P0)), "quotient rule");
    const ThermoScalar m = T * P;
    check(close(m.ddt(), P0) && close(m.ddp(), T0), "product rule");

    // pow with ThermoScalar exponent
    const ThermoScalar pw = pow(T, P);
    check(close(pw.ddt(), P0*std::pow(T0, P0 - 1.0)), "pow ddt");
    check(close(pw.ddp(), std::pow(T0, P0)*std::log(T0)), "pow ddp");

    // Derivatives are left at zero where they are not defined (value equal to zero)
    const ThermoScalar zero(0.0, 1.0, 1.0, 0.0, {Status::assigned, ""});
    check(sqrt(zero).ddt() == 0.0 && log(zero).ddp() == 0.0 && pow(zero, 2.0).ddt() == 0.0, "zero guards");
    // zero base with a seeded exponent must not produce NaN derivatives (0*log(0))
    const ThermoScalar zpw = pow(zero, P);
    check(zpw.ddt() == 0.0 && zpw.ddp() == 0.0, "pow zero base, exponent derivatives");

    // Setters
    ThermoScalar s(1.0, 2.0, 3.0, 0.0, {Status::assigned, ""});
    s.setVal(4.0); s.setDdt(5.0); s.setDdp(6.0);
    check(s.val() == 4.0 && s.ddt() == 5.0 && s.ddp() == 6.0, "setters");

    // Scalar assignment resets derivatives and sets the status
    s = 7.0;
    check(s.val() == 7.0 && s.ddt() == 0.0 && s.ddp() == 0.0 && s.sta.first == Status::assigned, "assign scalar");

    // Status propagation: notdefined is contagious, otherwise calculated
    const ThermoScalar nd;
    check((T + nd).sta.first == Status::notdefined, "status notdefined");
    check((T + P).sta.first == Status::calculated, "status calculated");

    // Error propagation: quadrature sum
    const ThermoScalar a(1.0, 0.0, 0.0, 3.0, {Status::assigned, ""});
    const ThermoScalar b(1.0, 0.0, 0.0, 4.0, {Status::assigned, ""});
    check(close((a + b).err, 5.0), "error of sum");

    if (failures == 0) std::printf("All autodiff tests passed\n");
    return failures == 0 ? 0 : 1;
}
