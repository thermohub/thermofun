// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "CpPolynomial.h"

#include <mutex>
#include <set>

namespace ThermoFun::v2 {

CpPolynomial::CpPolynomial(std::vector<std::pair<double, double>> intervals, std::vector<double> coefficients,
                           std::vector<PhaseTransition> transitions, std::string name)
    : SubstanceMethodBase(std::move(coefficients)), intervals_(std::move(intervals)), transitions_(std::move(transitions)), name_(std::move(name))
{
}

auto CpPolynomial::fromApplication(const MethodApplication& application, const std::string& name) -> CpPolynomial
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
    return CpPolynomial(std::move(intervals), std::move(coefficients), std::move(transitions), name);
}

auto CpPolynomial::segmentOf(double T, bool warn, bool& outside) const -> int
{
    const auto n = intervals_.size();
    for (size_t i = 0; i < n; ++i)
        if (intervals_[i].first <= T && intervals_[i].second > T)
            return static_cast<int>(i);

    outside = true;
    if (warn)
        thfun_logger->warn("{}: the temperature {} is not inside the T intervals of the Cp polynomial.", name_, T);
    if (T <= intervals_[0].first)
        return 0;
    if (T >= intervals_[n - 1].second)
        return static_cast<int>(n) - 1;
    // in a gap between two intervals: the nearest one
    for (size_t i = 0; i + 1 < n; ++i)
        if (T >= intervals_[i].second && T <= intervals_[i + 1].first)
            return (T - intervals_[i].second <= intervals_[i + 1].first - T) ? static_cast<int>(i) : static_cast<int>(i + 1);
    return -1;
}

template <typename Scalar>
auto CpPolynomial::compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
{
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
    if (values_.empty())
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

    const std::string what = name_ + " Cp polynomial";
    if (intervals_.empty() || !std::isfinite(T0))
    {
        errorModelParameters("Cp temperature intervals", what, __LINE__, __FILE__);
        return;
    }
    if (values_.size() < intervals_.size() * coefficientsPerSegment)
    {
        errorModelParameters("Cp empirical coefficients", what, __LINE__, __FILE__);
        return;
    }
    for (size_t i = 0; i < intervals_.size(); ++i)
        if (!std::isfinite(intervals_[i].first) || !std::isfinite(intervals_[i].second) || intervals_[i].first >= intervals_[i].second ||
            (i > 0 && intervals_[i].first < intervals_[i - 1].second))
        {
            errorModelParameters("Cp temperature intervals", what, __LINE__, __FILE__);
            return;
        }

    bool outside = false;
    const int k = segmentOf(T0, !std::is_same_v<Scalar, double> && in.seeded_parameter < 0, outside);
    if (k < 0)
    {
        errorModelParameters("Cp temperature intervals", what, __LINE__, __FILE__);
        return;
    }

    std::vector<Scalar> ac(coefficientsPerSegment);
    auto loadCoefficients = [&](int segment) {
        for (std::size_t i = 0; i < coefficientsPerSegment; ++i)
            ac[i] = parameter<Scalar>(segment * coefficientsPerSegment + i, in);
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
            TK = intervals_[j].second;

        if (!j)
            TrK = in.reference.T;  // the first interval holds the reference T
        else
            TrK = intervals_[j].first;

        const double Tst2 = TrK * TrK;
        const double Tst3 = Tst2 * TrK;
        const double Tst4 = Tst3 * TrK;
        const double Tst05 = std::sqrt(TrK);

        if (j && transitions_.empty())
        {
            funError("No phase transition properties present in the record.", "For substance " + name_ + ".", __LINE__, __FILE__);
            return;
        }

        if (j && transitions_[ft].T <= TrK)
        {
            S += transitions_[ft].dS;
            H += transitions_[ft].dH;
            V += transitions_[ft].dV;
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

template auto CpPolynomial::compute<double>(SubstanceState<double>&, const SubstanceInputs<double>&) const -> void;
template auto CpPolynomial::compute<TPReal>(SubstanceState<TPReal>&, const SubstanceInputs<TPReal>&) const -> void;

} // namespace ThermoFun::v2
