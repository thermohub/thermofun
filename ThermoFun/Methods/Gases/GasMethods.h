// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_GASES_GASMETHODS_H
#define THERMOFUN_METHODS_GASES_GASMETHODS_H

#include <string>
#include <vector>

#include <ThermoFun/Methods/MethodBase.h>
#include <ThermoFun/Model/Applicability.h>

namespace ThermoFun::v2 {

/// What a fluid method knows of its record, beside the parameters.
struct GasRecord
{
    double lower_T = 298.15;     ///< K, the lower T of the heat capacity of the record
    std::string formula;         ///< CORK and Sterner-Pitzer have special forms for CO2 and H2O
    Applicability applicability;
};

/// A fluid equation of state: it adds its terms to G, H and S of the ideal gas, and sets V. Parameters in SI, in the
/// order of the catalog. The options of the engine say if G keeps RT ln(phi), and if the pressure term is added.
template <typename Derived>
class GasMethod : public SubstanceMethodBase<Derived>
{
public:
    GasMethod(std::vector<double> parameters, GasRecord record)
    : SubstanceMethodBase<Derived>(std::move(parameters)), record_(std::move(record)) {}

    template <typename Scalar>
    auto compute(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in) const -> void;

    auto info(PropertiesInfo& out, const PropertiesInfo& reference, const InfoInputs& in) const -> void override;

private:
    GasRecord record_;
};

#define THERMOFUN_GAS_METHOD(NAME, ID)                                                                                 \
    class NAME final : public GasMethod<NAME>                                                                          \
    {                                                                                                                  \
    public:                                                                                                            \
        static constexpr const char* id = ID;                                                                          \
        using GasMethod<NAME>::GasMethod;                                                                              \
    };

THERMOFUN_GAS_METHOD(PRSV86, "PRSV86")
THERMOFUN_GAS_METHOD(ChurakovGottschalk03, "ChurakovGottschalk03")
THERMOFUN_GAS_METHOD(SoaveRedlichKwong72, "SoaveRedlichKwong72")
THERMOFUN_GAS_METHOD(SternerPitzer94, "SternerPitzer94")
THERMOFUN_GAS_METHOD(PengRobinson78, "PengRobinson78")
THERMOFUN_GAS_METHOD(CorkHP91, "CorkHP91")

#undef THERMOFUN_GAS_METHOD

} // namespace ThermoFun::v2

#endif
