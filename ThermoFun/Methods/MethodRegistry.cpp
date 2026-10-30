// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "MethodRegistry.h"

#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/Minerals/ConstantCp.h>
#include <ThermoFun/Methods/Minerals/ConstantProperties.h>
#include <ThermoFun/Methods/Minerals/ConstantVolume.h>
#include <ThermoFun/Methods/Minerals/CpPolynomial.h>
#include <ThermoFun/Methods/Minerals/IdealGas.h>

namespace ThermoFun::v2 {

MethodRegistry::MethodRegistry()
{
    using A = const MethodApplication&;
    using R = const SubstanceRecord&;
    add("ConstantProperties", "", [](A, R) { return std::make_shared<const ConstantProperties>(); });
    add("ConstantCp", "", [](A, R) { return std::make_shared<const ConstantCp>(); });
    add("CpPolynomial", "", [](A a, R r) { return std::make_shared<const CpPolynomial>(CpPolynomial::fromApplication(a, r.symbol)); });
    add("ConstantVolume", "", [](A, R) { return std::make_shared<const ConstantVolume>(); });
    add("IdealGas", "", [](A, R r) { return std::make_shared<const IdealGas>(r.class_ == SubstanceClass::GasFluid); });
}

auto MethodRegistry::instance() -> MethodRegistry&
{
    static MethodRegistry registry;
    return registry;
}

auto MethodRegistry::add(const std::string& id, const std::string& implementation, SubstanceFactory factory) -> void
{
    substance_[{id, implementation}] = std::move(factory);
}

auto MethodRegistry::remove(const std::string& id, const std::string& implementation) -> void
{
    substance_.erase({id, implementation});
}

auto MethodRegistry::hasSubstanceMethod(const std::string& id) const -> bool
{
    const auto it = substance_.lower_bound({id, ""});
    return it != substance_.end() && it->first.first == id;
}

auto MethodRegistry::create(const MethodApplication& application, const SubstanceRecord& record) const -> std::shared_ptr<const SubstanceMethod>
{
    auto it = substance_.find({application.method, application.implementation});
    if (it == substance_.end())
        it = substance_.find({application.method, ""});
    if (it == substance_.end())
        funError("Cannot make the method " + application.method, "There is no code for this method and implementation \"" + application.implementation + "\".", __LINE__, __FILE__);
    return it->second(application, record);
}

} // namespace ThermoFun::v2
