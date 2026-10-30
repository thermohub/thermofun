// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "MethodRegistry.h"

#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/Minerals/ConstantCp.h>
#include <ThermoFun/Methods/Minerals/ConstantProperties.h>
#include <ThermoFun/Methods/Minerals/ConstantVolume.h>
#include <ThermoFun/Methods/Minerals/CpPolynomial.h>
#include <ThermoFun/Methods/Minerals/IdealGas.h>
#include <ThermoFun/Methods/Water/WaterMethods.h>

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

    // "" is the default implementation, the first one of the catalog
    auto solvent = [this](const char* id, const char* implementation, SolventFactory f) {
        solvent_[{id, implementation}] = std::move(f);
    };
    auto dielectric = [this](const char* id, const char* implementation, DielectricFactory f) {
        dielectric_[{id, implementation}] = std::move(f);
    };
    SolventFactory hgkReaktoro = [](A) { return std::make_shared<const WaterHGK84Reaktoro>(); };
    SolventFactory iapws = [](A) { return std::make_shared<const WaterIAPWS95Reaktoro>(); };
    solvent("WaterHGK84", "", hgkReaktoro);
    solvent("WaterHGK84", "reaktoro", hgkReaktoro);
    solvent("WaterHGK84", "gems", [](A) { return std::make_shared<const WaterHGK84Gems>(); });
    solvent("WaterIAPWS95", "", iapws);
    solvent("WaterIAPWS95", "reaktoro", iapws);
    solvent("WaterZhangDuan05", "", [](A a) {
        DewGibbsCoefficients coefficients;
        for (int i = 0; i < 5; ++i)
            if (const auto p = a.parameters.find("g1kb_" + std::to_string(i)); p != a.parameters.end())
                coefficients.g1kb[i] = p->second.value;
        const auto dew = a.parameters.find("dew_gibbs");
        return std::make_shared<const WaterZhangDuan05>(dew != a.parameters.end() && dew->second.value != 0.0, coefficients);
    });
    DielectricFactory jnReaktoro = [] { return std::make_shared<const JohnsonNorton91Reaktoro>(); };
    dielectric("JohnsonNorton91", "", jnReaktoro);
    dielectric("JohnsonNorton91", "reaktoro", jnReaktoro);
    dielectric("JohnsonNorton91", "gems", [] { return std::make_shared<const JohnsonNorton91Gems>(); });
    dielectric("SverjenskyEtAl14", "", [] { return std::make_shared<const SverjenskyEtAl14>(); });
    dielectric("FernandezEtAl97", "", [] { return std::make_shared<const FernandezEtAl97>(); });
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

auto MethodRegistry::hasSolventMethod(const std::string& id) const -> bool
{
    return solvent_.count({id, ""}) > 0 || dielectric_.count({id, ""}) > 0;
}

namespace {

template <class Factories, class... Inputs>
auto makeFrom(const Factories& factories, const MethodApplication& application, const Inputs&... inputs)
{
    auto it = factories.find({application.method, application.implementation});
    if (it == factories.end())
        it = factories.find({application.method, ""});
    if (it == factories.end())
        funError("Cannot make the method " + application.method, "There is no code for this method and implementation \"" + application.implementation + "\".", __LINE__, __FILE__);
    return it->second(inputs...);
}

} // namespace

auto MethodRegistry::createSolvent(const MethodApplication& application) const -> std::shared_ptr<const SolventMethod>
{
    return makeFrom(solvent_, application, application);
}

auto MethodRegistry::createDielectric(const MethodApplication& application) const -> std::shared_ptr<const DielectricMethod>
{
    return makeFrom(dielectric_, application);
}

} // namespace ThermoFun::v2
