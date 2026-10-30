// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "MethodRegistry.h"

#include <initializer_list>
#include <type_traits>
#include <vector>

#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/Minerals/ConstantCp.h>
#include <ThermoFun/Methods/Minerals/ConstantProperties.h>
#include <ThermoFun/Methods/Minerals/ConstantVolume.h>
#include <ThermoFun/Methods/Minerals/CpPolynomial.h>
#include <ThermoFun/Methods/Minerals/IdealGas.h>
#include <ThermoFun/Methods/Minerals/MineralMethods.h>
#include <ThermoFun/Conventions/HydrogenIon.h>
#include <ThermoFun/Methods/Solutes/SoluteMethods.h>
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

    // a correction of a mineral: the coefficients of the names; none if the record gives none of them. "": an unused place
    auto mineral = [this](auto kind, const char* id, std::initializer_list<const char*> names) {
        using Method = typename decltype(kind)::type;
        add(id, "", [names = std::vector<const char*>(names)](A a, R r) -> std::shared_ptr<const SubstanceMethod> {
            std::vector<double> values;
            bool any = false;
            for (const char* n : names)
            {
                const auto it = a.parameters.find(n);
                any = any || it != a.parameters.end();
                values.push_back(it == a.parameters.end() ? 0.0 : it->second.value);
            }
            auto value = [&](const char* n) { const auto it = a.parameters.find(n); return it == a.parameters.end() ? 0.0 : it->second.value; };
            return std::make_shared<const Method>(any ? values : std::vector<double>{}, value("alpha0"), value("kappa0"), r.applicability);
        });
    };
    mineral(std::type_identity<LandauHP98>{}, "LandauHP98", {"Tc0", "Smax", "Vmax"});
    mineral(std::type_identity<LandauHP11>{}, "LandauHP11", {"Tc0", "Smax", "Vmax"});
    mineral(std::type_identity<MurnaghanHP98>{}, "MurnaghanHP98", {});
    mineral(std::type_identity<TaitHP11>{}, "TaitHP11", {"kappa0p", "kappa0pp", "n_atoms"});
    mineral(std::type_identity<VolumeBerman88>{}, "VolumeBerman88", {"a1", "a2", "a3", "b1", "b2"});
    mineral(std::type_identity<VolumeDorogokupets88>{}, "VolumeDorogokupets88", {"a1", "a2", "a3", "b1", "b2"});
    mineral(std::type_identity<BirchMurnaghan47>{}, "BirchMurnaghan47", {"a1", "a2", "a3", "", "", "kt00", "dkdt", "kp", "kpp"});
    mineral(std::type_identity<CpExtendedUNIQUAC>{}, "CpExtendedUNIQUAC", {"a", "b", "c", "alpha", "beta", "Theta"});
    add("NASA9", "", [](A a, R) { return std::make_shared<const NASA9>(a); });
    add("TPTable", "", [](A a, R) { return std::make_shared<const TPTable>(a); });

    // the parameters in the order of the catalog; none if the record gives none
    auto parametersOf = [](A a, std::initializer_list<const char*> names) {
        std::vector<double> values;
        if (a.parameters.empty())
            return values;
        for (const char* n : names)
        {
            const auto it = a.parameters.find(n);
            values.push_back(it == a.parameters.end() ? 0.0 : it->second.value);
        }
        return values;
    };
    auto hkfSpecies = [](R r) { return HKFSpecies{r.symbol, static_cast<double>(r.charge.value_or(0)), isHydrogenIon(r), r.applicability}; };
    auto hkfReaktoro = [parametersOf, hkfSpecies](A a, R r) -> std::shared_ptr<const SubstanceMethod> {
        return std::make_shared<const HKF88Reaktoro>(parametersOf(a, {"a1", "a2", "a3", "a4", "c1", "c2", "omega"}), hkfSpecies(r));
    };
    add("HKF88", "", hkfReaktoro);
    add("HKF88", "reaktoro", hkfReaktoro);
    add("HKF88", "gems", [parametersOf, hkfSpecies](A a, R r) -> std::shared_ptr<const SubstanceMethod> {
        return std::make_shared<const HKF88Gems>(parametersOf(a, {"a1", "a2", "a3", "a4", "c1", "c2", "omega"}), hkfSpecies(r));
    });
    add("SoluteHP98", "", [parametersOf](A a, R r) { return std::make_shared<const SoluteHP98>(parametersOf(a, {"b"}), r.symbol); });
    add("AndersonEtAl91", "", [](A, R) { return std::make_shared<const AndersonEtAl91>(); });
    add("AkinfievDiamond03", "", [parametersOf](A a, R r) { return std::make_shared<const AkinfievDiamond03>(parametersOf(a, {"xi", "a", "b"}), r.symbol); });

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
