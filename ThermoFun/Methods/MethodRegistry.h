// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_METHODREGISTRY_H
#define THERMOFUN_METHODS_METHODREGISTRY_H

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Model/MethodApplication.h>

namespace ThermoFun::v2 {

/// Makes the method objects from what a record says. It has one factory for each method id and implementation.
class MethodRegistry
{
public:
    using SubstanceFactory = std::function<std::shared_ptr<const SubstanceMethod>(const MethodApplication&)>;

    /// The one registry of the library.
    static auto instance() -> MethodRegistry&;

    /// Add a factory. An implementation of "" is the default of the method.
    auto add(const std::string& id, const std::string& implementation, SubstanceFactory factory) -> void;

    /// True if there is code for this method id.
    auto hasSubstanceMethod(const std::string& id) const -> bool;

    /// The method object of an application. Raises an error if there is no code for it.
    auto create(const MethodApplication& application) const -> std::shared_ptr<const SubstanceMethod>;

private:
    MethodRegistry();
    std::map<std::pair<std::string, std::string>, SubstanceFactory> substance_;
};

} // namespace ThermoFun::v2

#endif
