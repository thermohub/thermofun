// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_REACTIONS_REACTIONMETHODS_H
#define THERMOFUN_METHODS_REACTIONS_REACTIONMETHODS_H

#include <memory>
#include <vector>

#include <ThermoFun/Methods/MethodSpec.h>
#include <ThermoFun/Methods/Reactions/ReactionInputs.h>

namespace ThermoFun::v2 {

/// A method of a reaction, bound to its parameters. Immutable. Independent of the database, the engine and the record.
class ReactionMethod
{
public:
    virtual ~ReactionMethod() = default;

    /// The specification of the method.
    virtual auto spec() const -> const MethodSpec& = 0;

    /// The properties at T (K) and P (Pa), from those of the method before it. solvent: for a method that needs it.
    virtual auto calculate(double T, double P, const ThermoPropertiesReaction& incoming, const PropertiesSolvent* solvent) const -> ThermoPropertiesReaction = 0;
};

/// log K in one of its forms. The form ConstantCp is reduced to a simpler one if the record has fewer reference values.
auto makeLogK(const char* id, LogKForm form, ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>;

/// The reference properties at every T and P.
auto makeConstantReactionProperties(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>;

/// log K from a table in T: temperatures in K, ascending. order 0: natural cubic spline. Above 0: Lagrange polynomial of this
/// order through the nearest points, as in GEMS. Without a table the method does nothing, as in the legacy code.
auto makeLogKTable(std::vector<double> temperatures, std::vector<double> logK, int order, ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>;

auto makeDolejsManning10(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>;
auto makeMarshallFranck81(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>;
auto makeRyzhenkoBryzgalin(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>;
auto makeReactionVolumePolynomial(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>;
auto makeReactionConstantVolume(ReactionInputs inputs) -> std::shared_ptr<const ReactionMethod>;

/// The value, the first and the second derivative in T of an interpolation through (x, y).
struct InterpolatedValue
{
    real value, first, second;
};
auto naturalSpline(const std::vector<double>& x, const std::vector<double>& y, const real& at) -> InterpolatedValue;
auto lagrangePolynomial(const std::vector<double>& x, const std::vector<double>& y, const real& at, int order) -> InterpolatedValue;

/// The calculations of the methods, with their status rules, for the old classes too.
auto logKProperties(const ReactionInputs& inputs, LogKForm form, double T, double P) -> ThermoPropertiesReaction;
auto volumePolynomialProperties(const ReactionInputs& inputs, double T, double P, const ThermoPropertiesReaction& incoming) -> ThermoPropertiesReaction;
auto constantVolumeProperties(const ReactionInputs& inputs, double T, double P, const ThermoPropertiesReaction& incoming) -> ThermoPropertiesReaction;

} // namespace ThermoFun::v2

#endif
