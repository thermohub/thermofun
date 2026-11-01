// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "SoluteMethods.h"

#include <type_traits>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/FunctionMethod.h>
#include <ThermoFun/Methods/MethodHelpers.h>
#include <ThermoFun/Methods/Solutes/SoluteADgems.h>
#include <ThermoFun/Methods/Solutes/SoluteAnderson91.h>
#include <ThermoFun/Methods/Solutes/SoluteHKFgems.h>
#include <ThermoFun/Methods/Solutes/SoluteHKFreaktoro.h>
#include <ThermoFun/Methods/Solutes/SoluteHollandPowell98.h>

namespace ThermoFun::v2 {

namespace {

// From the SI values of the catalog to the units of the HKF formulas (cal, bar)
const double hkfFactor[] = {bar_to_Pa / cal_to_J, 1.0 / cal_to_J, bar_to_Pa / cal_to_J, 1.0 / cal_to_J,
                            1.0 / cal_to_J, 1.0 / cal_to_J, 1.0 / cal_to_J};

// From the SI values of the catalog to the units of the Akinfiev and Diamond formulas (cm3/g)
const double adFactor[] = {1.0, 1e3, 1e3};

template <typename Scalar>
auto requireSolvent(const SubstanceInputs<Scalar>& in, const char* id, bool atReference = false) -> void
{
    if (!in.solvent || (atReference && !in.solvent_at_reference))
        funError("Cannot run the method " + std::string(id), "It needs the state of the solvent, and none is given.", __LINE__, __FILE__);
}

template <typename Scalar>
auto hkfInputs(const std::vector<Scalar>& parameters, const HKFSpecies& species, const SubstanceInputs<Scalar>& in) -> HKFInputs
{
    HKFInputs out;
    out.symbol = species.symbol;
    out.parameters = parameters;
    out.G = in.reference.G ? *in.reference.G : Scalar(0.0);
    out.H = in.reference.H ? *in.reference.H : Scalar(0.0);
    out.S = in.reference.S ? *in.reference.S : Scalar(0.0);
    out.charge = species.charge;
    out.hydrogen_ion = species.hydrogen_ion;
    out.constants = species.constants;
    return out;
}

template <typename Scalar>
auto soluteReference(const SubstanceInputs<Scalar>& in) -> SoluteReference
{
    auto value = [](const std::optional<Scalar>& x) { return x ? *x : Scalar(0.0); };
    return {in.reference.T, value(in.reference.G), value(in.reference.H), value(in.reference.S), value(in.reference.Cp), value(in.reference.V) / jPerBarToM3};
}

using HKFParameters = MethodParameters<HKFSpecies>;
using SymbolParameters = MethodParameters<std::string>;

auto hkf88Info(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in, const HKFSpecies& species) -> void
{
    if (!in.solvent)
        return;
    const auto& wes = in.solvent->electro;
    const ValueInfo Q = infoOf(wes.bornQ), Z = infoOf(wes.bornZ), Y = infoOf(wes.bornY), X = infoOf(wes.bornX), rho = infoOf(in.solvent->properties.density);
    // the g function, which sets the effective electrostatic radius of the charged species, is evaluated from the solvent
    // density: for those the density is one more input
    if (species.charge != 0)
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

    const auto bounds = boundsMessage("HKF model", species.applicability, in.T, in.P);
    if (!bounds.empty())
        setMessage(out, bounds);
    const double density = in.solvent->properties.density.val;
    if (density >= 1400 || density <= 600)
        setMessage(out, "HKF model: outside of 600-1400 kg/m3 density of pure H2O interval");
}

template <typename Scalar>
auto hkf88Reaktoro(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in, const HKFParameters& p) -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass("HKF88");
    else
    {
        requireSolvent(in, "HKF88");
        std::vector<Scalar> parameters;
        for (std::size_t i = 0; i < p.values.size(); ++i)
            parameters.push_back(p.value(i, in) * hkfFactor[i]);
        const auto inputs = hkfInputs(parameters, p.data, in);
        const Scalar t = in.T;
        const Scalar p = in.P / bar_to_Pa;
        FunctionG g = functionG(t, p, in.solvent->properties, inputs.constants);
        const ElectroPropertiesSubstance aes = parameters.empty() ? ElectroPropertiesSubstance{} : speciesElectroStateHKF(g, inputs);
        setFromModel(state, thermoPropertiesAqSoluteHKFreaktoro(t, p, inputs, aes, in.solvent->electro, in.solvent->properties));
    }
}

template <typename Scalar>
auto hkf88Gems(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in, const HKFParameters& p) -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass("HKF88");
    else
    {
        requireSolvent(in, "HKF88");
        std::vector<Scalar> parameters;
        for (std::size_t i = 0; i < p.values.size(); ++i)
            parameters.push_back(p.value(i, in) * hkfFactor[i]);
        const auto inputs = hkfInputs(parameters, p.data, in);
        const Scalar t = in.T - C_to_K;
        const Scalar p = in.P / bar_to_Pa;
        FunctionG g = gShok2(t, p, in.solvent->properties, inputs.constants);
        const ElectroPropertiesSubstance aes = parameters.empty() ? ElectroPropertiesSubstance{} : omeg92(g, inputs);
        setFromModel(state, thermoPropertiesAqSoluteHKFgems(t, p, inputs, aes, in.solvent->electro, in.solvent->properties));
    }
}

template <typename Scalar>
auto soluteHP98(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in, const SymbolParameters& p) -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass("SoluteHP98");
    else
    {
        requireSolvent(in, "SoluteHP98", true);
        std::vector<Scalar> parameters;
        for (std::size_t i = 0; i < p.values.size(); ++i)
            parameters.push_back(p.value(i, in) * 1e-3);  // the formula is in kJ
        setFromModel(state, thermoPropertiesAqSoluteHP98(in.T, in.P / bar_to_Pa, soluteReference(in), parameters, p.data, in.solvent_at_reference->properties, in.solvent->properties));
    }
}

template <typename Scalar>
auto andersonEtAl91(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass("AndersonEtAl91");
    else
    {
        requireSolvent(in, "AndersonEtAl91", true);
        setFromModel(state, thermoPropertiesAqSoluteAN91(in.T, in.P / bar_to_Pa, soluteReference(in), in.solvent_at_reference->properties, in.solvent->properties));
    }
}

template <typename Scalar>
auto akinfievDiamond03(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in, const SymbolParameters& p) -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass("AkinfievDiamond03");
    else
    {
        requireSolvent(in, "AkinfievDiamond03", true);
        std::vector<Scalar> parameters;
        for (std::size_t i = 0; i < p.values.size(); ++i)
            parameters.push_back(p.value(i, in) * adFactor[i]);
        const auto incoming = toModel(state);
        const auto& s = *in.solvent;
        const auto& r = *in.solvent_at_reference;
        setFromModel(state, thermoPropertiesAqSoluteAD(in.T, in.P / bar_to_Pa, parameters, p.data, incoming, s.water, s.ideal_gas_water, s.properties, r.water, r.ideal_gas_water, r.properties));
    }
}

auto akinfievDiamond03Info(PropertiesInfo& out, const InfoInputs& in) -> void
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

} // namespace

auto hkf88ReaktoroMethod(std::vector<double> parameters, HKFSpecies species, HKFConstants constants) -> std::shared_ptr<const SubstanceMethod>
{
    species.constants = constants;
    return makeMethod("HKF88", HKFParameters{std::move(parameters), std::move(species)},
                      [](auto& state, const auto& in, const HKFParameters& p) { hkf88Reaktoro(state, in, p); },
                      [](PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in, const HKFParameters& p) { hkf88Info(out, ref, in, p.data); });
}

auto hkf88GemsMethod(std::vector<double> parameters, HKFSpecies species, HKFConstants constants) -> std::shared_ptr<const SubstanceMethod>
{
    species.constants = constants;
    return makeMethod("HKF88", HKFParameters{std::move(parameters), std::move(species)},
                      [](auto& state, const auto& in, const HKFParameters& p) { hkf88Gems(state, in, p); },
                      [](PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in, const HKFParameters& p) { hkf88Info(out, ref, in, p.data); });
}

auto soluteHP98Method(std::vector<double> parameters, std::string symbol) -> std::shared_ptr<const SubstanceMethod>
{
    return makeMethod("SoluteHP98", SymbolParameters{std::move(parameters), std::move(symbol)},
                      [](auto& state, const auto& in, const SymbolParameters& p) { soluteHP98(state, in, p); },
                      [](PropertiesInfo& out, const PropertiesInfo&, const InfoInputs&, const auto&) { allCalculated(out); });
}

auto andersonEtAl91Method() -> std::shared_ptr<const SubstanceMethod>
{
    return makeMethod("AndersonEtAl91", MethodParameters<>{},
                      [](auto& state, const auto& in, const auto&) { andersonEtAl91(state, in); },
                      [](PropertiesInfo& out, const PropertiesInfo&, const InfoInputs&, const auto&) { allCalculated(out); });
}

auto akinfievDiamond03Method(std::vector<double> parameters, std::string symbol) -> std::shared_ptr<const SubstanceMethod>
{
    return makeMethod("AkinfievDiamond03", SymbolParameters{std::move(parameters), std::move(symbol)},
                      [](auto& state, const auto& in, const SymbolParameters& p) { akinfievDiamond03(state, in, p); },
                      [](PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in, const auto&) { akinfievDiamond03Info(out, in); });
}

} // namespace ThermoFun::v2
