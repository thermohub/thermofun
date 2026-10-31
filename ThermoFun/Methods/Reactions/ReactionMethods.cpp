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

/// The inputs of a pass by one parameter: empty for none.
auto withSeed(ReactionInputs in, const std::string& parameter) -> ReactionInputs
{
    if (parameter.empty())
        return in;
    auto& s = in.seed;
    if (parameter == "logK") { s.logK = 1.0; return in; }
    if (parameter == "dG") { s.dG = 1.0; return in; }
    if (parameter == "dH") { s.dH = 1.0; return in; }
    if (parameter == "dS") { s.dS = 1.0; return in; }
    if (parameter == "dCp") { s.dCp = 1.0; return in; }
    if (parameter == "dV") { s.dV = 1e5; return in; }
    auto seedIn = [&](std::vector<real>& values, const std::vector<CoefficientUnit>& units) {
        for (std::size_t i = 0; i < units.size() && i < values.size(); ++i)
            if (units[i].name == parameter)
            {
                values[i][1] = units[i].factor;
                return true;
            }
        return false;
    };
    // a value that this method does not read has no derivative
    if (!seedIn(in.coefficients, in.coefficient_units))
        seedIn(in.cp_coefficients, in.cp_coefficient_units);
    return in;
}

/// The derivatives of a pass by a parameter, as values.
auto derivativesOf(const ThermoPropertiesReactionAD& a) -> ThermoPropertiesReaction
{
    ThermoPropertiesReaction out;
    out.log_equilibrium_constant = a.log_equilibrium_constant[1];
    out.ln_equilibrium_constant = a.ln_equilibrium_constant[1];
    out.reaction_gibbs_energy = a.reaction_gibbs_energy[1];
    out.reaction_enthalpy = a.reaction_enthalpy[1];
    out.reaction_entropy = a.reaction_entropy[1];
    out.reaction_heat_capacity_cp = a.reaction_heat_capacity_cp[1];
    out.reaction_volume = a.reaction_volume[1];
    return out;
}

/// A pass by a parameter in which lift() gives each incoming value with the derivative that its ddt holds. T does not carry it.
auto passOfIncoming(double T, double P) -> Reaktoro_::Pass
{
    Reaktoro_::Pass pass(T, P, Reaktoro_::Wrt::T);
    pass.T = real(T);
    return pass;
}

/// A reaction method: a function of T, P, the incoming properties and the solvent, and the same function in a pass by a parameter.
template <typename Calculate, typename Seeded>
class FunctionReactionMethod final : public ReactionMethod
{
public:
    FunctionReactionMethod(const char* id, ReactionInputs inputs, Calculate calculate, Seeded seeded)
    : spec_(specOf(id)), inputs_(std::move(inputs)), calculate_(std::move(calculate)), seeded_(std::move(seeded)) {}
    auto spec() const -> const MethodSpec& override { return spec_; }
    auto calculate(double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent* solvent) const -> ThermoPropertiesReaction override
    {
        return calculate_(inputs_, T, P, incoming, solvent);
    }

    auto parameterNames() const -> std::vector<std::string> override
    {
        std::vector<std::string> names;
        if (!inputs_.coefficients.empty())
            for (const auto& u : inputs_.coefficient_units)
                names.push_back(u.name);
        if (!inputs_.cp_coefficients.empty())
            for (const auto& u : inputs_.cp_coefficient_units)
                names.push_back(u.name);
        return names;
    }

    auto parameterDerivative(const std::string& parameter, double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent* solvent) const -> ThermoPropertiesReaction override
    {
        return derivativesOf(seeded_(withSeed(inputs_, parameter), T, P, incoming, solvent));
    }

private:
    const MethodSpec& spec_;
    ReactionInputs inputs_;
    Calculate calculate_;
    Seeded seeded_;
};

template <typename Calculate, typename Seeded>
auto makeReactionMethod(const char* id, ReactionInputs inputs, Calculate calculate, Seeded seeded) -> std::shared_ptr<const ReactionMethod>
{
    return std::make_shared<const FunctionReactionMethod<Calculate, Seeded>>(id, std::move(inputs), std::move(calculate), std::move(seeded));
}

/// The pressure term of a constant volume on the incoming properties. Vref: the reference volume in this pass, J/bar. Pref in bar.
auto constantVolumeTerms(const Reaktoro_::Pass& pass, ThermoPropertiesReactionAD a, const real& Vref, double Pref) -> ThermoPropertiesReactionAD
{
    const real Pbar = pass.P / 1e5; // the pressure of the pass (with its derivative)
    const real VP = a.reaction_volume * (Pbar - Pref);
    a.reaction_gibbs_energy += VP;
    a.reaction_enthalpy += VP;
    a.log_equilibrium_constant -= Vref * (Pbar - Pref) / (R_CONSTANT * pass.T) / lg_to_ln;
    a.reaction_entropy = (a.reaction_enthalpy - a.reaction_gibbs_energy) / pass.T;
    a.reaction_internal_energy = a.reaction_enthalpy - Pbar * a.reaction_volume;
    a.reaction_helmholtz_energy = a.reaction_internal_energy - pass.T * a.reaction_entropy;
    return a;
}

auto logKTable(const std::vector<double>& x, const std::vector<double>& y, int order, const ReactionInputs& inputs, double T, double P, const ThermoPropertiesReaction& incoming) -> ThermoPropertiesReaction
{
    if (x.empty() && y.empty())
        return incoming;  // a method without a table does nothing, as in the legacy code
    if (x.size() < 2 || x.size() != y.size())
        funError("The table of log K is not usable", "It needs at least two temperatures and one log K for each.", __LINE__, __FILE__);
    if (T < x.front() || T > x.back())
        funError("Temperature outside the table of log K", "T = " + std::to_string(T) + " K, the table covers " + std::to_string(x.front()) + " to " + std::to_string(x.back()) + " K.", __LINE__, __FILE__);

    const double Rln10 = R_CONSTANT * lg_to_ln;
    const real dVr = Reaktoro_::constant(inputs.reference.reaction_volume);
    auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        const real TK = pass.T;
        const auto s = order == 0 ? naturalSpline(x, y, TK) : lagrangePolynomial(x, y, TK, order);
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
    tpr.reaction_volume = inputs.reference.reaction_volume;
    return tpr;
}

// A model of the density of water: the isochoric heat capacity is not calculated.
template <typename Formula>
auto densityMethod(const char* id, Formula formula, ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    const std::string name = id;
    return makeReactionMethod(id, std::move(inputs),
        [name, formula](const ReactionInputs& in, double T, double P, const ThermoPropertiesReaction&, const PropertiesSolvent* solvent) {
            if (!solvent)
                funError("Cannot run the method " + name, "It needs the state of the solvent, and none is given.", __LINE__, __FILE__);
            auto tpr = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
                return formula(pass.T, pass.P / bar_to_Pa, in, lift(pass, *solvent));
            });
            tpr.reaction_heat_capacity_cv = Reaktoro_::ThermoProperty();
            return tpr;
        },
        [name, formula](const ReactionInputs& in, double T, double P, const ThermoPropertiesReaction&, const PropertiesSolvent* solvent) {
            if (!solvent)
                funError("Cannot run the method " + name, "It needs the state of the solvent, and none is given.", __LINE__, __FILE__);
            const Reaktoro_::Pass pass(T, P, Reaktoro_::Wrt::None);
            return formula(pass.T, pass.P / bar_to_Pa, in, lift(pass, *solvent));
        });
}

} // namespace

auto makeLogK(const char* id, LogKForm form, ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    return makeReactionMethod(id, std::move(inputs),
        [form](const ReactionInputs& in, double T, double P, const ThermoPropertiesReaction&, const PropertiesSolvent*) { return logKProperties(in, form, T, P); },
        [form](const ReactionInputs& in, double T, double P, const ThermoPropertiesReaction&, const PropertiesSolvent*) {
            const Reaktoro_::Pass pass(T, P, Reaktoro_::Wrt::None);
            return thermoPropertiesReaction_LogK_fT(pass.T, pass.P / bar_to_Pa, in, prepareLogK_fT(in, T, form).method);
        });
}

auto makeConstantReactionProperties(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    return makeReactionMethod("ConstantReactionProperties", std::move(inputs),
        [](const ReactionInputs& in, double, double, const ThermoPropertiesReaction&, const PropertiesSolvent*) { return in.reference; },
        [](const ReactionInputs& in, double, double, const ThermoPropertiesReaction&, const PropertiesSolvent*) {
            ThermoPropertiesReactionAD a;
            a.log_equilibrium_constant = seeded(in.reference.log_equilibrium_constant, in.seed.logK);
            a.reaction_gibbs_energy = seeded(in.reference.reaction_gibbs_energy, in.seed.dG);
            a.reaction_enthalpy = seeded(in.reference.reaction_enthalpy, in.seed.dH);
            a.reaction_entropy = seeded(in.reference.reaction_entropy, in.seed.dS);
            a.reaction_heat_capacity_cp = seeded(in.reference.reaction_heat_capacity_cp, in.seed.dCp);
            a.reaction_volume = seeded(in.reference.reaction_volume, in.seed.dV);
            return a;
        });
}

auto makeLogKTable(std::vector<double> temperatures, std::vector<double> logK, int order, ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    const bool empty = temperatures.empty() && logK.empty();
    return makeReactionMethod("LogKTable", std::move(inputs),
        [x = std::move(temperatures), y = std::move(logK), order](const ReactionInputs& in, double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent*) {
            return logKTable(x, y, order, in, T, P, incoming);
        },
        [empty](const ReactionInputs& in, double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent*) {
            // the values of the table are not parameters: only the reference volume has a derivative
            if (empty)
                return lift(passOfIncoming(T, P), incoming);
            ThermoPropertiesReactionAD a;
            a.reaction_volume = seeded(in.reference.reaction_volume, in.seed.dV);
            return a;
        });
}

auto makeDolejsManning10(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    auto f = [](const real& t, const real& p, const ReactionInputs& i, const PropertiesSolventAD& w) { return thermoPropertiesDolejsManning2010(t, p, i, w); };
    return densityMethod("DolejsManning10", f, std::move(inputs));
}

auto makeMarshallFranck81(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    auto f = [](const real& t, const real& p, const ReactionInputs& i, const PropertiesSolventAD& w) { return thermoPropertiesFrantzMarshall(t, p, i, w); };
    return densityMethod("MarshallFranck81", f, std::move(inputs));
}

auto makeRyzhenkoBryzgalin(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    auto f = [](const real& t, const real& p, const ReactionInputs& i, const PropertiesSolventAD& w) { return thermoPropertiesRyzhenkoBryzgalin(t, p, i, w); };
    return densityMethod("RyzhenkoBryzgalin", f, std::move(inputs));
}

auto makeReactionVolumePolynomial(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    return makeReactionMethod("ReactionVolumePolynomial", std::move(inputs),
        [](const ReactionInputs& in, double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent*) { return volumePolynomialProperties(in, T, P, incoming); },
        [](const ReactionInputs& in, double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent*) {
            const auto pass = passOfIncoming(T, P);
            return thermoPropertiesReaction_Vol_fT(pass.T, pass.P / 1e5, in, lift(pass, incoming));
        });
}

auto makeReactionConstantVolume(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>
{
    return makeReactionMethod("ReactionConstantVolume", std::move(inputs),
        [](const ReactionInputs& in, double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent*) { return constantVolumeProperties(in, T, P, incoming); },
        [](const ReactionInputs& in, double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent*) {
            const auto pass = passOfIncoming(T, P);
            return constantVolumeTerms(pass, lift(pass, incoming), seeded(in.reference.reaction_volume, in.seed.dV), in.Pr / 1e5);
        });
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
        return constantVolumeTerms(pass, lift(pass, tprIn), pass(Vref), Pref);
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
