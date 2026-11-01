// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_FUNCTIONMETHOD_H
#define THERMOFUN_METHODS_FUNCTIONMETHOD_H

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Methods/MethodCatalog.h>

namespace ThermoFun::v2 {

/// For a method that needs nothing beside its numbers.
struct NoData
{
};

/// What a formula gets from its record: the parameters that can be fitted, in SI, and what else it needs and does not fit.
template <typename Data = NoData>
struct MethodParameters
{
    std::vector<double> values;
    Data data;
    std::vector<std::string> names;  ///< of the values, if they are not those of the catalog

    /// The value of parameter i in this pass: the seeded parameter carries the derivative.
    template <typename Scalar>
    auto value(std::size_t i, const SubstanceInputs<Scalar>& in) const -> Scalar
    {
        Scalar p = values[i];
        if constexpr (!std::is_same_v<Scalar, double>)
            if (in.seeded_parameter == static_cast<int>(i))
                p[1] = 1.0;
        return p;
    }
};

/// A substance method made from a formula and its status rule, which are functions. All that a method needs for the engine is here, once:
///   formula(state, inputs, parameters): generic, called for each scalar
///   info(out, reference, infoInputs, parameters): the status and the error of what the formula gives
template <typename Data, typename Formula, typename Info>
class FunctionMethod final : public SubstanceMethod
{
public:
    FunctionMethod(const char* id, MethodParameters<Data> parameters, Formula formula, Info info)
    : spec_(MethodCatalog::instance().find(id, OperatesOn::Substance)), parameters_(std::move(parameters)), formula_(std::move(formula)), info_(std::move(info))
    {
    }

    auto spec() const -> const MethodSpec& override { return *spec_; }
    auto apply(SubstanceState<double>& state, const SubstanceInputs<double>& in) const -> void override { formula_(state, in, parameters_); }
    auto apply(SubstanceState<TPReal>& state, const SubstanceInputs<TPReal>& in) const -> void override { formula_(state, in, parameters_); }
    auto parameterValues() const -> std::vector<double> override { return parameters_.values; }

    auto parameterNames() const -> std::vector<std::string> override
    {
        if (!parameters_.names.empty())
            return parameters_.names;
        // the moduli of a mineral come after its coefficients
        std::vector<std::string> names;
        for (const auto& p : spec_->parameters)
            if (!p.constant && p.name != "alpha0" && p.name != "kappa0")
                names.push_back(p.name);
        for (const auto& p : spec_->parameters)
            if (p.name == "alpha0" || p.name == "kappa0")
                names.push_back(p.name);
        if (names.size() != parameters_.values.size())
            names.clear();
        return names;
    }

    auto withParameter(std::size_t index, double value_si) const -> std::shared_ptr<const SubstanceMethod> override
    {
        if (index >= parameters_.values.size())
            funError("Cannot change a parameter of the method " + spec_->id,
                     "It has " + std::to_string(parameters_.values.size()) + " parameters, the index is " + std::to_string(index) + ".", __LINE__, __FILE__);
        auto copy = std::make_shared<FunctionMethod>(*this);
        copy->parameters_.values[index] = value_si;
        return copy;
    }

    auto info(PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs& in) const -> void override { info_(out, reference, in, parameters_); }

private:
    const MethodSpec* spec_;
    MethodParameters<Data> parameters_;
    Formula formula_;
    Info info_;
};

/// The method of a formula and a status rule, with these parameters.
template <typename Data, typename Formula, typename Info>
auto makeMethod(const char* id, MethodParameters<Data> parameters, Formula formula, Info info) -> std::shared_ptr<const SubstanceMethod>
{
    return std::make_shared<const FunctionMethod<Data, Formula, Info>>(id, std::move(parameters), std::move(formula), std::move(info));
}

} // namespace ThermoFun::v2

#endif
