// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_METHODREGISTRY_H
#define THERMOFUN_METHODS_METHODREGISTRY_H

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <ThermoFun/Methods/Method.h>
#include <ThermoFun/Methods/Water/SolventMethod.h>
#include <ThermoFun/Model/MethodApplication.h>
#include <ThermoFun/Model/SubstanceRecord.h>

namespace ThermoFun::v2 {

/// Makes the method objects from what a record says. It has one factory for each method id and implementation.
class MethodRegistry
{
public:
    /// Makes a method from an application and the record it belongs to.
    using SubstanceFactory = std::function<std::shared_ptr<const SubstanceMethod>(const MethodApplication&, const SubstanceRecord&)>;

    /// Makes an equation of state of water or a dielectric model.
    using SolventFactory = std::function<std::shared_ptr<const SolventMethod>(const MethodApplication&)>;
    using DielectricFactory = std::function<std::shared_ptr<const DielectricMethod>()>;

    /// The one registry of the library.
    static auto instance() -> MethodRegistry&;

    /// Add a factory. An implementation of "" is the default of the method.
    auto add(const std::string& id, const std::string& implementation, SubstanceFactory factory) -> void;

    /// Remove a factory.
    auto remove(const std::string& id, const std::string& implementation) -> void;

    /// True if there is code for this method id.
    auto hasSubstanceMethod(const std::string& id) const -> bool;

    /// The method object of an application. Raises an error if there is no code for it.
    auto create(const MethodApplication& application, const SubstanceRecord& record = {}) const -> std::shared_ptr<const SubstanceMethod>;

    /// True if there is code for this id as an equation of state of water or a dielectric model.
    auto hasSolventMethod(const std::string& id) const -> bool;

    /// The equation of state of an application. Raises an error if there is no code for it.
    auto createSolvent(const MethodApplication& application) const -> std::shared_ptr<const SolventMethod>;

    /// The dielectric model of an application. Raises an error if there is no code for it.
    auto createDielectric(const MethodApplication& application) const -> std::shared_ptr<const DielectricMethod>;

private:
    MethodRegistry();
    std::map<std::pair<std::string, std::string>, SubstanceFactory> substance_;
    std::map<std::pair<std::string, std::string>, SolventFactory> solvent_;
    std::map<std::pair<std::string, std::string>, DielectricFactory> dielectric_;
};

} // namespace ThermoFun::v2

#endif
