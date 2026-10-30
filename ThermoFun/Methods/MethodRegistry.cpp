// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "MethodRegistry.h"

#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/Minerals/ConstantCp.h>
#include <ThermoFun/Methods/Minerals/ConstantProperties.h>
#include <ThermoFun/Methods/Minerals/CpPolynomial.h>

namespace ThermoFun::v2 {

MethodRegistry::MethodRegistry()
{
    add("ConstantProperties", "", [](const MethodApplication&) { return std::make_shared<const ConstantProperties>(); });
    add("ConstantCp", "", [](const MethodApplication&) { return std::make_shared<const ConstantCp>(); });
    add("CpPolynomial", "", [](const MethodApplication& a) { return std::make_shared<const CpPolynomial>(CpPolynomial::fromApplication(a)); });
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

auto MethodRegistry::hasSubstanceMethod(const std::string& id) const -> bool
{
    const auto it = substance_.lower_bound({id, ""});
    return it != substance_.end() && it->first.first == id;
}

auto MethodRegistry::create(const MethodApplication& application) const -> std::shared_ptr<const SubstanceMethod>
{
    auto it = substance_.find({application.method, application.implementation});
    if (it == substance_.end())
        it = substance_.find({application.method, ""});
    if (it == substance_.end())
        funError("Cannot make the method " + application.method, "There is no code for this method and implementation \"" + application.implementation + "\".", __LINE__, __FILE__);
    return it->second(application);
}

} // namespace ThermoFun::v2
