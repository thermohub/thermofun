// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "CpPolynomial.h"

#include <mutex>
#include <set>

#include <ThermoFun/Methods/FunctionMethod.h>

namespace ThermoFun::v2 {

namespace {

constexpr std::size_t coefficientsPerSegment = 10;

/// What the formula needs beside the coefficients.
struct CpPolynomialData
{
    std::vector<std::pair<double, double>> intervals;
    std::vector<PhaseTransition> transitions;
    std::string name;
};

using Parameters = MethodParameters<CpPolynomialData>;

auto segmentOf(const CpPolynomialData& data, double T, bool warn, bool& outside) -> int
{
    const auto n = data.intervals.size();
    for (size_t i = 0; i < n; ++i)
        if (data.intervals[i].first <= T && data.intervals[i].second > T)
            return static_cast<int>(i);

    outside = true;
    if (warn)
        thfun_logger->warn("{}: the temperature {} is not inside the T intervals of the Cp polynomial.", data.name, T);
    if (T <= data.intervals[0].first)
        return 0;
    if (T >= data.intervals[n - 1].second)
        return static_cast<int>(n) - 1;
    // in a gap between two intervals: the nearest one
    for (size_t i = 0; i + 1 < n; ++i)
        if (T >= data.intervals[i].second && T <= data.intervals[i + 1].first)
            return (T - data.intervals[i].second <= data.intervals[i + 1].first - T) ? static_cast<int>(i) : static_cast<int>(i + 1);
    return -1;
}

auto cpPolynomialInfo(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in, const Parameters& parameters) -> void
{
    const double T = in.T, Tr = in.Tr;
    out.G.propagateFrom(ref.G, ref.S);
    out.G.setError({{1.0, ref.G.err}, {T - Tr, ref.S.err}});
    out.H.propagateFrom(ref.H);
    out.S.propagateFrom(ref.S);
    out.Cp.status = ValueInfo::Status::Calculated;
    out.Cp.err = 0.0;
    if (parameters.values.empty())  // the constant reference value: its error goes on
    {
        out.Cp = ref.Cp;
        out.G.setError({{1.0, ref.G.err}, {T - Tr, ref.S.err}, {T * std::log(T / Tr) - (T - Tr), ref.Cp.err}});
        out.H.setError({{1.0, ref.H.err}, {T - Tr, ref.Cp.err}});
        out.S.setError({{1.0, ref.S.err}, {std::log(T / Tr), ref.Cp.err}});
    }
    out.V = ValueInfo{};
    out.V.status = ValueInfo::Status::Assigned;
    if (in.outside)
    {
        const std::string message = "Empirical Cp integration: Outside temperature bounds; ";
        for (ValueInfo* v : {&out.G, &out.H, &out.S, &out.Cp})
            if (v->status == ValueInfo::Status::Calculated)
                v->message += message;
    }
}

template <typename Scalar>
auto cpPolynomial(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in, const Parameters& parameters) -> void
{
    const CpPolynomialData& data = parameters.data;
    using std::log;
    using std::pow;
    using ThermoFun::pow;

    double T0 = 0.0;
    if constexpr (std::is_same_v<Scalar, double>)
        T0 = in.T;
    else
        T0 = in.T.val();

    Scalar S = in.reference.S.value_or(Scalar(0.0));
    Scalar G = in.reference.G.value_or(Scalar(0.0));
    Scalar H = in.reference.H.value_or(Scalar(0.0));
    Scalar V = 0.0;
    Scalar TK = in.T;
    double TrK = in.reference.T;

    // without coefficients: Cp is the constant reference value; at or below the empty marker of GEMS it counts as 0
    if (parameters.values.empty())
    {
        double cp0 = 0.0;
        if (in.reference.Cp)
        {
            if constexpr (std::is_same_v<Scalar, double>)
                cp0 = *in.reference.Cp;
            else
                cp0 = in.reference.Cp->val();
        }
        if (!std::isfinite(cp0) || std::fabs(cp0) < 1e-30)
            cp0 = 0.0;
        G -= S * (TK - TrK) + cp0 * (TK * log(TK / TrK) - (TK - TrK));
        H += cp0 * (TK - TrK);
        S += cp0 * log(TK / TrK);
        state.Cp = cp0;
        state.G = G;
        state.H = H;
        state.S = S;
        state.V = V;
        state.outside_range = false;
        return;
    }

    const std::string what = data.name + " empirical Cp integration";
    if (data.intervals.empty() || !std::isfinite(T0))
    {
        errorModelParameters("Cp temperature intervals", what, __LINE__, __FILE__);
        return;
    }
    if (parameters.values.size() < data.intervals.size() * coefficientsPerSegment)
    {
        errorModelParameters("Cp empirical coefficients", what, __LINE__, __FILE__);
        return;
    }
    for (size_t i = 0; i < data.intervals.size(); ++i)
        if (!std::isfinite(data.intervals[i].first) || !std::isfinite(data.intervals[i].second) || data.intervals[i].first >= data.intervals[i].second ||
            (i > 0 && data.intervals[i].first < data.intervals[i - 1].second))
        {
            errorModelParameters("Cp temperature intervals", what, __LINE__, __FILE__);
            return;
        }

    bool outside = false;
    const int k = segmentOf(data, T0, !std::is_same_v<Scalar, double> && in.seeded_parameter < 0, outside);
    if (k < 0)
    {
        errorModelParameters("Cp temperature intervals", what, __LINE__, __FILE__);
        return;
    }

    std::vector<Scalar> ac(coefficientsPerSegment);
    auto loadCoefficients = [&](int segment) {
        for (std::size_t i = 0; i < coefficientsPerSegment; ++i)
            ac[i] = parameters.value<Scalar>(segment * coefficientsPerSegment + i, in);
    };

    loadCoefficients(k);
    Scalar Cp = (ac[0] +
                 ac[1] * TK +
                 ac[2] / (TK * TK) +
                 ac[3] / (pow(TK, 0.5)) +
                 ac[4] * (TK * TK) +
                 ac[5] * (TK * TK * TK) +
                 ac[6] * (TK * TK * TK * TK) +
                 ac[7] / (TK * TK * TK) +
                 ac[8] / TK +
                 ac[9] * (pow(TK, 0.5)));

    for (int j = 0, ft = 0; j <= k; ++j)
    {
        if (j == k)
            TK = in.T;  // the current T is the end of the integration
        else
            TK = data.intervals[j].second;

        if (!j)
            TrK = in.reference.T;  // the first interval holds the reference T
        else
            TrK = data.intervals[j].first;

        const double Tst2 = TrK * TrK;
        const double Tst3 = Tst2 * TrK;
        const double Tst4 = Tst3 * TrK;
        const double Tst05 = std::sqrt(TrK);

        if (j && data.transitions.empty())
        {
            funError("No phase transition properties present in the record.", "For substance " + data.name + ".", __LINE__, __FILE__);
            return;
        }

        if (j && data.transitions[ft].T <= TrK)
        {
            S += data.transitions[ft].dS;
            H += data.transitions[ft].dH;
            V += data.transitions[ft].dV;
            ++ft;
        }

        G -= S * (TK - TrK);

        loadCoefficients(j);

        S += (ac[0] * log((TK / TrK)) +
              ac[1] * (TK - TrK) +
              ac[2] * (1. / Tst2 - 1. / (TK * TK)) / 2. +
              ac[3] * 2. * (1. / Tst05 - 1. / (pow(TK, 0.5))) +
              ac[4] * ((TK * TK) - Tst2) / 2. +
              ac[5] * ((pow(TK, 3)) - Tst3) / 3. +
              ac[6] * ((pow(TK, 4)) - Tst4) / 4. +
              ac[7] * (1. / Tst3 - 1. / (pow(TK, 3))) / 3. +
              ac[8] * (1. / TrK - 1. / TK) +
              ac[9] * 2. * ((pow(TK, 0.5)) - Tst05));

        G -= (ac[0] * (TK * log((TK / TrK)) - (TK - TrK)) +
              ac[1] * (pow((TK - TrK), 2)) / 2. +
              ac[2] * (pow((TK - TrK), 2)) / TK / Tst2 / 2. +
              ac[3] * 2. * ((pow(TK, 0.5)) - Tst05) * ((pow(TK, 0.5)) - Tst05) / Tst05 +
              ac[4] * ((pow(TK, 3)) + 2. * Tst3 - 3. * TK * Tst2) / 6. +
              ac[5] * ((pow(TK, 4)) + 3. * Tst4 - 4. * TK * Tst3) / 12. +
              ac[6] * ((pow(TK, 4)) * TK + 4. * Tst4 * TrK - 5. * TK * Tst4) / 20. +
              ac[7] * (Tst3 - 3. * (pow(TK, 2)) * TrK + 2. * (pow(TK, 3))) / 6. / (pow(TK, 2)) / Tst3 +
              ac[8] * ((TK / TrK) - 1. - log((TK / TrK))) +
              ac[9] * 2. * (2. * TK * (pow(TK, 0.5)) - 3. * TK * Tst05 + TrK * Tst05) / 3.);

        H += (ac[0] * (TK - TrK) +
              ac[1] * ((pow(TK, 2)) - Tst2) / 2. +
              ac[2] * (1. / TrK - 1. / TK) +
              ac[3] * 2. * ((pow(TK, 0.5)) - Tst05) +
              ac[4] * ((pow(TK, 3)) - Tst3) / 3. +
              ac[5] * ((pow(TK, 4)) - Tst4) / 4. +
              ac[6] * ((pow(TK, 4)) * TK - Tst4 * TrK) / 5 +
              ac[7] * (1. / Tst2 - 1. / (pow(TK, 2))) / 2. +
              ac[8] * log((TK / TrK)) +
              ac[9] * 2. * (TK * (pow(TK, 0.5)) - TrK * Tst05) / 3.);
    }

    state.Cp = Cp;
    state.G = G;
    state.H = H;
    state.S = S;
    state.V = V;
    state.outside_range = outside;
}

} // namespace

auto cpPolynomialMethod(std::vector<std::pair<double, double>> intervals, std::vector<double> coefficients,
                        std::vector<PhaseTransition> transitions, std::string name) -> std::shared_ptr<const SubstanceMethod>
{
    return makeMethod("CpPolynomial", Parameters{std::move(coefficients), {std::move(intervals), std::move(transitions), std::move(name)}},
                      [](auto& state, const auto& in, const Parameters& p) { cpPolynomial(state, in, p); }, cpPolynomialInfo);
}

auto cpPolynomialMethod(const MethodApplication& application, const std::string& name) -> std::shared_ptr<const SubstanceMethod>
{
    static const char* names[coefficientsPerSegment] = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9"};
    std::vector<std::pair<double, double>> intervals;
    std::vector<double> coefficients;
    for (const auto& segment : application.segments)
    {
        intervals.emplace_back(segment.t_min, segment.t_max);
        if (segment.parameters.empty())
            continue;  // a segment without coefficients has no row of coefficients
        for (const char* n : names)
        {
            const auto it = segment.parameters.find(n);
            coefficients.push_back(it == segment.parameters.end() ? 0.0 : it->second.value);
        }
    }
    std::vector<PhaseTransition> transitions;
    for (const auto& row : application.transitions)
    {
        auto value = [&](const char* key) { const auto it = row.find(key); return it == row.end() ? 0.0 : it->second.value; };
        transitions.push_back({value("T"), value("dS"), value("dH"), value("dV")});
    }
    return cpPolynomialMethod(std::move(intervals), std::move(coefficients), std::move(transitions), name);
}

} // namespace ThermoFun::v2
