// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "FluidMethods.h"

#include <map>
#include <type_traits>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Methods/FunctionMethod.h>
#include <ThermoFun/Methods/Fluids/FluidCGF.h>
#include <ThermoFun/Methods/Fluids/FluidCORK.h>
#include <ThermoFun/Methods/Fluids/FluidPR78.h>
#include <ThermoFun/Methods/Fluids/FluidPRSV.h>
#include <ThermoFun/Methods/Fluids/FluidSRK.h>
#include <ThermoFun/Methods/Fluids/FluidSTP.h>
#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/Methods/MethodHelpers.h>

namespace ThermoFun::model {

namespace {

using Model = ThermoPropertiesSubstanceAD (*)(real T, real Pbar, const FluidInputs& inputs, ThermoPropertiesSubstanceAD state, bool cancel);

/// One equation of state. pressure_at_place_1: the parameter at place 1 is Pc, which the formula has in bar.
struct FluidRow
{
    Model model;
    const char* name;
    bool pressure_at_place_1;
};

struct FluidData
{
    FluidRecord record;
    const FluidRow* row = nullptr;
};

using Parameters = MethodParameters<FluidData>;

template <typename Scalar>
auto fluid(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in, const Parameters& parameters, const char* id) -> void
{
    if constexpr (std::is_same_v<Scalar, double>)
        needsAPass(id);
    else
    {
        const FluidData& data = parameters.data;
        FluidInputs inputs;
        inputs.lower_T = data.record.lower_T;
        inputs.formula = data.record.formula;
        inputs.constants = data.record.constants;
        for (std::size_t i = 0; i < parameters.values.size(); ++i)
        {
            const bool pressure = i == 1 && data.row->pressure_at_place_1;
            inputs.parameters.push_back(pressure ? parameters.value(i, in) / bar_to_Pa : parameters.value(i, in));
        }
        const Scalar p = in.P / bar_to_Pa;
        // G(T, P) = G(T, Pr) + RT ln(P/Pr) + RT ln(phi), so dG/dP = V, with two options for its two terms. The model gives the residual terms; H has no other term.
        auto out = data.row->model(in.T, p, inputs, toModel(state), in.options.cancel_fugacity_coefficient);
        if (in.options.apply_pressure_correction && p > 0.0)
        {
            const Scalar lnP = log(p / (in.reference.P / bar_to_Pa));
            out.gibbs_energy += R_CONSTANT * in.T * lnP;
            out.entropy -= R_CONSTANT * lnP;
        }
        setFromModel(state, out);
    }
}

auto fluidInfo(PropertiesInfo& out, const InfoInputs& in, const FluidData& data) -> void
{
    // the Gibbs energy, enthalpy and entropy are incremented, the volume is set to a value
    const ValueInfo G = out.G, H = out.H, S = out.S;
    out.G.propagateFrom(G);
    out.H.propagateFrom(H);
    out.S.propagateFrom(S);
    out.V = ValueInfo{ValueInfo::Status::Assigned, 0.0, ""};
    const auto message = boundsMessage(data.row->name, data.record.applicability, in.T, in.P);
    if (!message.empty())
        setMessage(out, message);
}

auto rowOf(const std::string& id) -> const FluidRow&
{
    static const std::map<std::string, FluidRow> rows = {
        {"PRSV86", {thermoPropertiesFluidPRSV, "PRSV Peng-Robinson-Stryjek-Vera fluid model", true}},
        {"ChurakovGottschalk03", {thermoPropertiesFluidCGF, "Churakov and Gottschalk fluid model", false}},
        {"SoaveRedlichKwong72", {thermoPropertiesFluidSRK, "SRK Soave-Redlich-Kwong fluid model", true}},
        {"SternerPitzer94", {thermoPropertiesFluidSTP, "STP Sterner-Pitzer fluid model", true}},
        {"PengRobinson78", {thermoPropertiesFluidPR78, "PR78 Peng-Robinson fluid model", true}},
        {"CorkHP91", {thermoPropertiesFluidCORK, "CORK compensated-Redlich-Kwong fluid model", true}},
    };
    const auto it = rows.find(id);
    if (it == rows.end())
        funError("Unknown fluid method", id, __LINE__, __FILE__);
    return it->second;
}

} // namespace

auto fluidMethod(const std::string& id, std::vector<double> parameters, FluidRecord record) -> std::shared_ptr<const SubstanceMethod>
{
    std::size_t expected = 0;
    if (const auto* spec = MethodCatalog::instance().find(id, OperatesOn::Substance))
        for (const auto& p : spec->parameters)
            expected += p.constant ? 1 : 0;
    if (record.constants.size() != expected)
        funError("Cannot make the method " + id, "It needs " + std::to_string(expected) + " constants and has " + std::to_string(record.constants.size()) + ".", __LINE__, __FILE__);
    return makeMethod(id.c_str(), Parameters{std::move(parameters), {std::move(record), &rowOf(id)}},
                      [id](auto& state, const auto& in, const Parameters& p) { fluid(state, in, p, id.c_str()); },
                      [](PropertiesInfo& out, const PropertiesInfo&, const InfoInputs& in, const Parameters& p) { fluidInfo(out, in, p.data); });
}

} // namespace ThermoFun::model
