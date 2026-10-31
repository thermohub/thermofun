// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The template of ThermoEngine::Impl::addParameterErrors. Only for ThermoEngine.cpp, which calls it.

#ifndef THERMOFUN_THERMOENGINEPARAMETERERRORS_HPP
#define THERMOFUN_THERMOENGINEPARAMETERERRORS_HPP

#include "ThermoEngineImpl.h"

namespace ThermoFun
{

    /// Add the effect of the uncertain parameters to the errors of the properties calculated by eval: the standard error
    /// of a property is the quadrature sum of (y(p+s) - y(p-s))/2 over the parameters p with an error s (first-order
    /// propagation, independent parameters; parameters shared by several records are perturbed together)
template<class Props, class Eval>
auto ThermoEngine::Impl::addParameterErrors(Props& result, const Records& records, const Substance* givenS, const Reaction* givenR, Eval eval) const -> void
{
    using Sub = Substance;
    using Rea = Reaction;
    std::vector<Parameter> parameters;

    auto listSubstance = [&](Sub s, int kind, const std::string& symbol) {
        auto ref = s.thermoReferenceProperties();
        auto refs = referenceFields(ref);
        for (size_t i = 0; i < refs.size(); ++i)
            if (refs[i]->err > 0.0) parameters.push_back({kind, symbol, int(i), "", refs[i]->err});
        auto p = s.thermoParameters();
        for (const auto& c : uncertainCoefficients(p)) parameters.push_back({kind, symbol, -1, c.key, c.error});
    };
    auto listReaction = [&](Rea r, int kind, const std::string& symbol) {
        auto ref = r.thermoReferenceProperties();
        auto refs = referenceFields(ref);
        for (size_t i = 0; i < refs.size(); ++i)
            if (refs[i]->err > 0.0) parameters.push_back({kind, symbol, int(i), "", refs[i]->err});
        auto p = r.thermoParameters();
        for (const auto& c : uncertainCoefficients(p)) parameters.push_back({kind, symbol, -1, c.key, c.error});
    };
    for (const auto& symbol : records.substances) listSubstance(database.getSubstance(symbol), 0, symbol);
    for (const auto& symbol : records.reactions) listReaction(database.getReaction(symbol), 1, symbol);
    if (givenS) listSubstance(*givenS, 2, givenS->symbol());
    if (givenR) listReaction(*givenR, 3, givenR->symbol());
    if (parameters.empty()) return;

    auto& engine = perturbedEngine();
    auto& db = engine.pimpl->database;

    auto perturb = [&](const Parameter& q, double delta) -> Props {
        // the value of the parameter is changed in a copy of its record
        Substance tmpS; Reaction tmpR;
        const bool isSub = (q.record == 0 || q.record == 2);
        if (q.record == 0) tmpS = db.getSubstance(q.symbol);
        if (q.record == 1) tmpR = db.getReaction(q.symbol);
        if (q.record == 2) tmpS = *givenS;
        if (q.record == 3) tmpR = *givenR;
        const Substance origS = tmpS; const Reaction origR = tmpR;

        auto apply = [&](auto& record) {
            if (q.reference >= 0)
            {
                auto ref = record.thermoReferenceProperties();
                referenceFields(ref)[q.reference]->val += delta;
                record.setThermoReferenceProperties(ref);
            }
            else
            {
                auto par = record.thermoParameters();
                for (const auto& c : uncertainCoefficients(par))
                    if (c.key == q.key) { *c.value += delta; break; }
                record.setThermoParameters(par);
            }
        };
        if (isSub) apply(tmpS); else apply(tmpR);

        Props out;
        try
        {
            if (q.record == 0) db.setMapSubstances({{tmpS.symbol(), tmpS}});
            if (q.record == 1) db.setMapReactions({{tmpR.symbol(), tmpR}});
            out = eval(engine, q.record == 2 ? &tmpS : givenS, q.record == 3 ? &tmpR : givenR);
        }
        catch (...)
        {
            if (q.record == 0) db.setMapSubstances({{origS.symbol(), origS}});
            if (q.record == 1) db.setMapReactions({{origR.symbol(), origR}});
            throw;
        }
        if (q.record == 0) db.setMapSubstances({{origS.symbol(), origS}});
        if (q.record == 1) db.setMapReactions({{origR.symbol(), origR}});
        return out;
    };

    auto fr = fields(result);
    std::vector<double> sum2(fr.size(), 0.0);
    std::vector<char> incomplete(fr.size(), 0); // a contribution to the error is missing
    auto defined = [&](size_t k) { return fr[k]->sta.first != Reaktoro_::Status::notdefined; };
    for (const auto& q : parameters)
    {
        try
        {
            auto plus  = perturb(q, +q.sigma);
            auto minus = perturb(q, -q.sigma);
            auto fp = fields(plus), fm = fields(minus);
            for (size_t k = 0; k < fp.size(); ++k)
            {
                if (!defined(k)) continue;
                if (fp[k]->sta.first == Reaktoro_::Status::notdefined || fm[k]->sta.first == Reaktoro_::Status::notdefined)
                {
                    incomplete[k] = 1;
                    continue;
                }
                const double d = 0.5 * (fp[k]->val - fm[k]->val);
                if (std::isfinite(d)) sum2[k] += d * d; else incomplete[k] = 1;
            }
        }
        catch (...) // the calculation fails for this parameter: the error of every property is incomplete
        {
            std::fill(incomplete.begin(), incomplete.end(), 1);
        }
    }
    for (size_t k = 0; k < fr.size(); ++k)
    {
        if (!defined(k)) { fr[k]->err = std::sqrt(sum2[k]); continue; }
        if (!incomplete[k]) { fr[k]->err = std::sqrt(sum2[k]); continue; }
        // not a number instead of an error that is too small (or 0) and looks complete
        fr[k]->err = std::numeric_limits<double>::quiet_NaN();
        auto& message = fr[k]->sta.second;
        message += std::string(message.empty() ? "" : " ") +
            "The uncertainty from the parameter errors is incomplete (the calculation failed or was not defined for a perturbed parameter): err is not a number.";
    }
}

} // namespace ThermoFun

#endif
