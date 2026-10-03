// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The interface of ThermoFun used by external codes is not changed by the use of autodiff:
// the header Common/ThermoScalar.hpp with the types Reaktoro_::ThermoScalar, Temperature and Pressure (with their
// arithmetic), ThermoVariables, the members val, ddt, ddp, err and sta of the properties, and the ThermoEngine calls.
#include <cmath>
#include <cstdio>

#include "Common/ThermoScalar.hpp"

// a forward declaration as in external code
namespace ThermoFun { struct ThermoPropertiesSubstance; struct ThermoPropertiesReaction; struct PropertiesSolvent; }

#include "ThermoEngine.h"
#include "ThermoProperties.h"

static int failures = 0;

static void check(bool ok, const char* what)
{
    if (!ok) { std::printf("FAILED: %s\n", what); ++failures; }
}

static bool close(double a, double b, double tol = 1e-12)
{
    return std::fabs(a - b) <= tol * (1.0 + std::fabs(b));
}

int main(int argc, char** argv)
{
    // ThermoScalar, Temperature and Pressure with their arithmetic
    Reaktoro_::Temperature T(300.0);
    Reaktoro_::Pressure P(2.0);
    Reaktoro_::ThermoScalar x = exp(T/100.0) * P + 3.0;
    check(close(x.val, std::exp(3.0)*2.0 + 3.0), "ThermoScalar value");
    check(close(x.ddt, std::exp(3.0)*2.0/100.0), "ThermoScalar ddt");
    check(close(x.ddp, std::exp(3.0)), "ThermoScalar ddp");
    check(x.sta.first == Reaktoro_::Status::calculated, "ThermoScalar status");

    Reaktoro_::ThermoScalar y(1.0, 2.0, 3.0, 0.5, {Reaktoro_::Status::assigned, ""});
    check(y.val == 1.0 && y.ddt == 2.0 && y.ddp == 3.0 && y.err == 0.5, "ThermoScalar constructor");

    ThermoFun::ThermoVariables variables;
    variables.temperature = Reaktoro_::Temperature(350.0);
    variables.pressure = Reaktoro_::Pressure(1e5);
    check(variables.temperature.val == 350.0 && variables.pressure.ddp == 1.0, "ThermoVariables");

    // Both syntaxes: x.val (ThermoFun) and x.val() (autodiff); also val(x)
    {
        Reaktoro_::ThermoScalar z(2.0, 3.0, 4.0, 0.0, {Reaktoro_::Status::assigned, ""});
        check(z.val == 2.0 && z.val() == 2.0 && z.ddt() == 3.0 && z.ddp() == 4.0 && val(z) == 2.0, "val and val()");
        z.val = 5.0; z.ddt() = 6.0; z.ddp += 1.0;
        check(z.val() == 5.0 && z.ddt == 6.0 && z.ddp == 5.0, "assignment through val and val()");
        double plain = z.val;
        double& ref = z.val;
        ref = 7.0;
        check(plain == 5.0 && z.val() == 7.0, "conversion to double and to double&");
        check(std::sqrt(z.val) == std::sqrt(7.0), "val as a double argument");
        check(sizeof(Reaktoro_::ThermoScalar) == sizeof(double) * 4 + sizeof(Reaktoro_::StatusMessage), "layout");
        const Reaktoro_::ThermoScalar c = z;
        check(c.val() == 7.0 && c.val == 7.0, "val and val() of a const instance");
    }

    // The properties calculated by the engine
    if (argc > 1)
    {
        ThermoFun::ThermoEngine engine(argv[1]);
        double P_ = 1e5;
        ThermoFun::ThermoPropertiesSubstance tps = engine.thermoPropertiesSubstance(298.15, P_, "Quartz");
        Reaktoro_::ThermoScalar G = tps.gibbs_energy;
        check(std::isfinite(G.val()) && G.ddt() != 0.0, "engine property");
        check(close(G.ddt, -tps.entropy.val, 1e-6), "dG/dT = -S");
        check(G.sta.first != Reaktoro_::Status::notdefined, "engine status");
        double inJ = (tps.gibbs_energy + tps.enthalpy * 2.0).val;
        check(std::isfinite(inJ), "arithmetic on the engine properties");

        ThermoFun::ThermoPropertiesReaction tpr = engine.thermoPropertiesReaction(298.15, P_, "Cal = Ca+2 + CO3-2");
        check(std::isfinite(tpr.log_equilibrium_constant.val), "reaction property");

        ThermoFun::PropertiesSolvent ps = engine.propertiesSolvent(298.15, P_, "H2O@");
        check(ps.density.val > 900.0 && ps.density.ddt < 0.0, "solvent property");
    }

    if (failures == 0) std::printf("All interface tests passed\n");
    return failures == 0 ? 0 : 1;
}
