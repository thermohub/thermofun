// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "ReactionMethods.h"

#include <algorithm>
#include <utility>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/Methods/Reactions/DolejsManning2010.h>
#include <ThermoFun/Methods/Reactions/FrantzMarshall.h>
#include <ThermoFun/Methods/Reactions/LogK_function_of_T.h>
#include <ThermoFun/Methods/Reactions/RyzhenkoBryzgalyn.h>
#include <ThermoFun/Methods/Reactions/Volume_function_of_T.h>

namespace ThermoFun::v2 {

namespace {

auto specOf(const char* id) -> const MethodSpec&
{
    return *MethodCatalog::instance().find(id, OperatesOn::Reaction);
}

class Base : public ReactionMethod
{
public:
    Base(const char* id, ReactionInputs inputs) : spec_(specOf(id)), inputs_(std::move(inputs)) {}
    auto spec() const -> const MethodSpec& override { return spec_; }

protected:
    const MethodSpec& spec_;
    ReactionInputs inputs_;
};

class LogK final : public Base
{
public:
    LogK(const char* id, LogKForm form, ReactionInputs inputs) : Base(id, std::move(inputs)), form_(form) {}
    auto calculate(double T, double P, const ThermoPropertiesReaction&, const PropertiesSolvent*) const -> ThermoPropertiesReaction override
    {
        return logKProperties(inputs_, form_, T, P);
    }

private:
    LogKForm form_;
};

class ConstantProperties final : public Base
{
public:
    using Base::Base;
    auto calculate(double, double, const ThermoPropertiesReaction&, const PropertiesSolvent*) const -> ThermoPropertiesReaction override
    {
        return inputs_.reference;
    }
};

class Table final : public Base
{
public:
    Table(std::vector<double> x, std::vector<double> y, int order, ReactionInputs inputs)
    : Base("LogKTable", std::move(inputs)), x_(std::move(x)), y_(std::move(y)), order_(order) {}

    auto calculate(double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent*) const -> ThermoPropertiesReaction override
    {
        if (x_.empty() && y_.empty())
            return incoming;  // a method without a table does nothing, as in the legacy code
        if (x_.size() < 2 || x_.size() != y_.size())
            funError("The table of log K is not usable", "It needs at least two temperatures and one log K for each.", __LINE__, __FILE__);
        if (T < x_.front() || T > x_.back())
            funError("Temperature outside the table of log K", "T = " + std::to_string(T) + " K, the table covers " + std::to_string(x_.front()) + " to " + std::to_string(x_.back()) + " K.", __LINE__, __FILE__);

        const double Rln10 = R_CONSTANT * lg_to_ln;
        const real dVr = Reaktoro_::constant(inputs_.reference.reaction_volume);
        auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
            const real TK = pass.T;
            const auto s = order_ == 0 ? naturalSpline(x_, y_, TK) : lagrangePolynomial(x_, y_, TK, order_);
            ThermoPropertiesReactionAD a;
            a.log_equilibrium_constant = s.value;
            a.ln_equilibrium_constant = s.value * lg_to_ln;
            a.reaction_gibbs_energy = -Rln10 * TK * s.value;
            a.reaction_enthalpy = Rln10 * TK * TK * s.first;
            a.reaction_entropy = Rln10 * (TK * s.first + s.value);
            a.reaction_heat_capacity_cp = Rln10 * (2.0 * TK * s.first + TK * TK * s.second);
            a.reaction_volume = dVr;
            return a;
        });
        tpr.log_equilibrium_constant.asCalculated();
        tpr.ln_equilibrium_constant.asCalculated();
        tpr.reaction_gibbs_energy.asCalculated();
        tpr.reaction_enthalpy.asCalculated();
        tpr.reaction_entropy.asCalculated();
        tpr.reaction_heat_capacity_cp.asCalculated();
        tpr.reaction_volume = inputs_.reference.reaction_volume;
        return tpr;
    }

private:
    std::vector<double> x_, y_;
    int order_;
};

// A model of the density of water: the isochoric heat capacity is not calculated.
template <typename Formula>
class Density final : public Base
{
public:
    Density(const char* id, Formula formula, ReactionInputs inputs) : Base(id, std::move(inputs)), formula_(formula) {}
    auto calculate(double T, double P, const ThermoPropertiesReaction&, const PropertiesSolvent* solvent) const -> ThermoPropertiesReaction override
    {
        if (!solvent)
            funError("Cannot run the method " + spec_.id, "It needs the state of the solvent, and none is given.", __LINE__, __FILE__);
        auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
            return formula_(pass.T, pass.P / bar_to_Pa, inputs_, lift(pass, *solvent));
        });
        tpr.reaction_heat_capacity_cv = Reaktoro_::ThermoProperty();
        return tpr;
    }

private:
    Formula formula_;
};

class VolumePolynomial final : public Base
{
public:
    using Base::Base;
    auto calculate(double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent*) const -> ThermoPropertiesReaction override
    {
        return volumePolynomialProperties(inputs_, T, P, incoming);
    }
};

class ConstantVolume final : public Base
{
public:
    using Base::Base;
    auto calculate(double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent*) const -> ThermoPropertiesReaction override
    {
        return constantVolumeProperties(inputs_, T, P, incoming);
    }
};

} // namespace

auto makeLogK(const char* id, LogKForm form, ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    return std::make_shared<const LogK>(id, form, std::move(inputs));
}

auto makeConstantReactionProperties(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    return std::make_shared<const ConstantProperties>("ConstantReactionProperties", std::move(inputs));
}

auto makeLogKTable(std::vector<double> temperatures, std::vector<double> logK, int order, ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    return std::make_shared<const Table>(std::move(temperatures), std::move(logK), order, std::move(inputs));
}

auto makeDolejsManning10(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    auto f = [](const real& t, const real& p, const ReactionInputs& i, const PropertiesSolventAD& w) { return thermoPropertiesDolejsManning2010(t, p, i, w); };
    return std::make_shared<const Density<decltype(f)>>("DolejsManning10", f, std::move(inputs));
}

auto makeMarshallFranck81(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    auto f = [](const real& t, const real& p, const ReactionInputs& i, const PropertiesSolventAD& w) { return thermoPropertiesFrantzMarshall(t, p, i, w); };
    return std::make_shared<const Density<decltype(f)>>("MarshallFranck81", f, std::move(inputs));
}

auto makeRyzhenkoBryzgalin(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    auto f = [](const real& t, const real& p, const ReactionInputs& i, const PropertiesSolventAD& w) { return thermoPropertiesRyzhenkoBryzgalin(t, p, i, w); };
    return std::make_shared<const Density<decltype(f)>>("RyzhenkoBryzgalin", f, std::move(inputs));
}

auto makeReactionVolumePolynomial(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    return std::make_shared<const VolumePolynomial>("ReactionVolumePolynomial", std::move(inputs));
}

auto makeReactionConstantVolume(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    return std::make_shared<const ConstantVolume>("ReactionConstantVolume", std::move(inputs));
}

auto naturalSpline(const std::vector<double>& x, const std::vector<double>& y, const real& at) -> InterpolatedValue
{
    const size_t n = x.size();
    // second derivatives M of the natural spline (M0 = Mn-1 = 0), tridiagonal system by the Thomas algorithm
    std::vector<double> M(n, 0.0), c(n, 0.0), d(n, 0.0);
    for (size_t i = 1; i + 1 < n; ++i)
    {
        const double h0 = x[i] - x[i - 1], h1 = x[i + 1] - x[i];
        const double a = h0, b = 2.0 * (h0 + h1), cc = h1;
        const double rhs = 6.0 * ((y[i + 1] - y[i]) / h1 - (y[i] - y[i - 1]) / h0);
        const double w = b - a * c[i - 1];
        c[i] = cc / w;
        d[i] = (rhs - a * d[i - 1]) / w;
    }
    for (size_t i = n - 2; i >= 1 && n > 2; --i)
    {
        M[i] = d[i] - c[i] * M[i + 1];
        if (i == 1)
            break;
    }

    // the interval of the point
    const double t = at.val();
    size_t i = std::upper_bound(x.begin(), x.end(), t) - x.begin();
    i = i == 0 ? 0 : std::min(i - 1, n - 2);
    const double h = x[i + 1] - x[i];
    const real left = x[i + 1] - at, right = at - x[i];
    InterpolatedValue s;
    s.value = M[i] * left * left * left / (6.0 * h) + M[i + 1] * right * right * right / (6.0 * h) +
              (y[i] / h - M[i] * h / 6.0) * left + (y[i + 1] / h - M[i + 1] * h / 6.0) * right;
    s.first = -M[i] * left * left / (2.0 * h) + M[i + 1] * right * right / (2.0 * h) - (y[i] / h - M[i] * h / 6.0) +
              (y[i + 1] / h - M[i + 1] * h / 6.0);
    s.second = M[i] * left / h + M[i + 1] * right / h;
    return s;
}

auto lagrangePolynomial(const std::vector<double>& x, const std::vector<double>& y, const real& at, int order) -> InterpolatedValue
{
    // the points, as in LagranInterp1D of GEMS: from the interval of the point, moved back to fit the order at the end of the table
    const int n = static_cast<int>(x.size());
    const int degree = std::min(n - 1, order);
    int start = 0;
    while (start < n - 1 && !(at.val() >= x[start] && at.val() <= x[start + 1]))
        ++start;
    if (start >= n - degree)
        start = n - degree - 1;

    // L_i = prod_{j != i} (T - x_j) / (x_i - x_j), and its first two derivatives from the products without one or two factors
    InterpolatedValue out;
    out.value = out.first = out.second = 0.0;
    for (int i = start; i <= start + degree; ++i)
    {
        double denominator = 1.0;
        real value = 1.0, first = 0.0, second = 0.0;
        for (int j = start; j <= start + degree; ++j)
        {
            if (j == i)
                continue;
            denominator *= x[i] - x[j];
            const real factor = at - x[j];
            second = second * factor + 2.0 * first;
            first = first * factor + value;
            value = value * factor;
        }
        out.value += y[i] * value / denominator;
        out.first += y[i] * first / denominator;
        out.second += y[i] * second / denominator;
    }
    return out;
}

auto logKProperties(const ReactionInputs& reaction, LogKForm form, double T, double P) -> ThermoPropertiesReaction
{
    const auto inputs = prepareLogK_fT(reaction, T, form);
    auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesReaction_LogK_fT(pass.T, pass.P / bar_to_Pa, reaction, inputs.method);
    });
    setStatusLogK_fT(tpr, inputs, T, P);
    return tpr;
}

auto volumePolynomialProperties(const ReactionInputs& reaction, double T, double P, const ThermoPropertiesReaction& tprIn) -> ThermoPropertiesReaction
{
    // P in Pa, the model works with the pressure in bar
    auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesReaction_Vol_fT(pass.T, pass.P / 1e5, reaction, lift(pass, tprIn));
    });

    auto keep = [](Reaktoro_::ThermoProperty& o, const Reaktoro_::ThermoProperty& i) { o.sta = i.sta; o.err = i.err; };
    keep(tpr.reaction_heat_capacity_cv, tprIn.reaction_heat_capacity_cv);
    tpr.reaction_volume.asCalculated();
    tpr.reaction_gibbs_energy.propagateFrom(tprIn.reaction_gibbs_energy, tpr.reaction_volume);
    tpr.reaction_enthalpy.propagateFrom(tprIn.reaction_enthalpy, tpr.reaction_volume);
    tpr.reaction_entropy.propagateFrom(tprIn.reaction_entropy, tpr.reaction_volume);
    tpr.reaction_heat_capacity_cp.propagateFrom(tprIn.reaction_heat_capacity_cp, tpr.reaction_volume);
    tpr.ln_equilibrium_constant.propagateFrom(tprIn.ln_equilibrium_constant, tpr.reaction_volume);
    tpr.log_equilibrium_constant.propagateFrom(tpr.ln_equilibrium_constant);
    tpr.reaction_internal_energy.propagateFrom(tpr.reaction_enthalpy, tpr.reaction_volume);
    tpr.reaction_helmholtz_energy.propagateFrom(tpr.reaction_internal_energy, tpr.reaction_entropy);

    // first-order propagation: the volume model has no uncertainty; the errors of the input are kept, U and A weighted
    const double Pbar = P / 1e5;
    auto keepErr = [](Reaktoro_::ThermoProperty& o, const Reaktoro_::ThermoProperty& i) { o.err = i.err; };
    keepErr(tpr.reaction_gibbs_energy, tprIn.reaction_gibbs_energy);
    keepErr(tpr.reaction_enthalpy, tprIn.reaction_enthalpy);
    keepErr(tpr.reaction_entropy, tprIn.reaction_entropy);
    keepErr(tpr.reaction_heat_capacity_cp, tprIn.reaction_heat_capacity_cp);
    keepErr(tpr.ln_equilibrium_constant, tprIn.ln_equilibrium_constant);
    keepErr(tpr.log_equilibrium_constant, tprIn.log_equilibrium_constant);
    tpr.reaction_internal_energy.setError({{1.0, tpr.reaction_enthalpy.err}, {Pbar, tpr.reaction_volume.err}});
    tpr.reaction_helmholtz_energy.setError({{1.0, tpr.reaction_internal_energy.err}, {T, tpr.reaction_entropy.err}});
    return tpr;
}

auto constantVolumeProperties(const ReactionInputs& reaction, double T, double P, const ThermoPropertiesReaction& tprIn) -> ThermoPropertiesReaction
{
    const auto Pref = reaction.Pr / 1e5;
    const auto Vref = reaction.reference.reaction_volume;

    auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        auto a = lift(pass, tprIn);
        const real Pbar = pass.P / 1e5; // the pressure of the pass (with its derivative)
        const real VP = a.reaction_volume * (Pbar - Pref);
        a.reaction_gibbs_energy += VP;
        a.reaction_enthalpy += VP;
        a.log_equilibrium_constant -= pass(Vref) * (Pbar - Pref) / (R_CONSTANT * pass.T) / lg_to_ln;
        a.reaction_entropy = (a.reaction_enthalpy - a.reaction_gibbs_energy) / pass.T;
        a.reaction_internal_energy = a.reaction_enthalpy - Pbar * a.reaction_volume;
        a.reaction_helmholtz_energy = a.reaction_internal_energy - pass.T * a.reaction_entropy;
        return a;
    });

    // the properties that were not changed keep their error and status
    auto keep = [](Reaktoro_::ThermoProperty& o, const Reaktoro_::ThermoProperty& i) { o.sta = i.sta; o.err = i.err; };
    keep(tpr.ln_equilibrium_constant, tprIn.ln_equilibrium_constant);
    keep(tpr.reaction_volume, tprIn.reaction_volume);
    keep(tpr.reaction_heat_capacity_cp, tprIn.reaction_heat_capacity_cp);
    keep(tpr.reaction_heat_capacity_cv, tprIn.reaction_heat_capacity_cv);
    tpr.reaction_gibbs_energy.propagateFrom(tprIn.reaction_gibbs_energy, tprIn.reaction_volume);
    tpr.reaction_enthalpy.propagateFrom(tprIn.reaction_enthalpy, tprIn.reaction_volume);
    tpr.log_equilibrium_constant.propagateFrom(tprIn.log_equilibrium_constant, Vref);
    tpr.reaction_entropy.propagateFrom(tpr.reaction_enthalpy, tpr.reaction_gibbs_energy);
    tpr.reaction_internal_energy.propagateFrom(tpr.reaction_enthalpy, tpr.reaction_volume);
    tpr.reaction_helmholtz_energy.propagateFrom(tpr.reaction_internal_energy, tpr.reaction_entropy);

    // first-order propagation of the errors (independent inputs)
    const double dP = P / 1e5 - Pref;
    const double RT = R_CONSTANT * T;
    tpr.reaction_gibbs_energy.setError({{1.0, tprIn.reaction_gibbs_energy.err}, {dP, tprIn.reaction_volume.err}});
    tpr.reaction_enthalpy.setError({{1.0, tprIn.reaction_enthalpy.err}, {dP, tprIn.reaction_volume.err}});
    tpr.log_equilibrium_constant.setError({{1.0, tprIn.log_equilibrium_constant.err}, {dP/(RT*lg_to_ln), Vref.err}});
    tpr.reaction_entropy.setError({{1.0/T, tpr.reaction_enthalpy.err}, {1.0/T, tpr.reaction_gibbs_energy.err}});
    tpr.reaction_internal_energy.setError({{1.0, tpr.reaction_enthalpy.err}, {P/1e5, tpr.reaction_volume.err}});
    tpr.reaction_helmholtz_energy.setError({{1.0, tpr.reaction_internal_energy.err}, {T, tpr.reaction_entropy.err}});
    return tpr;
}

} // namespace ThermoFun::v2
