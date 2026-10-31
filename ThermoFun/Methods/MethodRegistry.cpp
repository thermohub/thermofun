// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "MethodRegistry.h"

#include <algorithm>

#include <cmath>
#include <initializer_list>
#include <type_traits>
#include <vector>

#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/Gases/GasMethods.h>
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

    // the reactions: the coefficients of the names in the units of the formulas; none if the record gives none of them
    auto coefficients = [](A a, std::initializer_list<std::pair<const char*, double>> names) {
        std::vector<real> values;
        bool any = false;
        for (const auto& [n, factor] : names)
        {
            const auto it = a.parameters.find(n);
            any = any || it != a.parameters.end();
            values.push_back(it == a.parameters.end() ? 0.0 : it->second.value * factor);
        }
        return any ? values : std::vector<real>{};
    };
    using RR = const ReactionRecord&;
    auto reactionInputs = [coefficients](A a, RR r, std::initializer_list<std::pair<const char*, double>> names) {
        ReactionInputs in;
        in.Tr = r.reference_conditions.T;
        in.Pr = r.reference_conditions.P;
        auto load = [&](ReactionPropertyName name, Reaktoro_::ThermoProperty& to, double factor = 1.0) {
            const auto it = r.reaction_properties.find(name);
            if (it == r.reaction_properties.end())
                return;
            to.val = it->second.value * factor;
            to.sta = {Reaktoro_::Status::read, ""};
            if (it->second.uncertainty)
                to.err = it->second.uncertainty->standard * factor;
        };
        auto& ref = in.reference;
        load(ReactionPropertyName::logK, ref.log_equilibrium_constant);
        load(ReactionPropertyName::dG, ref.reaction_gibbs_energy);
        load(ReactionPropertyName::dH, ref.reaction_enthalpy);
        load(ReactionPropertyName::dS, ref.reaction_entropy);
        load(ReactionPropertyName::dCp, ref.reaction_heat_capacity_cp);
        load(ReactionPropertyName::dV, ref.reaction_volume, 1e5);  // J/bar
        in.coefficients = coefficients(a, names);
        return in;
    };
    auto logKMethod = [this, reactionInputs, coefficients](const char* id, LogKForm form) {
        reaction_[id] = [=](A a, RR r) {
            auto in = reactionInputs(a, r, {{"a0", 1}, {"a1", 1}, {"a2", 1}, {"a3", 1}, {"a4", 1}, {"a5", 1}, {"a6", 1}});
            in.cp_coefficients = coefficients(a, {{"a", 1}, {"b", 1}, {"c", 1}, {"d", 1}, {"e", 1}});
            return makeLogK(id, form, std::move(in));
        };
    };
    logKMethod("LogKPolynomial", LogKForm::Polynomial);
    logKMethod("ReactionCpPolynomial", LogKForm::Polynomial);
    logKMethod("LogKNordstromMunoz88", LogKForm::Polynomial);
    logKMethod("LogKConstant", LogKForm::Constant);
    logKMethod("LogKZeroEntropy", LogKForm::ZeroEntropy);
    logKMethod("LogKVantHoff", LogKForm::VantHoff);
    logKMethod("LogKConstantCp", LogKForm::ConstantCp);
    reaction_["ConstantReactionProperties"] = [reactionInputs](A a, RR r) { return makeConstantReactionProperties(reactionInputs(a, r, {})); };
    reaction_["LogKTable"] = [reactionInputs](A a, RR r) {
        std::vector<double> x, y;
        if (a.table)
        {
            x = a.table->temperatures;
            if (const auto it = a.table->values.find("logK"); it != a.table->values.end())
                y = it->second;
        }
        const int order = a.interpolation == Interpolation::Lagrange ? a.interpolation_order : 0;
        return makeLogKTable(std::move(x), std::move(y), order, reactionInputs(a, r, {}));
    };
    reaction_["DolejsManning10"] = [reactionInputs](A a, RR r) { return makeDolejsManning10(reactionInputs(a, r, {{"a", 1e-3}, {"b", 1}, {"c", 1}, {"d", 1}, {"e", 1}})); };
    reaction_["MarshallFranck81"] = [reactionInputs](A a, RR r) { return makeMarshallFranck81(reactionInputs(a, r, {{"a", 1}, {"b", 1}, {"c", 1}, {"d", 1}, {"e", 1}, {"f", 1}, {"g", 1}})); };
    reaction_["RyzhenkoBryzgalin"] = [reactionInputs](A a, RR r) { return makeRyzhenkoBryzgalin(reactionInputs(a, r, {{"pK298", 1}, {"A", 1}, {"B", 1}})); };
    reaction_["ReactionVolumePolynomial"] = [reactionInputs](A a, RR r) { return makeReactionVolumePolynomial(reactionInputs(a, r, {{"a1", 1}, {"a2", 1}, {"a3", 1}, {"b1", 1e5}, {"b2", 1e10}})); };
    reaction_["ReactionConstantVolume"] = [reactionInputs](A a, RR r) { return makeReactionConstantVolume(reactionInputs(a, r, {})); };

    // a fluid: the parameters in the order of the names; the lower T of the record is that of its heat capacity
    auto gas = [this](auto kind, const char* id, std::vector<const char*> names) {
        using Method = typename decltype(kind)::type;
        add(id, "", [names, id = std::string(id)](A a, R r) -> std::shared_ptr<const SubstanceMethod> {
            std::vector<double> values;
            for (const char* n : names)
            {
                const auto it = a.parameters.find(n);
                values.push_back(it == a.parameters.end() ? 0.0 : it->second.value);
            }
            GasRecord record{r.reference_conditions.T, r.formula, r.applicability};
            bool found = false;
            for (const auto& other : r.methods)
                if (other.method == "CpPolynomial" && !found)
                    for (const auto& segment : other.segments)
                        if (std::isfinite(segment.t_min) && !found)
                        {
                            record.lower_T = segment.t_min;
                            found = true;
                        }
            if (!found)
                thfun_logger->warn("{}: substance \"{}\" has no temperature intervals (they come only from a CpPolynomial method); using its reference temperature {} K as the lower temperature bound.", id, r.symbol, record.lower_T);
            return std::make_shared<const Method>(values, record);
        });
    };
    const std::vector<const char*> cubic = {"Tc", "Pc", "omega", "N"}, interaction = {"Tc", "Pc", "omega", "k1", "k2", "k3"};
    gas(std::type_identity<PRSV86>{}, "PRSV86", interaction);
    gas(std::type_identity<CorkHP91>{}, "CorkHP91", interaction);
    gas(std::type_identity<SoaveRedlichKwong72>{}, "SoaveRedlichKwong72", cubic);
    gas(std::type_identity<PengRobinson78>{}, "PengRobinson78", cubic);
    gas(std::type_identity<SternerPitzer94>{}, "SternerPitzer94", {"Tc", "Pc"});
    gas(std::type_identity<ChurakovGottschalk03>{}, "ChurakovGottschalk03", {"sigma0", "eps0", "mu0", "pol0", "sigma1", "sigma2", "eps1", "eps2", "mu1", "mu2", "pol1", "pol2"});

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

auto MethodRegistry::implementations(const std::string& id) const -> std::vector<std::string>
{
    std::vector<std::string> out;
    auto addFrom = [&](const auto& factories) {
        for (const auto& [key, factory] : factories)
            if (key.first == id && std::find(out.begin(), out.end(), key.second) == out.end())
                out.push_back(key.second);
    };
    addFrom(substance_);
    addFrom(solvent_);
    addFrom(dielectric_);
    return out;
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

auto MethodRegistry::hasReactionMethod(const std::string& id) const -> bool
{
    return reaction_.count(id) > 0;
}

auto MethodRegistry::createReaction(const MethodApplication& application, const ReactionRecord& record) const -> std::shared_ptr<const ReactionMethod>
{
    const auto it = reaction_.find(application.method);
    if (it == reaction_.end())
        funError("Cannot make the method " + application.method, "There is no code for this method of a reaction.", __LINE__, __FILE__);
    return it->second(application, record);
}

auto MethodRegistry::createSolvent(const MethodApplication& application) const -> std::shared_ptr<const SolventMethod>
{
    return makeFrom(solvent_, application, application);
}

auto MethodRegistry::createDielectric(const MethodApplication& application) const -> std::shared_ptr<const DielectricMethod>
{
    return makeFrom(dielectric_, application);
}

} // namespace ThermoFun::v2
