// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_MINERALS_MINERALMETHODS_H
#define THERMOFUN_METHODS_MINERALS_MINERALMETHODS_H

#include <vector>

#include <ThermoFun/Methods/MethodBase.h>
#include <ThermoFun/Model/Applicability.h>
#include <ThermoFun/Model/MethodApplication.h>

namespace ThermoFun::v2 {

/// A correction of a mineral. Its parameters in SI, in the order of the catalog: the coefficients, then alpha0 and
/// kappa0 if the method has them. No coefficients: the record gives none, and the formulas raise their error.
template <typename Derived>
class MineralMethod : public SubstanceMethodBase<Derived>
{
public:
    MineralMethod(std::vector<double> coefficients, double alpha0, double kappa0, Applicability applicability)
    : SubstanceMethodBase<Derived>(withModuli(coefficients, alpha0, kappa0)), coefficients_(coefficients.size()), applicability_(std::move(applicability)) {}

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void;

protected:
    std::size_t coefficients_;
    Applicability applicability_;

private:
    static auto withModuli(std::vector<double> values, double alpha0, double kappa0) -> std::vector<double>
    {
        values.push_back(alpha0);
        values.push_back(kappa0);
        return values;
    }
};

#define THERMOFUN_MINERAL_METHOD(NAME, ID)                                                                             \
    class NAME final : public MineralMethod<NAME>                                                                      \
    {                                                                                                                  \
    public:                                                                                                            \
        static constexpr const char* id = ID;                                                                          \
        using MineralMethod<NAME>::MineralMethod;                                                                      \
        auto info(PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs& in) const -> void override;  \
    };

THERMOFUN_MINERAL_METHOD(LandauHP98, "LandauHP98")
THERMOFUN_MINERAL_METHOD(LandauHP11, "LandauHP11")
THERMOFUN_MINERAL_METHOD(MurnaghanHP98, "MurnaghanHP98")
THERMOFUN_MINERAL_METHOD(TaitHP11, "TaitHP11")
THERMOFUN_MINERAL_METHOD(VolumeBerman88, "VolumeBerman88")
THERMOFUN_MINERAL_METHOD(VolumeDorogokupets88, "VolumeDorogokupets88")
THERMOFUN_MINERAL_METHOD(BirchMurnaghan47, "BirchMurnaghan47")
THERMOFUN_MINERAL_METHOD(CpExtendedUNIQUAC, "CpExtendedUNIQUAC")

#undef THERMOFUN_MINERAL_METHOD

/// NASA9: the coefficients by T segment.
class NASA9 final : public SubstanceMethodBase<NASA9>
{
public:
    static constexpr const char* id = "NASA9";

    explicit NASA9(const MethodApplication& application);

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void;

    auto info(PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs& in) const -> void override;

private:
    std::vector<std::vector<double>> rows_, intervals_;
};

/// TPTable: a table in T and P.
class TPTable final : public SubstanceMethodBase<TPTable>
{
public:
    static constexpr const char* id = "TPTable";

    explicit TPTable(const MethodApplication& application);

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void;

    auto info(PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs& in) const -> void override;

private:
    TableData table_;
};

} // namespace ThermoFun::v2

#endif
