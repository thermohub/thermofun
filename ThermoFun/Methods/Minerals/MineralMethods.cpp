// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "MineralMethods.h"

#include <cmath>
#include <map>
#include <type_traits>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Methods/FunctionMethod.h>
#include <ThermoFun/Methods/MethodHelpers.h>
#include <ThermoFun/Methods/Minerals/SolidBMGottschalk.h>
#include <ThermoFun/Methods/Minerals/SolidBerman88.h>
#include <ThermoFun/Methods/Minerals/SolidHPLandau.h>
#include <ThermoFun/Methods/Minerals/SolidLandauHP11.h>
#include <ThermoFun/Methods/Minerals/SolidMurnaghanHP98.h>
#include <ThermoFun/Methods/Minerals/SolidTaitHP11.h>
#include <ThermoFun/Methods/Minerals/StandardThermoModelsExtra.h>

namespace ThermoFun::model {

namespace {

template <typename Scalar>
auto valueOf(const std::optional<Scalar>& x) -> Scalar
{
    return x ? *x : Scalar(0.0);
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

struct MineralRow;

/// What a mineral formula gets beside its numbers. values: the coefficients, then alpha0 and kappa0.
struct MineralData
{
    std::size_t coefficients = 0;
    Applicability applicability;
    const MineralRow* row = nullptr;
};

using Parameters = MethodParameters<MineralData>;
using Model = ThermoPropertiesSubstanceAD (*)(const real& T, const real& Pbar, const MineralInputs& inputs, const ThermoPropertiesSubstanceAD& state);
using StatusRule = void (*)(PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs& in, const Parameters& p);

/// One mineral method: its formula, the units of its coefficients and its status rule.
struct MineralRow
{
    Model model;
    std::vector<Unit> units;
    StatusRule info;
};

template <typename Scalar>
auto mineral(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in, const Parameters& p, const char* id) -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass(id);
    else
    {
        auto inputs = inputsOf(in);
        const auto& units = p.data.row->units;
        for (std::size_t i = 0; i < p.data.coefficients; ++i)
        {
            const auto unit = i < units.size() ? units[i] : Unit{};
            inputs.coefficients.push_back(p.value(i, in) * unit.factor + unit.offset);
        }
        inputs.alpha0 = p.value(p.data.coefficients, in);
        inputs.kappa0 = p.value(p.data.coefficients + 1, in) * 1e-8;  // kbar
        setFromModel(state, p.data.row->model(in.T, in.P / bar_to_Pa, inputs, toModel(state)));
    }
}

auto boundsOf(PropertiesInfo& out, const char* model, const Applicability& a, const InfoInputs& in) -> void
{
    const auto message = boundsMessage(model, a, in.T, in.P);
    if (!message.empty())
        setMessage(out, message);
}

auto landauHP98Info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in, const Parameters& p) -> void
{
    const ValueInfo G = out.G, S = out.S, H = out.H, Cp = out.Cp;
    out.G.propagateFrom(G);
    out.S.propagateFrom(S);
    out.H.propagateFrom(H);
    out.V.status = ValueInfo::Status::Calculated;
    out.V.err = 0.0;
    // the heat capacity is corrected at subcritical T only
    const double Smax = p.values[1] > 0.0 ? p.values[1] : 1.0e-20;
    const double Tcritical = p.values[0] + (p.values[2] == 0.0 ? 0.0 : p.values[2] * 1e5 / Smax * in.P / bar_to_Pa);
    if (in.T < Tcritical)
        out.Cp.propagateFrom(Cp);
    boundsOf(out, "Holland and Powell Landau model", p.data.applicability, in);
}

auto landauHP11Info(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in, const Parameters& p) -> void
{
    const ValueInfo G = out.G, S = out.S, H = out.H, V = out.V, Cp = out.Cp;
    out.G.propagateFrom(G);
    out.S.propagateFrom(S);
    out.H.propagateFrom(H);
    out.V.propagateFrom(V);
    const double Smax = p.values[1] > 0.0 ? p.values[1] : 1.0e-20;
    if (in.T < p.values[0] + p.values[2] * (in.P - in.Pr) / Smax)
        out.Cp.propagateFrom(Cp);
}

// The rule of Murnaghan and Tait: the terms need a volume that varies; if not, the volume is the reference volume
auto variableVolumeInfo(PropertiesInfo& out, const PropertiesInfo& ref, bool variable) -> void
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

auto murnaghanHP98Info(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in, const Parameters& p) -> void
{
    const bool variable = (in.P != in.Pr || in.T != in.Tr) && std::fabs(in.Vr) > 1e-9 && std::fabs(p.values[p.data.coefficients]) > 1e-9 && std::fabs(p.values[p.data.coefficients + 1] * 1e-8) > 1e-9;
    variableVolumeInfo(out, ref, variable);
    boundsOf(out, "Holland and Powell Murnaghan model", p.data.applicability, in);
}

auto taitHP11Info(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in, const Parameters& p) -> void
{
    variableVolumeInfo(out, ref, p.values[p.data.coefficients + 1] != 0.0 && std::fabs(in.Vr) >= 1e-9);
    boundsOf(out, "Holland and Powell Tait model", p.data.applicability, in);
}

// The rule of the molar volumes of Berman: the volume is the reference volume
auto bermanInfo(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in, const Parameters& p) -> void
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
    boundsOf(out, "Berman multisite model", p.data.applicability, in);
}

auto birchMurnaghan47Info(PropertiesInfo& out, const PropertiesInfo& ref, const InfoInputs& in, const Parameters& p) -> void
{
    if (in.P != in.Pr || in.T != in.Tr)
    {
        const ValueInfo V = out.V, S = out.S, G = out.G, H = out.H;
        out.V.propagateFrom(V, ref.V);
        out.S.propagateFrom(S);
        out.G.propagateFrom(G);
        out.H.propagateFrom(H);
    }
    boundsOf(out, "BMGottschalk model", p.data.applicability, in);
}

auto extendedUNIQUACInfo(PropertiesInfo& out, const PropertiesInfo&, const InfoInputs&, const Parameters&) -> void
{
    allCalculated(out);
}

auto rowOf(const std::string& id) -> const MineralRow&
{
    static const std::map<std::string, MineralRow> rows = {
        {"LandauHP98", {[](const real& T, const real& P, const MineralInputs& i, const ThermoPropertiesSubstanceAD& s) { return thermoPropertiesHPLandau(T, P, i, s); },
                        {{1.0, -C_to_K}, {}, {1e5, 0.0}}, landauHP98Info}},                      // Tc0 degC, Smax, Vmax J/bar
        {"LandauHP11", {[](const real& T, const real& P, const MineralInputs& i, const ThermoPropertiesSubstanceAD& s) { return thermoPropertiesHP11Landau(T, P, i, s); },
                        {{1.0, -C_to_K}, {}, {1e5, 0.0}}, landauHP11Info}},
        {"MurnaghanHP98", {[](const real& T, const real& P, const MineralInputs& i, const ThermoPropertiesSubstanceAD& s) { return thermoPropertiesMinMurnaghanEOSHP98(T, P, i, s); },
                           {}, murnaghanHP98Info}},
        {"TaitHP11", {[](const real& T, const real& P, const MineralInputs& i, const ThermoPropertiesSubstanceAD& s) { return thermoPropertiesMinTaitHP11(T, P, i, s); },
                      {{}, {1e8, 0.0}, {}}, taitHP11Info}},                                      // kappa0p, kappa0pp 1/kbar, n_atoms
        {"VolumeBerman88", {[](const real& T, const real& P, const MineralInputs& i, const ThermoPropertiesSubstanceAD& s) { return thermoPropertiesMinBerman88(T, P, i, s); },
                            {{}, {}, {}, {1e5, 0.0}, {1e10, 0.0}}, bermanInfo}},                 // a1, a2, a3, b1 1/bar, b2 1/bar2
        {"VolumeDorogokupets88", {[](const real& T, const real& P, const MineralInputs& i, const ThermoPropertiesSubstanceAD& s) { return thermoPropertiesMinBerman88(T, P, i, s); },
                                  {{}, {}, {}, {1e5, 0.0}, {1e10, 0.0}}, bermanInfo}},
        {"BirchMurnaghan47", {[](const real& T, const real& P, const MineralInputs& i, const ThermoPropertiesSubstanceAD& s) { return thermoPropertiesMinBMGottschalk(T, P, i, s); },
                              {{}, {}, {}, {0.0, 0.0}, {0.0, 0.0}, {1e-6, 0.0}, {1e-6, 0.0}, {}, {1e6, 0.0}}, birchMurnaghan47Info}},  // two unused places; MPa
        {"CpExtendedUNIQUAC", {[](const real& T, const real& P, const MineralInputs& i, const ThermoPropertiesSubstanceAD&) { return thermoPropertiesExtendedUNIQUAC(T, P, i); },
                               {{}, {}, {}, {1e5, 0.0}, {1e10, 0.0}, {}}, extendedUNIQUACInfo}},  // a, b, c, alpha 1/bar, beta 1/bar2, Theta
    };
    const auto it = rows.find(id);
    if (it == rows.end())
        funError("Unknown mineral method", id, __LINE__, __FILE__);
    return it->second;
}

struct NasaData
{
    std::vector<std::vector<double>> rows, intervals;
};

template <typename Scalar>
auto nasa9(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in, const NasaData& data) -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass("NASA9");
    else
    {
        bool outside = false;
        setFromModel(state, thermoPropertiesNasaPolynomials(in.T, in.P / bar_to_Pa, NasaInputs{data.rows, data.intervals, valueOf(in.reference.V) / jPerBarToM3}, &outside));
        state.outside_range = outside;
    }
}

auto nasa9Info(PropertiesInfo& out, const InfoInputs& in) -> void
{
    allCalculated(out);
    if (in.outside)
        setMessage(out, "NASA polynomials: Outside temperature bounds");
}

template <typename Scalar>
auto tpTable(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in, const TableData& table) -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass("TPTable");
    else
    {
        TableInputs inputs{table.temperatures, table.pressures, {}, table.reference_pressure.value_or(0.0), valueOf(in.reference.V) / jPerBarToM3};
        static const std::pair<const char*, const char*> keys[] = {{"G", "gibbs_energy"}, {"H", "enthalpy"}, {"V", "volume"}, {"Cp", "heat_capacity_cp"}};
        for (const auto& [from, to] : keys)
            if (const auto it = table.values.find(from); it != table.values.end())
                inputs.values[to] = it->second;
        bool outside = false;
        setFromModel(state, thermoPropertiesTPTableInterpolation(in.T, in.P / bar_to_Pa, inputs, &outside));
        state.outside_range = outside;
    }
}

auto tpTableInfo(PropertiesInfo& out, const InfoInputs& in, const TableData& table) -> void
{
    allCalculated(out);
    auto has = [&table](const char* key) { const auto it = table.values.find(key); return it != table.values.end() && !it->second.empty(); };
    if (!has("H"))
        out.H = out.S = ValueInfo{};
    if (!has("Cp"))
        out.Cp = ValueInfo{};
    if (in.outside)
        setMessage(out, "Interpolation table: Outside the table");
}

} // namespace

auto mineralMethod(const std::string& id, std::vector<double> coefficients, double alpha0, double kappa0, Applicability applicability, std::vector<std::string> names) -> std::shared_ptr<const SubstanceMethod>
{
    const MineralRow& row = rowOf(id);
    const std::size_t count = coefficients.size();
    coefficients.push_back(alpha0);
    coefficients.push_back(kappa0);
    if (!names.empty())
    {
        names.push_back("alpha0");
        names.push_back("kappa0");
    }
    return makeMethod(id.c_str(), Parameters{std::move(coefficients), {count, std::move(applicability), &row}, std::move(names)},
                      [id](auto& state, const auto& in, const Parameters& p) { mineral(state, in, p, id.c_str()); },
                      [](PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs& in, const Parameters& p) { p.data.row->info(out, reference, in, p); });
}

auto nasa9Method(const MethodApplication& application) -> std::shared_ptr<const SubstanceMethod>
{
    static const char* names[] = {"a1", "a2", "a3", "a4", "a5", "a6", "a7", "b1", "b2"};
    NasaData data;
    for (const auto& segment : application.segments)
    {
        std::vector<double> row;
        for (const char* n : names)
        {
            const auto it = segment.parameters.find(n);
            row.push_back(it == segment.parameters.end() ? 0.0 : it->second.value);
        }
        data.rows.push_back(row);
        data.intervals.push_back({segment.t_min, segment.t_max});
    }
    return makeMethod("NASA9", MethodParameters<NasaData>{{}, std::move(data)},
                      [](auto& state, const auto& in, const auto& p) { nasa9(state, in, p.data); },
                      [](PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in, const auto&) { nasa9Info(out, in); });
}

auto tpTableMethod(const MethodApplication& application) -> std::shared_ptr<const SubstanceMethod>
{
    return makeMethod("TPTable", MethodParameters<TableData>{{}, application.table.value_or(TableData{})},
                      [](auto& state, const auto& in, const auto& p) { tpTable(state, in, p.data); },
                      [](PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in, const auto& p) { tpTableInfo(out, in, p.data); });
}

} // namespace ThermoFun::model
