// Tests of the autodiff infrastructure: the derivatives with respect to temperature and pressure calculated
// with autodiff::real in two passes (see Pass), and the explicit propagation of errors and statuses.
#include <cmath>
#include <cstdio>

#include "ThermoProperties.h"
#include "Common/Rounding.hpp"

using namespace ThermoFun;
using Reaktoro_::Pass;
using Reaktoro_::Wrt;
using Reaktoro_::ThermoProperty;
using Reaktoro_::Status;

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
static real f(const real& T, const real& P)
{
    return exp(T/300.0) * log(P) / sqrt(T) + pow(P, 1.5) - 2.0/T;
}

int main()
{
    const double T0 = 350.0, P0 = 2.5;

    // Value and derivatives of a function of T and P calculated in the two passes
    const auto r = Reaktoro_::toProperty(f(Pass(T0, P0, Wrt::T).T, Pass(T0, P0, Wrt::T).P),
                                         f(Pass(T0, P0, Wrt::P).T, Pass(T0, P0, Wrt::P).P));

    const double e = std::exp(T0/300.0);
    check(close(r.val, e*std::log(P0)/std::sqrt(T0) + std::pow(P0, 1.5) - 2.0/T0), "value");
    check(close(r.ddt, e*std::log(P0)/std::sqrt(T0)*(1.0/300.0 - 0.5/T0) + 2.0/(T0*T0)), "ddt analytic");
    check(close(r.ddp, e/(P0*std::sqrt(T0)) + 1.5*std::sqrt(P0)), "ddp analytic");

    // twoPass with a property struct
    const auto tps = twoPass(T0, P0, [](const Pass& pass) {
        ThermoPropertiesSubstanceAD a;
        a.gibbs_energy = pass.T * pass.P;
        a.volume = pass.T / pass.P;
        return a;
    });
    check(close(tps.gibbs_energy.ddt, P0) && close(tps.gibbs_energy.ddp, T0), "product rule");
    check(close(tps.volume.ddt, 1.0/P0) && close(tps.volume.ddp, -T0/(P0*P0)), "quotient rule");
    check(tps.gibbs_energy.sta.first == Status::calculated, "calculated status");

    // Lifting a property: its derivatives are those of the pass, constants have no derivative
    ThermoProperty x(2.0, 3.0, 4.0, 0.1, {Status::assigned, ""});
    check(Pass(T0, P0, Wrt::T)(x).val() == 2.0 && Pass(T0, P0, Wrt::T)(x)[1] == 3.0, "lift wrt T");
    check(Pass(T0, P0, Wrt::P)(x)[1] == 4.0, "lift wrt P");
    check(Pass(T0, P0, Wrt::None)(x)[1] == 0.0, "lift without derivatives");
    check(Reaktoro_::constant(x)[1] == 0.0, "constant");

    // Assigning a value gives a constant that is assigned
    ThermoProperty y;
    check(y.sta.first == Status::notdefined, "default property not defined");
    y = 7.0;
    check(y.val == 7.0 && y.ddt == 0.0 && y.ddp == 0.0 && y.sta.first == Status::assigned, "assign value");

    // Status propagation: not defined is contagious, otherwise calculated
    ThermoProperty a(1.0, 0.0, 0.0, 3.0, {Status::assigned, ""});
    ThermoProperty b(1.0, 0.0, 0.0, 4.0, {Status::read, ""});
    ThermoProperty nd;
    ThermoProperty c;
    c.propagateFrom(a, b);
    check(c.sta.first == Status::calculated, "status calculated");
    c.propagateFrom(a, nd);
    check(c.sta.first == Status::notdefined, "status not defined");

    // Error propagation: quadrature sum
    c.propagateFrom(a, b);
    check(close(c.err, 5.0), "error of sum");

    // The result of a calculation can be propagated from itself
    c.propagateFrom(c, nd);
    check(c.sta.first == Status::notdefined, "propagate from itself");

    // Absolute errors of the ThermoScalar functions: |df/dx| err (x = 4 +- 0.2, exponent p = 3 +- 0.1)
    {
        const double v = 4.0, ev = 0.2, p = 3.0, ep = 0.1;
        ThermoProperty xs(v, 1.0, 0.0, ev, {Status::assigned, ""});
        ThermoProperty ps(p, 0.0, 0.0, ep, {Status::assigned, ""});
        check(close(sqrt(xs).err, ev/(2.0*std::sqrt(v))), "error of sqrt");
        check(close(pow(xs, 3.0).err, 3.0*v*v*ev), "error of pow(x, p)");
        check(close(pow(xs, ps).err, std::hypot(p*std::pow(v, p - 1.0)*ev, std::pow(v, p)*std::log(v)*ep)), "error of pow(x, y)");
        check(close(log(xs).err, ev/v), "error of log");
        check(close(log10(xs).err, ev/v/std::log(10.0)), "error of log10");
        check(close((5.0/xs).err, 5.0*ev/(v*v)), "error of scalar / x");
    }

    // Rounding: a large scaled number with a fraction of .25 is not a tie
    check(ThermoFun::rounding::roundHalfEven(281474976710657.25, 0) == 281474976710657.0 &&
          ThermoFun::rounding::roundHalfEven(25.45, 1) == 25.4, "roundHalfEven of a non-tie at 2.8e14");

    if (failures == 0) std::printf("All autodiff tests passed\n");
    return failures == 0 ? 0 : 1;
}
