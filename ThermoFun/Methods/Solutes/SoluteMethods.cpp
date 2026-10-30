// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "SoluteMethods.h"

#include <type_traits>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/Solutes/SoluteADgems.h>
#include <ThermoFun/Methods/Solutes/SoluteAnderson91.h>
#include <ThermoFun/Methods/Solutes/SoluteHKFgems.h>
#include <ThermoFun/Methods/Solutes/SoluteHKFreaktoro.h>
#include <ThermoFun/Methods/Solutes/SoluteHollandPowell98.h>

namespace ThermoFun::v2 {

namespace {

constexpr double jPerBarToM3 = 1e-5;

// From the SI values of the catalog to the units of the HKF formulas (cal, bar)
const double hkfFactor[] = {bar_to_Pa / cal_to_J, 1.0 / cal_to_J, bar_to_Pa / cal_to_J, 1.0 / cal_to_J,
                            1.0 / cal_to_J, 1.0 / cal_to_J, 1.0 / cal_to_J};

// From the SI values of the catalog to the units of the Akinfiev and Diamond formulas (cm3/g)
const double adFactor[] = {1.0, 1e3, 1e3};

// These methods work with the derivatives of the solvent of the pass, so they have no pass with plain numbers.
[[noreturn]] auto needsAPass(const std::string& id) -> void
{
    funError("Cannot run the method " + id, "It needs the state of the solvent, which exists only in a pass with derivatives.", __LINE__, __FILE__);
}

template <typename Scalar>
auto requireSolvent(const SubstanceInputs<Scalar>& in, const char* id, bool atReference = false) -> void
{
    if (!in.solvent || (atReference && !in.solvent_at_reference))
        funError("Cannot run the method " + std::string(id), "It needs the state of the solvent, and none is given.", __LINE__, __FILE__);
}

auto infoOf(const Reaktoro_::ThermoScalar& x) -> ValueInfo
{
    ValueInfo out;
    out.status = x.sta.first == Reaktoro_::Status::notdefined ? ValueInfo::Status::NotDefined : ValueInfo::Status::Calculated;
    out.err = x.err;
    return out;
}

template <typename Scalar>
auto setFromModel(SubstanceState<Scalar>& state, const ThermoPropertiesSubstanceAD& tps) -> void
{
    state.G = tps.gibbs_energy;
    state.H = tps.enthalpy;
    state.S = tps.entropy;
    state.Cp = tps.heat_capacity_cp;
    state.V = tps.volume * jPerBarToM3;
}

template <typename Method, typename Scalar>
auto hkfInputs(const std::vector<Scalar>& parameters, const HKFSpecies& species, const SubstanceInputs<Scalar>& in) -> HKFInputs
{
    HKFInputs out;
    out.symbol = species.symbol;
    out.parameters = parameters;
    out.G = in.reference.G ? (*in.reference.G)[0] : 0.0;
    out.H = in.reference.H ? (*in.reference.H)[0] : 0.0;
    out.S = in.reference.S ? (*in.reference.S)[0] : 0.0;
    out.charge = species.charge;
    out.hydrogen_ion = species.hydrogen_ion;
    return out;
}

template <typename Scalar>
auto soluteReference(const SubstanceInputs<Scalar>& in) -> SoluteReference
{
    auto value = [](const std::optional<Scalar>& x) { return x ? (*x)[0] : 0.0; };
    return {in.reference.T, value(in.reference.G), value(in.reference.H), value(in.reference.S), value(in.reference.Cp), value(in.reference.V) / jPerBarToM3};
}

// The status and the message of a point outside a range: all properties are calculated, with the message
auto setMessage(PropertiesInfo& out, const std::string& message) -> void
{
    for (auto* x : {&out.G, &out.H, &out.S, &out.Cp, &out.V})
    {
        x->status = ValueInfo::Status::Calculated;
        x->message = message;
    }
}

auto boundsMessage(const std::string& model, const Applicability& a, double T, double P) -> std::string
{
    const double tMin = a.t_min.value_or(-1e300), tMax = a.t_max.value_or(1e300), pMin = a.p_min.value_or(-1e300), pMax = a.p_max.value_or(1e300);
    if (pMax >= P && tMax >= T && pMin <= P && tMin <= T)
        return "";
    return model + ": out of T(" + std::to_string(tMin) + "-" + std::to_string(tMax) + " K) and P(" + std::to_string(pMin) + "-" + std::to_string(pMax) + " Pa) bounds";
}

// A method that gives all properties as calculated, with no error
auto allCalculated(PropertiesInfo& out) -> void
{
    for (auto* x : {&out.G, &out.H, &out.S, &out.Cp, &out.V})
    {
        x->status = ValueInfo::Status::Calculated;
        x->err = 0.0;
    }
}

} // namespace

template <typename Derived>
auto HKF88Base<Derived>::info(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in) const -> void
{
    if (!in.solvent)
        return;
    const auto& wes = in.solvent->electro;
    const ValueInfo Q = infoOf(wes.bornQ), Z = infoOf(wes.bornZ), Y = infoOf(wes.bornY), X = infoOf(wes.bornX), rho = infoOf(in.solvent->properties.density);
    // the g function, which sets the effective electrostatic radius of the charged species, is evaluated from the solvent
    // density: for those the density is one more input
    if (species_.charge != 0)
    {
        out.V.propagateFrom(Q, Z, rho);
        out.S.propagateFrom(ref.S, Y, Z, rho);
        out.G.propagateFrom(ref.G, ref.S, Z, rho);
        out.H.propagateFrom(ref.H, Z, Y, rho);
        out.Cp.propagateFrom(X, Y, Z, rho);
    }
    else
    {
        out.V.propagateFrom(Q, Z);
        out.S.propagateFrom(ref.S, Y, Z);
        out.G.propagateFrom(ref.G, ref.S, Z);
        out.H.propagateFrom(ref.H, Z, Y);
        out.Cp.propagateFrom(X, Y, Z);
    }
    out.V.err *= jPerBarToM3;
    out.G.setError({{1.0, ref.G.err}, {in.T - in.Tr, ref.S.err}}); // G = G298 - S298 (T - Tr) + ...

    const auto bounds = boundsMessage("HKF model", species_.applicability, in.T, in.P);
    if (!bounds.empty())
        setMessage(out, bounds);
    const double density = in.solvent->properties.density.val;
    if (density >= 1400 || density <= 600)
        setMessage(out, "HKF model: outside of 600-1400 kg/m3 density of pure H2O interval");
}

template class HKF88Base<HKF88Reaktoro>;
template class HKF88Base<HKF88Gems>;

template <typename Scalar>
auto HKF88Reaktoro::compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass(id);
    else
    {
        requireSolvent(in, id);
        std::vector<Scalar> parameters;
        for (std::size_t i = 0; i < values_.size(); ++i)
            parameters.push_back(this->parameter(i, in) * hkfFactor[i]);
        const auto inputs = hkfInputs<HKF88Reaktoro>(parameters, species_, in);
        const Scalar t = in.T;
        const Scalar p = in.P / bar_to_Pa;
        FunctionG g = functionG(t, p, in.solvent->properties);
        const ElectroPropertiesSubstance aes = parameters.empty() ? ElectroPropertiesSubstance{} : speciesElectroStateHKF(g, inputs);
        setFromModel(state, thermoPropertiesAqSoluteHKFreaktoro(t, p, inputs, aes, in.solvent->electro, in.solvent->properties));
    }
}

template <typename Scalar>
auto HKF88Gems::compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass(id);
    else
    {
        requireSolvent(in, id);
        std::vector<Scalar> parameters;
        for (std::size_t i = 0; i < values_.size(); ++i)
            parameters.push_back(this->parameter(i, in) * hkfFactor[i]);
        const auto inputs = hkfInputs<HKF88Gems>(parameters, species_, in);
        const Scalar t = in.T - C_to_K;
        const Scalar p = in.P / bar_to_Pa;
        FunctionG g = gShok2(t, p, in.solvent->properties);
        const ElectroPropertiesSubstance aes = parameters.empty() ? ElectroPropertiesSubstance{} : omeg92(g, inputs);
        setFromModel(state, thermoPropertiesAqSoluteHKFgems(t, p, inputs, aes, in.solvent->electro, in.solvent->properties));
    }
}

template <typename Scalar>
auto SoluteHP98::compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass(id);
    else
    {
        requireSolvent(in, id, true);
        std::vector<Scalar> parameters;
        for (std::size_t i = 0; i < values_.size(); ++i)
            parameters.push_back(this->parameter(i, in) * 1e-3);  // the formula is in kJ
        setFromModel(state, thermoPropertiesAqSoluteHP98(in.T, in.P / bar_to_Pa, soluteReference(in), parameters, symbol_, in.solvent_at_reference->properties, in.solvent->properties));
    }
}

auto SoluteHP98::info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs&) const -> void
{
    allCalculated(out);
}

template <typename Scalar>
auto AndersonEtAl91::compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass(id);
    else
    {
        requireSolvent(in, id, true);
        setFromModel(state, thermoPropertiesAqSoluteAN91(in.T, in.P / bar_to_Pa, soluteReference(in), in.solvent_at_reference->properties, in.solvent->properties));
    }
}

auto AndersonEtAl91::info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs&) const -> void
{
    allCalculated(out);
}

template <typename Scalar>
auto AkinfievDiamond03::compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass(id);
    else
    {
        requireSolvent(in, id, true);
        std::vector<Scalar> parameters;
        for (std::size_t i = 0; i < values_.size(); ++i)
            parameters.push_back(this->parameter(i, in) * adFactor[i]);
        ThermoPropertiesSubstanceAD incoming;
        incoming.gibbs_energy = state.G;
        incoming.enthalpy = state.H;
        incoming.entropy = state.S;
        incoming.heat_capacity_cp = state.Cp;
        incoming.volume = state.V / jPerBarToM3;
        const auto& s = *in.solvent;
        const auto& r = *in.solvent_at_reference;
        setFromModel(state, thermoPropertiesAqSoluteAD(in.T, in.P / bar_to_Pa, parameters, symbol_, incoming, s.water, s.ideal_gas_water, s.properties, r.water, r.ideal_gas_water, r.properties));
    }
}

auto AkinfievDiamond03::info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in) const -> void
{
    if (!in.solvent)
        return;
    const auto& w = in.solvent->water;
    const auto& r = in.solvent->water_at_reference;
    const ValueInfo g = infoOf(w.gibbs_energy), s = infoOf(w.entropy), c = infoOf(w.heat_capacity_cp), gr = infoOf(r.gibbs_energy), sr = infoOf(r.entropy), cr = infoOf(r.heat_capacity_cp);
    // the properties are not defined if the properties they are calculated from are not defined
    const auto G = out.G, S = out.S, H = out.H, Cp = out.Cp;
    out.G.propagateFrom(G, g, s, c, gr, sr, cr);
    out.S.propagateFrom(S, g, s, c, gr, sr, cr);
    out.H.propagateFrom(H, g, s, c, gr, sr, cr);
    out.Cp.propagateFrom(Cp, g, s, c, gr, sr, cr);
    out.V.status = ValueInfo::Status::Calculated;
    out.V.err = 0.0;
}

#define THERMOFUN_INSTANTIATE_SOLUTE(M)                                                                                \
    template auto M::compute(SubstanceState<double>&, const SubstanceInputs<double>&) const -> void;                   \
    template auto M::compute(SubstanceState<TPReal>&, const SubstanceInputs<TPReal>&) const -> void;

THERMOFUN_INSTANTIATE_SOLUTE(HKF88Reaktoro)
THERMOFUN_INSTANTIATE_SOLUTE(HKF88Gems)
THERMOFUN_INSTANTIATE_SOLUTE(SoluteHP98)
THERMOFUN_INSTANTIATE_SOLUTE(AndersonEtAl91)
THERMOFUN_INSTANTIATE_SOLUTE(AkinfievDiamond03)

#undef THERMOFUN_INSTANTIATE_SOLUTE

} // namespace ThermoFun::v2
