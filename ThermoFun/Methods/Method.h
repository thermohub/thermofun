// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_METHOD_H
#define THERMOFUN_METHODS_METHOD_H

#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <ThermoFun/Common/Real.hpp>
#include <ThermoFun/Methods/MethodSpec.h>

namespace ThermoFun::v2 {

/// The number type of a pass with derivatives: the derivative is with respect to T, to P or to one parameter.
using TPReal = ThermoFun::real;

/// The properties of a substance during an evaluation. SI: J/mol, J/mol, J/(mol K), J/(mol K), m3/mol.
template <typename Scalar>
struct SubstanceState
{
    Scalar G{}, H{}, S{}, Cp{}, V{};
    bool outside_range = false;  ///< the method used a value outside its range, for example the nearest T segment
};

/// The reference state of a record: T and P (K, Pa), and the standard-state values that the record gives (SI).
template <typename Scalar>
struct ReferenceState
{
    double T = 298.15;
    double P = 1e5;
    std::optional<Scalar> G, H, S, Cp, V;
};

/// How far a value can be trusted: its status and its error.
struct ValueInfo
{
    enum class Status { NotDefined, Read, Calculated, Assigned };
    Status status = Status::NotDefined;
    double err = 0.0;
    std::string message;

    /// The status and the error of a value calculated from others: the errors add in quadrature, and it is not defined if one of them is not.
    template <typename... Values>
    auto propagateFrom(const Values&... values) -> ValueInfo&
    {
        status = Status::Calculated;
        err = 0.0;
        ((status = (status == Status::NotDefined || values.status == Status::NotDefined) ? Status::NotDefined : Status::Calculated,
          err = std::sqrt(err * err + values.err * values.err)), ...);
        return *this;
    }

    /// The error of a value from the errors of others with their factors: {factor, error}, ...
    auto setError(std::initializer_list<std::pair<double, double>> terms) -> ValueInfo&
    {
        double sum = 0.0;
        for (const auto& [factor, error] : terms)
            sum += factor * factor * error * error;
        err = std::sqrt(sum);
        return *this;
    }
};

/// The status and the error of each property.
struct PropertiesInfo
{
    ValueInfo G, H, S, Cp, V;
};

/// The state of the water solvent at T and P.
template <typename Scalar>
struct SolventState;

/// The options of an evaluation that change what a method does.
struct SubstanceOptions
{
    bool apply_pressure_correction = false;  ///< gases: add the pressure term to G and H
    bool cancel_fugacity_coefficient = true; ///< fluids: G is the standard-state value
};

/// What a substance method gets at each call.
template <typename Scalar>
struct SubstanceInputs
{
    Scalar T;                                       ///< K
    Scalar P;                                       ///< Pa
    const ReferenceState<Scalar>& reference;
    const SolventState<Scalar>* solvent = nullptr;  ///< set if the method needs it
    const SolventState<Scalar>* solvent_at_reference = nullptr;
    const SubstanceState<Scalar>* ideal_gas_water = nullptr;
    int seeded_parameter = -1;                      ///< the parameter of this method that the pass derives by; -1: none
    SubstanceOptions options{};
};

/// What the status and the errors of a method depend on. K, Pa.
struct InfoInputs
{
    double T = 0.0, P = 0.0, Tr = 298.15, Pr = 1e5;
    bool outside = false;  ///< the method used a value outside its range
    SubstanceOptions options{};
};

/// A substance method, bound to its parameter values. Immutable. Independent of the database, the engine and the records.
class SubstanceMethod
{
public:
    virtual ~SubstanceMethod() = default;

    /// The specification of the method.
    virtual auto spec() const -> const MethodSpec& = 0;

    /// Apply the method to the incoming state, as its effects declare.
    virtual auto apply(SubstanceState<double>& state, const SubstanceInputs<double>& in) const -> void = 0;
    virtual auto apply(SubstanceState<TPReal>& state, const SubstanceInputs<TPReal>& in) const -> void = 0;

    /// The parameters in SI. A method with T segments gives those of one segment after the other.
    virtual auto parameterValues() const -> std::vector<double> = 0;

    /// A copy of this method with one parameter changed (SI). The original does not change.
    virtual auto withParameter(std::size_t index, double value_si) const -> std::shared_ptr<const SubstanceMethod> = 0;

    /// The status and the error of what the method gives. out holds those of the incoming properties.
    virtual auto info(PropertiesInfo& /*out*/, const PropertiesInfo& /*reference*/, const InfoInputs&) const -> void {}

    /// Named outputs beside the properties, calculated only on request.
    virtual auto auxiliary(const SubstanceInputs<double>&) const -> std::map<std::string, double> { return {}; }
};

} // namespace ThermoFun::v2

#endif
