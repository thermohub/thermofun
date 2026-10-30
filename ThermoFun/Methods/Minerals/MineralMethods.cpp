// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "MineralMethods.h"

#include <cmath>
#include <type_traits>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Methods/MethodHelpers.h>
#include <ThermoFun/Methods/Minerals/SolidBMGottschalk.h>
#include <ThermoFun/Methods/Minerals/SolidBerman88.h>
#include <ThermoFun/Methods/Minerals/SolidHPLandau.h>
#include <ThermoFun/Methods/Minerals/SolidLandauHP11.h>
#include <ThermoFun/Methods/Minerals/SolidMurnaghanHP98.h>
#include <ThermoFun/Methods/Minerals/SolidTaitHP11.h>
#include <ThermoFun/Methods/Minerals/StandardThermoModelsExtra.h>

namespace ThermoFun::v2 {

namespace {

template <typename Scalar>
auto valueOf(const std::optional<Scalar>& x) -> double
{
    return x ? (*x)[0] : 0.0;
}

// What the formulas read of the record; the coefficients are added by the method
template <typename Scalar>
auto inputsOf(const SubstanceInputs<Scalar>& in) -> MineralInputs
{
    MineralInputs out;
    out.Tr = in.reference.T;
    out.Pr = in.reference.P;
    out.G = valueOf(in.reference.G);
    out.H = valueOf(in.reference.H);
    out.S = valueOf(in.reference.S);
    out.Cp = valueOf(in.reference.Cp);
    out.V = valueOf(in.reference.V) / jPerBarToM3;
    return out;
}

// From SI to the units of the formulas: value * factor + offset. Each row is one coefficient of the method.
struct Unit
{
    double factor = 1.0, offset = 0.0;
};

template <typename Method>
auto unitsOf() -> std::vector<Unit>
{
    if constexpr (std::is_same_v<Method, LandauHP98> || std::is_same_v<Method, LandauHP11>)
        return {{1.0, -C_to_K}, {}, {1e5, 0.0}};          // Tc0 degC, Smax, Vmax J/bar
    else if constexpr (std::is_same_v<Method, TaitHP11>)
        return {{}, {1e8, 0.0}, {}};                      // kappa0p, kappa0pp 1/kbar, n_atoms
    else if constexpr (std::is_same_v<Method, VolumeBerman88> || std::is_same_v<Method, VolumeDorogokupets88>)
        return {{}, {}, {}, {1e5, 0.0}, {1e10, 0.0}};     // a1, a2, a3, b1 1/bar, b2 1/bar2
    else if constexpr (std::is_same_v<Method, BirchMurnaghan47>)
        return {{}, {}, {}, {0.0, 0.0}, {0.0, 0.0}, {1e-6, 0.0}, {1e-6, 0.0}, {}, {1e6, 0.0}};  // two unused places; MPa
    else if constexpr (std::is_same_v<Method, CpExtendedUNIQUAC>)
        return {{}, {}, {}, {1e5, 0.0}, {1e10, 0.0}, {}}; // a, b, c, alpha 1/bar, beta 1/bar2, Theta
    else
        return {};
}

template <typename Method>
auto formula(const real& T, const real& Pbar, const MineralInputs& inputs, const ThermoPropertiesSubstanceAD& state) -> ThermoPropertiesSubstanceAD
{
    if constexpr (std::is_same_v<Method, LandauHP98>)
        return thermoPropertiesHPLandau(T, Pbar, inputs, state);
    else if constexpr (std::is_same_v<Method, LandauHP11>)
        return thermoPropertiesHP11Landau(T, Pbar, inputs, state);
    else if constexpr (std::is_same_v<Method, MurnaghanHP98>)
        return thermoPropertiesMinMurnaghanEOSHP98(T, Pbar, inputs, state);
    else if constexpr (std::is_same_v<Method, TaitHP11>)
        return thermoPropertiesMinTaitHP11(T, Pbar, inputs, state);
    else if constexpr (std::is_same_v<Method, VolumeBerman88> || std::is_same_v<Method, VolumeDorogokupets88>)
        return thermoPropertiesMinBerman88(T, Pbar, inputs, state);
    else if constexpr (std::is_same_v<Method, BirchMurnaghan47>)
        return thermoPropertiesMinBMGottschalk(T, Pbar, inputs, state);
    else
        return thermoPropertiesExtendedUNIQUAC(T, Pbar, inputs);
}

template <typename Method>
auto boundsOf(PropertiesInfo& out, const char* model, const Applicability& a, const InfoInputs& in) -> void
{
    const auto message = boundsMessage(model, a, in.T, in.P);
    if (!message.empty())
        setMessage(out, message);
}

} // namespace

template <typename Derived>
template <typename Scalar>
auto MineralMethod<Derived>::compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass(Derived::id);
    else
    {
        auto inputs = inputsOf(in);
        const auto units = unitsOf<Derived>();
        for (std::size_t i = 0; i < coefficients_; ++i)
        {
            const auto unit = i < units.size() ? units[i] : Unit{};
            inputs.coefficients.push_back(this->parameter(i, in) * unit.factor + unit.offset);
        }
        inputs.alpha0 = this->parameter(coefficients_, in);
        inputs.kappa0 = this->parameter(coefficients_ + 1, in) * 1e-8;  // kbar
        setFromModel(state, formula<Derived>(in.T, in.P / bar_to_Pa, inputs, toModel(state)));
    }
}

auto LandauHP98::info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in) const -> void
{
    const ValueInfo G = out.G, S = out.S, H = out.H, Cp = out.Cp;
    out.G.propagateFrom(G);
    out.S.propagateFrom(S);
    out.H.propagateFrom(H);
    out.V.status = ValueInfo::Status::Calculated;
    out.V.err = 0.0;
    // the heat capacity is corrected at subcritical T only
    const double Smax = values_[1] > 0.0 ? values_[1] : 1.0e-20;
    const double Tcritical = values_[0] + (values_[2] == 0.0 ? 0.0 : values_[2] * 1e5 / Smax * in.P / bar_to_Pa);
    if (in.T < Tcritical)
        out.Cp.propagateFrom(Cp);
    boundsOf<LandauHP98>(out, "Holland and Powell Landau model", applicability_, in);
}

auto LandauHP11::info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in) const -> void
{
    const ValueInfo G = out.G, S = out.S, H = out.H, V = out.V, Cp = out.Cp;
    out.G.propagateFrom(G);
    out.S.propagateFrom(S);
    out.H.propagateFrom(H);
    out.V.propagateFrom(V);
    const double Smax = values_[1] > 0.0 ? values_[1] : 1.0e-20;
    if (in.T < values_[0] + values_[2] * (in.P - in.Pr) / Smax)
        out.Cp.propagateFrom(Cp);
}

// The rule of Murnaghan and Tait: the terms need a volume that varies; if not, the volume is the reference volume
static auto variableVolumeInfo(PropertiesInfo& out, const PropertiesInfo& ref, bool variable) -> void
{
    const ValueInfo G = out.G, H = out.H, S = out.S, V = out.V, Cp = out.Cp;
    const ValueInfo volume = volumeInJPerBar(ref.V);
    out.G.propagateFrom(G, volume);
    out.H.propagateFrom(H, volume);
    if (variable)
    {
        out.S.propagateFrom(S, volume);
        out.V.propagateFrom(V, ref.V);
        out.Cp.propagateFrom(Cp, volume);
    }
    else
    {
        out.V.status = ref.V.status;
        out.V.err = ref.V.err;
    }
}

auto MurnaghanHP98::info(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in) const -> void
{
    const bool variable = (in.P != in.Pr || in.T != in.Tr) && std::fabs(in.Vr) > 1e-9 && std::fabs(values_[coefficients_]) > 1e-9 && std::fabs(values_[coefficients_ + 1] * 1e-8) > 1e-9;
    variableVolumeInfo(out, ref, variable);
    boundsOf<MurnaghanHP98>(out, "Holland and Powell Murnaghan model", applicability_, in);
}

auto TaitHP11::info(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in) const -> void
{
    variableVolumeInfo(out, ref, values_[coefficients_ + 1] != 0.0 && std::fabs(in.Vr) >= 1e-9);
    boundsOf<TaitHP11>(out, "Holland and Powell Tait model", applicability_, in);
}

// The rule of the molar volumes of Berman: the volume is the reference volume
static auto bermanInfo(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in) -> void
{
    const ValueInfo G = out.G, H = out.H, S = out.S;
    out.V.status = ref.V.status;
    out.V.err = ref.V.err;
    if (in.P != in.Pr || in.T != in.Tr)
    {
        const ValueInfo volume = volumeInJPerBar(ref.V);
        out.G.propagateFrom(G, volume);
        out.H.propagateFrom(H, volume);
        out.S.propagateFrom(S, volume);
        out.V.propagateFrom(ref.V);
    }
}

auto VolumeBerman88::info(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in) const -> void
{
    bermanInfo(out, ref, in);
    boundsOf<VolumeBerman88>(out, "Berman multisite model", applicability_, in);
}

auto VolumeDorogokupets88::info(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in) const -> void
{
    bermanInfo(out, ref, in);
    boundsOf<VolumeDorogokupets88>(out, "Berman multisite model", applicability_, in);
}

auto BirchMurnaghan47::info(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in) const -> void
{
    if (in.P != in.Pr || in.T != in.Tr)
    {
        const ValueInfo V = out.V, S = out.S, G = out.G, H = out.H;
        out.V.propagateFrom(V, ref.V);
        out.S.propagateFrom(S);
        out.G.propagateFrom(G);
        out.H.propagateFrom(H);
    }
    boundsOf<BirchMurnaghan47>(out, "BMGottschalk model", applicability_, in);
}

auto CpExtendedUNIQUAC::info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs&) const -> void
{
    allCalculated(out);
}

NASA9::NASA9(const MethodApplication& application)
{
    static const char* names[] = {"a1", "a2", "a3", "a4", "a5", "a6", "a7", "b1", "b2"};
    for (const auto& segment : application.segments)
    {
        std::vector<double> row;
        for (const char* n : names)
        {
            const auto it = segment.parameters.find(n);
            row.push_back(it == segment.parameters.end() ? 0.0 : it->second.value);
        }
        rows_.push_back(row);
        intervals_.push_back({segment.t_min, segment.t_max});
    }
}

template <typename Scalar>
auto NASA9::compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass(id);
    else
    {
        bool outside = false;
        setFromModel(state, thermoPropertiesNasaPolynomials(in.T, in.P / bar_to_Pa, NasaInputs{rows_, intervals_, valueOf(in.reference.V) / jPerBarToM3}, &outside));
        state.outside_range = outside;
    }
}

auto NASA9::info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in) const -> void
{
    allCalculated(out);
    if (in.outside)
        setMessage(out, "NASA polynomials: Outside temperature bounds");
}

TPTable::TPTable(const MethodApplication& application)
{
    if (application.table)
        table_ = *application.table;
}

template <typename Scalar>
auto TPTable::compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass(id);
    else
    {
        TableInputs inputs{table_.temperatures, table_.pressures, {}, table_.reference_pressure.value_or(0.0), valueOf(in.reference.V) / jPerBarToM3};
        static const std::pair<const char*, const char*> keys[] = {{"G", "gibbs_energy"}, {"H", "enthalpy"}, {"V", "volume"}, {"Cp", "heat_capacity_cp"}};
        for (const auto& [from, to] : keys)
            if (const auto it = table_.values.find(from); it != table_.values.end())
                inputs.values[to] = it->second;
        bool outside = false;
        setFromModel(state, thermoPropertiesTPTableInterpolation(in.T, in.P / bar_to_Pa, inputs, &outside));
        state.outside_range = outside;
    }
}

auto TPTable::info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in) const -> void
{
    allCalculated(out);
    auto has = [this](const char* key) { const auto it = table_.values.find(key); return it != table_.values.end() && !it->second.empty(); };
    if (!has("H"))
        out.H = out.S = ValueInfo{};
    if (!has("Cp"))
        out.Cp = ValueInfo{};
    if (in.outside)
        setMessage(out, "Interpolation table: Outside the table");
}

#define THERMOFUN_INSTANTIATE_MINERAL(M)                                                                               \
    template auto MineralMethod<M>::compute(SubstanceState<double>&, const SubstanceInputs<double>&) const -> void;    \
    template auto MineralMethod<M>::compute(SubstanceState<TPReal>&, const SubstanceInputs<TPReal>&) const -> void;

THERMOFUN_INSTANTIATE_MINERAL(LandauHP98)
THERMOFUN_INSTANTIATE_MINERAL(LandauHP11)
THERMOFUN_INSTANTIATE_MINERAL(MurnaghanHP98)
THERMOFUN_INSTANTIATE_MINERAL(TaitHP11)
THERMOFUN_INSTANTIATE_MINERAL(VolumeBerman88)
THERMOFUN_INSTANTIATE_MINERAL(VolumeDorogokupets88)
THERMOFUN_INSTANTIATE_MINERAL(BirchMurnaghan47)
THERMOFUN_INSTANTIATE_MINERAL(CpExtendedUNIQUAC)

#undef THERMOFUN_INSTANTIATE_MINERAL

template auto NASA9::compute(SubstanceState<double>&, const SubstanceInputs<double>&) const -> void;
template auto NASA9::compute(SubstanceState<TPReal>&, const SubstanceInputs<TPReal>&) const -> void;
template auto TPTable::compute(SubstanceState<double>&, const SubstanceInputs<double>&) const -> void;
template auto TPTable::compute(SubstanceState<TPReal>&, const SubstanceInputs<TPReal>&) const -> void;

} // namespace ThermoFun::v2
