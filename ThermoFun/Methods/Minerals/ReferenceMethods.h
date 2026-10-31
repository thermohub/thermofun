// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_MINERALS_REFERENCEMETHODS_H
#define THERMOFUN_METHODS_MINERALS_REFERENCEMETHODS_H

#include <memory>

#include <ThermoFun/Methods/Method.h>

namespace ThermoFun::v2 {

/// The reference properties at every T and P. A property that the record does not give is 0.
auto constantPropertiesMethod() -> std::shared_ptr<const SubstanceMethod>;

/// Integrates the constant reference Cp from the reference S, H and G. Without S or Cp in the record, the terms that need it are left out.
auto constantCpMethod() -> std::shared_ptr<const SubstanceMethod>;

/// Adds V dP to G and H, with the reference volume. Without a reference volume it does nothing.
auto constantVolumeMethod() -> std::shared_ptr<const SubstanceMethod>;

/// Sets V = R T / P for a gas. With the pressure correction switched on, it also adds V (P - Pr) to G and H.
/// isGas: the volume comes from the gas law only for a record of the class gas_fluid.
auto idealGasMethod(bool isGas) -> std::shared_ptr<const SubstanceMethod>;

} // namespace ThermoFun::v2

#endif
