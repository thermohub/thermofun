// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_METHODBASE_H
#define THERMOFUN_METHODS_METHODBASE_H

#include <utility>

#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Methods/MethodCatalog.h>

namespace ThermoFun::v2 {

/// What is common to all substance methods: the parameter values, and the two apply functions that call the one
/// template with the formulas. Derived has `static constexpr const char* id` and
/// `template <typename Scalar> auto compute(SubstanceState<Scalar>&, const SubstanceInputs<Scalar>&) const -> void`.
template <typename Derived>
class SubstanceMethodBase : public SubstanceMethod
{
public:
    auto spec() const -> const MethodSpec& override
    {
        static const MethodSpec* const s = MethodCatalog::instance().find(Derived::id, OperatesOn::Substance);
        return *s;
    }

    auto apply(SubstanceState<double>& state, const SubstanceInputs<double>& in) const -> void override
    {
        static_cast<const Derived&>(*this).compute(state, in);
    }

    auto apply(SubstanceState<TPReal>& state, const SubstanceInputs<TPReal>& in) const -> void override
    {
        static_cast<const Derived&>(*this).compute(state, in);
    }

    auto parameterValues() const -> std::vector<double> override { return values_; }

    auto withParameter(std::size_t index, double value_si) const -> std::shared_ptr<const SubstanceMethod> override
    {
        if (index >= values_.size())
            funError("Cannot change a parameter of the method " + spec().id,
                     "It has " + std::to_string(values_.size()) + " parameters, the index is " + std::to_string(index) + ".", __LINE__, __FILE__);
        auto copy = std::make_shared<Derived>(static_cast<const Derived&>(*this));
        copy->values_[index] = value_si;
        return copy;
    }

protected:
    explicit SubstanceMethodBase(std::vector<double> values = {}) : values_(std::move(values)) {}

    /// The value of parameter i in this pass: the seeded parameter carries the derivative.
    template <typename Scalar>
    auto parameter(std::size_t i, const SubstanceInputs<Scalar>& in) const -> Scalar
    {
        Scalar p = values_[i];
        if constexpr (!std::is_same_v<Scalar, double>)
            if (in.seeded_parameter == static_cast<int>(i))
                p[1] = 1.0;
        return p;
    }

    std::vector<double> values_;
};

} // namespace ThermoFun::v2

#endif
