// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_SOLUTES_SOLUTEMETHODS_H
#define THERMOFUN_METHODS_SOLUTES_SOLUTEMETHODS_H

#include <string>
#include <vector>

#include <ThermoFun/Methods/MethodBase.h>
#include <ThermoFun/Model/Applicability.h>

namespace ThermoFun::v2 {

/// What the HKF method knows of its species, beside the parameters.
struct HKFSpecies
{
    std::string symbol;
    double charge = 0.0;
    bool hydrogen_ion = false;
    Applicability applicability;
};

/// HKF88, in the implementation given by Derived. The parameters in SI, in the order of the catalog: a1, a2, a3, a4,
/// c1, c2, omega. No parameters: the method raises the error of a record without them.
template <typename Derived>
class HKF88Base : public SubstanceMethodBase<Derived>
{
public:
    static constexpr const char* id = "HKF88";

    HKF88Base(std::vector<double> parameters, HKFSpecies species)
    : SubstanceMethodBase<Derived>(std::move(parameters)), species_(std::move(species)) {}

    auto info(PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs& in) const -> void override;

protected:
    HKFSpecies species_;
};

/// HKF88, implementation reaktoro
class HKF88Reaktoro final : public HKF88Base<HKF88Reaktoro>
{
public:
    using HKF88Base::HKF88Base;

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void;
};

/// HKF88, implementation gems
class HKF88Gems final : public HKF88Base<HKF88Gems>
{
public:
    using HKF88Base::HKF88Base;

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void;
};

/// SoluteHP98: the density model of Holland and Powell (1998). The parameter: b, J/(mol K2).
class SoluteHP98 final : public SubstanceMethodBase<SoluteHP98>
{
public:
    static constexpr const char* id = "SoluteHP98";

    SoluteHP98(std::vector<double> parameters, std::string symbol)
    : SubstanceMethodBase(std::move(parameters)), symbol_(std::move(symbol)) {}

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void;

    auto info(PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs& in) const -> void override;

private:
    std::string symbol_;
};

/// AndersonEtAl91: the density model of Anderson et al. (1991).
class AndersonEtAl91 final : public SubstanceMethodBase<AndersonEtAl91>
{
public:
    static constexpr const char* id = "AndersonEtAl91";

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void;

    auto info(PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs& in) const -> void override;
};

/// AkinfievDiamond03: from the state of the base method and of water. The parameters in SI: xi, a, b.
class AkinfievDiamond03 final : public SubstanceMethodBase<AkinfievDiamond03>
{
public:
    static constexpr const char* id = "AkinfievDiamond03";

    AkinfievDiamond03(std::vector<double> parameters, std::string symbol)
    : SubstanceMethodBase(std::move(parameters)), symbol_(std::move(symbol)) {}

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void;

    auto info(PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs& in) const -> void override;

private:
    std::string symbol_;
};

} // namespace ThermoFun::v2

#endif
