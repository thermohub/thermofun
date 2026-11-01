// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Tests of the method objects, with no database and no engine: values, derivatives against finite differences,
// parameter derivatives, withParameter and the declared effects. Then each method against the old model on every
// bundled record that uses it.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <initializer_list>
#include <map>
#include <string>

#include <nlohmann/json.hpp>

#include "Engine/Executor.h"
#include "Conventions/HydrogenIon.h"
#include "Engine/Solvent.h"
#include "IO/Legacy/LegacyReader.h"
#include "Methods/Evaluate.h"
#include "Methods/MethodRegistry.h"
#include "Methods/Minerals/CpPolynomial.h"
#include "Methods/Minerals/ReferenceMethods.h"
#include "Methods/MethodCatalog.h"
#include "Methods/Water/WaterMethods.h"
#include "Substance.h"
#include "check.h"

using namespace ThermoFun::v2;
using nlohmann::json;

namespace {

bool near(double a, double b, double rel, double abs = 1e-9)
{
    return std::fabs(a - b) <= rel * std::fmax(std::fabs(a), std::fabs(b)) + abs;
}

auto reference() -> ReferenceValues
{
    ReferenceValues r;
    r.T = 298.15;
    r.P = 1e5;
    r.G = -1129178.0;
    r.H = -1207600.0;
    r.S = 91.7;
    r.Cp = 83.47;
    r.V = 3.6934e-5;
    return r;
}

// Two segments of Cp with a transition: all terms used
auto polynomial() -> std::shared_ptr<const SubstanceMethod>
{
    return cpPolynomialMethod({{273.15, 800.0}, {800.0, 2000.0}},
                        {99.7, 0.027, -2148800.0, 0.0, 1e-6, 2e-9, 1e-12, 1e6, 10.0, 0.5,
                         120.0, 0.01, -1e6, 50.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1},
                        {{800.0, 2.5, 2000.0, 1e-6}});
}

using Get = PropertyValue MethodResult::*;
const Get properties[] = {&MethodResult::G, &MethodResult::H, &MethodResult::S, &MethodResult::Cp, &MethodResult::V};

void checkDerivatives(const SubstanceMethod& m)
{
    if (!Reaktoro_::kWithAutodiff)  // built without autodiff, the derivatives are 0
        return;
    const auto r = reference();
    for (double T : {350.0, 900.0})
    {
        const double P = 2e7;
        const auto x = evaluate(m, T, P, r);
        const double h = 1e-3, hp = 1.0;
        const auto tu = evaluate(m, T + h, P, r), td = evaluate(m, T - h, P, r);
        const auto pu = evaluate(m, T, P + hp, r), pd = evaluate(m, T, P - hp, r);
        for (Get g : properties)
        {
            CHECK(near((x.*g).ddt, ((tu.*g).val - (td.*g).val) / (2 * h), 1e-6, 1e-5));
            CHECK(near((x.*g).ddp, ((pu.*g).val - (pd.*g).val) / (2 * hp), 1e-6, 1e-5));
        }
        // every parameter: the derivative against finite differences, and withParameter changes only that parameter
        const auto values = m.parameterValues();
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            const auto up = m.withParameter(i, values[i] + 1.0);
            auto changed = up->parameterValues();
            changed[i] = values[i];
            CHECK(changed == values);  // only parameter i changes
            CHECK(m.parameterValues() == values);  // and the original does not
            const auto d = evaluateParameterDerivative(m, i, T, P, r);
            for (Get g : properties)
            {
                // the step makes a change of the property well above its round-off
                const double dp = std::fmax(std::fabs(values[i]) * 1e-6, 1.0 / std::fmax(std::fabs((d.*g).ddt), 1e-3));
                const auto a = evaluate(*m.withParameter(i, values[i] + dp), T, P, r);
                const auto b = evaluate(*m.withParameter(i, values[i] - dp), T, P, r);
                CHECK(near((d.*g).ddt, ((a.*g).val - (b.*g).val) / (2 * dp), 1e-5, 1e-5));
            }
        }
    }
}

void testConstantProperties()
{
    const auto method = constantPropertiesMethod();
    const auto& m = *method;
    const auto r = evaluate(m, 500.0, 1e7, reference());
    CHECK(r.G.val == -1129178.0 && r.H.val == -1207600.0 && r.S.val == 91.7 && r.Cp.val == 83.47 && r.V.val == 3.6934e-5);
    CHECK(r.G.ddt == 0.0 && r.G.ddp == 0.0);
    CHECK(m.spec().id == "ConstantProperties" && m.spec().effects.base);
    ReferenceValues empty;
    CHECK(evaluate(m, 500.0, 1e7, empty).G.val == 0.0);
    checkDerivatives(m);
}

void testConstantCp()
{
    const auto method = constantCpMethod();
    const auto& m = *method;
    const auto r = evaluate(m, 500.0, 1e5, reference());
    const double Tr = 298.15, T = 500.0, S0 = 91.7, Cp = 83.47;
    CHECK(near(r.S.val, S0 + Cp * std::log(T / Tr), 1e-14));
    CHECK(near(r.H.val, -1207600.0 + Cp * (T - Tr), 1e-14));
    CHECK(near(r.G.val, -1129178.0 - S0 * (T - Tr) - Cp * (T * std::log(T / Tr) - (T - Tr)), 1e-14));
    CHECK(r.Cp.val == Cp);
    // without S or Cp, the terms are left out
    ReferenceValues noS = reference();
    noS.S.reset();
    noS.Cp.reset();
    const auto n = evaluate(m, 500.0, 1e5, noS);
    CHECK(n.G.val == -1129178.0 && n.H.val == -1207600.0 && n.S.val == 0.0);
    checkDerivatives(m);
}

void testCpPolynomial()
{
    const auto method = polynomial();
    const auto& m = *method;
    CHECK(m.parameterValues().size() == 23);  // 2 segments of 10, and dS, dH, dV of the transition
    checkDerivatives(m);

    // the reference state gives the reference values, and Cp is dH/dT and T dS/dT
    const auto r = reference();
    const auto at = evaluate(m, 298.15, 1e5, r);
    CHECK(near(at.G.val, *r.G, 1e-12) && near(at.H.val, *r.H, 1e-12) && near(at.S.val, *r.S, 1e-12));
    for (double T : {350.0, 900.0})
    {
        if (!Reaktoro_::kWithAutodiff)
            break;
        const auto x = evaluate(m, T, 1e5, r);
        CHECK(near(x.H.ddt, x.Cp.val, 1e-10));
        CHECK(near(T * x.S.ddt, x.Cp.val, 1e-10));
        CHECK(near(x.G.ddt, -x.S.val, 1e-10));
    }
    // the transition is in the state above it
    CHECK(evaluate(m, 500.0, 1e5, r).V.val == 0.0);
    CHECK(near(evaluate(m, 900.0, 1e5, r).V.val, 1e-6, 1e-12));
    // outside the intervals: the nearest one, and it says so
    CHECK(evaluate(m, 3000.0, 1e5, r).outside_range);
    CHECK(!evaluate(m, 500.0, 1e5, r).outside_range);
    // without coefficients: Cp is the constant reference value, without a coefficient to fit
    const auto noneMethod = cpPolynomialMethod(std::vector<std::pair<double, double>>{}, std::vector<double>{});
    const auto& none = *noneMethod;
    const auto c = evaluate(none, 400.0, 1e5, r);
    CHECK(c.Cp.val == 83.47);
    CHECK(near(c.H.val, -1207600.0 + 83.47 * (400.0 - 298.15), 1e-14));
    // the empty marker of GEMS counts as 0
    ReferenceValues marker = reference();
    marker.Cp = 1.1754943508222875e-38;
    CHECK(evaluate(none, 400.0, 1e5, marker).Cp.val == 0.0);
}

// The effects that a specification declares are what the method does
void testVolumeMethods()
{
    const auto r = reference();
    // constant volume: V dP on G and H
    SubstanceState<double> s;
    s.G = -1e6; s.H = -1.1e6; s.S = 100.0;
    ReferenceState<double> ref = r;
    SubstanceInputs<double> in{500.0, 1e7, ref};
    constantVolumeMethod()->apply(s, in);
    const double dV = 3.6934e-5 * (1e7 - 1e5);
    CHECK(s.V == 3.6934e-5 && s.G == -1e6 + dV && s.H == -1.1e6 + dV && s.S == 100.0);
    // no reference volume: it does nothing
    ref.V.reset();
    SubstanceState<double> t;
    t.G = 5.0;
    constantVolumeMethod()->apply(t, in);
    CHECK(t.G == 5.0 && t.V == 0.0);
    // ideal gas: V = RT/P; without the correction no change of G and H
    in.options.apply_pressure_correction = false;
    SubstanceState<double> g;
    g.G = 5.0;
    idealGasMethod()->apply(g, in);
    CHECK(near(g.V, 8.31451 * 500.0 / 1e7, 1e-14) && g.G == 5.0);
    // with it G(T, P) has RT ln(P/Pr), S has -R ln(P/Pr), and H does not change
    in.options.apply_pressure_correction = true;
    SubstanceState<double> c;
    c.G = 5.0;
    c.H = 7.0;
    idealGasMethod()->apply(c, in);
    CHECK(near(c.G, 5.0 + 8.31451 * 500.0 * std::log(1e7 / 1e5), 1e-12));
    CHECK(near(c.S, -8.31451 * std::log(1e7 / 1e5), 1e-12));
    CHECK(c.H == 7.0);
}

void testEffects()
{
    const auto r = reference();
    SubstanceState<double> state;
    state.G = 1.0; state.H = 2.0; state.S = 3.0; state.Cp = 4.0; state.V = 5.0;
    ReferenceState<double> ref = r;
    SubstanceInputs<double> in{500.0, 1e7, ref};
    const auto cp = polynomial();
    auto check = [&](const SubstanceMethod& m) {
        CHECK(m.spec().effects.base);
        SubstanceState<double> a = state, b;
        m.apply(a, in);
        m.apply(b, in);
        // a base method sets everything: the incoming state does not matter
        CHECK(a.G == b.G && a.H == b.H && a.S == b.S && a.Cp == b.Cp && a.V == b.V);
    };
    check(*constantPropertiesMethod());
    check(*constantCpMethod());
    check(*cp);
}

void testRegistry()
{
    auto& registry = MethodRegistry::instance();
    CHECK(registry.hasSubstanceMethod("CpPolynomial") && !registry.hasSubstanceMethod("NoSuchMethod"));
    MethodApplication a;
    a.method = "ConstantCp";
    CHECK(registry.create(a)->spec().id == "ConstantCp");
    a.method = "NoSuchMethod";
    bool raised = false;
    try { registry.create(a); } catch (const std::exception&) { raised = true; }
    CHECK(raised);
}

auto referenceOf(const SubstanceRecord& s) -> ReferenceValues
{
    ReferenceValues r;
    r.T = s.reference_conditions.T;
    r.P = s.reference_conditions.P;
    auto get = [&](PropertyName p) -> std::optional<double> {
        const auto it = s.standard_state_properties.find(p);
        if (it == s.standard_state_properties.end())
            return std::nullopt;
        return it->second.value;
    };
    r.G = get(PropertyName::G);
    r.H = get(PropertyName::H);
    r.S = get(PropertyName::S);
    r.Cp = get(PropertyName::Cp);
    r.V = get(PropertyName::V);
    return r;
}

// Stand-ins for corrections: each one adds its id to G as a digit, so the value of G shows the order
template <int Digit>
class Stand final : public SubstanceMethod
{
public:
    explicit Stand(const char* id) : spec_(*MethodCatalog::instance().find(id)) {}
    auto spec() const -> const MethodSpec& override { return spec_; }
    auto apply(SubstanceState<double>& s, const SubstanceInputs<double>&) const -> void override { s.G = s.G * 10 + Digit; }
    auto apply(SubstanceState<TPReal>& s, const SubstanceInputs<TPReal>&) const -> void override { s.G = s.G * 10 + Digit; }
    auto parameterValues() const -> std::vector<double> override { return {}; }
    auto withParameter(std::size_t, double) const -> std::shared_ptr<const SubstanceMethod> override { return nullptr; }
private:
    MethodSpec spec_;
};

void testExecutionOrder()
{
    auto& registry = MethodRegistry::instance();
    registry.add("VolumeBerman88", "", [](const MethodApplication&, const SubstanceRecord&) { return std::make_shared<const Stand<3>>("VolumeBerman88"); });
    registry.add("LandauHP98", "", [](const MethodApplication&, const SubstanceRecord&) { return std::make_shared<const Stand<2>>("LandauHP98"); });
    registry.add("MurnaghanHP98", "", [](const MethodApplication&, const SubstanceRecord&) { return std::make_shared<const Stand<4>>("MurnaghanHP98"); });

    std::vector<std::string> ids = {"MurnaghanHP98", "VolumeBerman88", "LandauHP98", "ConstantProperties"};
    std::sort(ids.begin(), ids.end());
    do  // every order in the file gives one order of execution
    {
        auto record = std::make_shared<SubstanceRecord>();
        record->symbol = "X";
        for (const auto& id : ids)
        {
            MethodApplication a;
            a.method = id;
            record->methods.push_back(a);
        }
        CHECK(canBind(*record));
        const auto bound = bindSubstance(record);
        std::vector<ExplainStep> steps;
        const auto result = executeSubstance(*bound, 400.0, 1e5, {}, &steps);
        CHECK(steps.size() == 4);
        // base; the correction that sets with the lowest rank; the next one; the one that only adds
        CHECK(steps[0].method == "ConstantProperties" && steps[1].method == "LandauHP98");
        CHECK(steps[2].method == "VolumeBerman88" && steps[3].method == "MurnaghanHP98");
        CHECK(result.values.G.val == 234.0);
    } while (std::next_permutation(ids.begin(), ids.end()));

    for (const char* id : {"VolumeBerman88", "LandauHP98", "MurnaghanHP98"})
        registry.remove(id, "");

    // a record that the new path cannot run: a method without code, no base method, two base methods
    SubstanceRecord r;
    MethodApplication a;
    a.method = "TaitHP11";
    r.methods = {a};
    CHECK(!canBind(r));
    r.methods.clear();
    CHECK(!canBind(r));
    a.method = "ConstantCp";
    r.methods = {a, a};
    CHECK(!canBind(r));
}

void testConflictMessages()
{
    auto check = [](std::vector<std::string> ids, SubstanceClass cls, bool withVolume) {
        SubstanceRecord r;
        r.symbol = "Quartz";
        r.class_ = cls;
        if (withVolume)
            r.standard_state_properties[PropertyName::V] = Quantity{};
        for (const auto& id : ids)
        {
            MethodApplication a;
            a.method = id;
            r.methods.push_back(a);
        }
        return checkMethods(r).all();
    };
    auto says = [](const ThermoFun::Diagnostic& d, const char* code, const std::string& message) {
        return d.code == code && d.message == message && !d.fix.empty() && d.record == "Quartz";
    };
    const auto C = SubstanceClass::Component;

    auto found = check({"CpPolynomial"}, C, true);
    CHECK(found.empty());

    found = check({}, C, false);
    CHECK(found.size() == 1 && says(found[0], "V2-MTH-012", "Quartz has no base method and no reference values."));
    found = check({}, C, true);
    CHECK(found.empty());

    found = check({"ConstantCp", "CpPolynomial"}, C, true);
    CHECK(found.size() == 1 && says(found[0], "V2-MTH-012", "Quartz has two base methods: ConstantCp and CpPolynomial."));

    found = check({"CpPolynomial", "LandauHP98", "ConstantVolume"}, C, true);
    CHECK(found.size() == 1 && says(found[0], "V2-MTH-013", "LandauHP98 and ConstantVolume both set the volume of Quartz."));

    found = check({"CpPolynomial", "MurnaghanHP98"}, C, false);
    CHECK(found.size() == 1 && says(found[0], "V2-MTH-014", "MurnaghanHP98 adds to a volume that is not defined."));
    CHECK(found[0].severity == ThermoFun::Severity::Warning);
    found = check({"CpPolynomial", "MurnaghanHP98"}, C, true);
    CHECK(found.empty());

    found = check({"CpPolynomial", "HKF88"}, C, true);
    bool classMessage = false;
    for (const auto& d : found)
        classMessage = classMessage || says(d, "V2-MTH-005", "HKF88 is for aqueous solutes; Quartz is a component.");
    CHECK(classMessage);
}

void testSolvent()
{
    auto waterRecord = [](std::vector<std::pair<std::string, std::string>> methods) {
        auto r = std::make_shared<SubstanceRecord>();
        r->symbol = "H2O@";
        r->class_ = SubstanceClass::WaterSolvent;
        for (const auto& [id, implementation] : methods)
        {
            MethodApplication a;
            a.method = id;
            a.implementation = implementation;
            r->methods.push_back(a);
        }
        return r;
    };

    // the methods of a water record: one equation of state, and the dielectric model
    auto r = waterRecord({{"JohnsonNorton91", "gems"}, {"WaterHGK84", "gems"}});
    auto bound = bindSolvent(r);
    CHECK(bound->equation_of_state->implementation() == "gems");
    CHECK(bound->dielectric->implementation() == "gems");
    CHECK(bound->equation_of_state->gibbsEnergyContainsRTlnP());

    // the default implementation is the first one of the catalog
    r = waterRecord({{"WaterHGK84", ""}});
    bound = bindSolvent(r);
    CHECK(bound->equation_of_state->implementation() == "reaktoro" && !bound->dielectric);

    // only a dielectric model: no properties of water
    CHECK(!bindSolvent(waterRecord({{"JohnsonNorton91", ""}}))->equation_of_state);
    // water by reference values: the base method runs alone, and the dielectric model stays
    bound = bindSolvent(waterRecord({{"JohnsonNorton91", "gems"}, {"ConstantCp", ""}, {"ConstantVolume", ""}}));
    CHECK(!bound->equation_of_state && bound->reference_values && bound->dielectric);
    CHECK(bound->reference_values->record->methods.size() == 1);
    // an equation of state goes first, and a method that does not act on water is left out
    bound = bindSolvent(waterRecord({{"WaterHGK84", ""}, {"HKF88", ""}}));
    CHECK(bound->equation_of_state && !bound->reference_values);

    // a pressure of 0 is the saturation pressure, and it is written back (IAPWS-95: 1554928 Pa; the code of GEMS has
    // its own saturation equation, 0.08 % lower)
    for (const auto& method : {waterHGK84GemsMethod(), MethodRegistry::instance().createSolvent(MethodApplication{"WaterHGK84", "", "reaktoro"}), MethodRegistry::instance().createSolvent(MethodApplication{"WaterIAPWS95", "", "reaktoro"})})
    {
        double P = 0.0;
        method->properties(473.15, P, 0);
        CHECK(std::abs(P - 1554928.0) < 3000.0);
        P = 0.0;
        method->substanceProperties(473.15, P, 0, "NEA_HGK");
        CHECK(std::abs(P - 1554928.0) < 3000.0);
    }

    // the state of a record
    SubstanceRecord state;
    state.aggregate_state = AggregateState::Aqueous;
    CHECK(solventStateOf(state) == 0);
    state.aggregate_state = AggregateState::Gas;
    CHECK(solventStateOf(state) == 1);

    // the rules of the choice of the solvent, in their order
    const std::map<std::string, SubstanceClass> records = {{"H2O@", SubstanceClass::WaterSolvent}, {"H2O@2", SubstanceClass::WaterSolvent},
                                                           {"Na+", SubstanceClass::AqueousSolute}, {"Z", SubstanceClass::Other}};
    CHECK(chooseSolvent("H2O@", "H2O@2", records) == "H2O@");
    CHECK(chooseSolvent("none", "H2O@", records) == "H2O@");
    CHECK(chooseSolvent("none", "none", records) == "H2O@2");
    CHECK(chooseSolvent("none", "", {{"Na+", SubstanceClass::AqueousSolute}}).empty());
}

void testHydrogenIon()
{
    SubstanceRecord r;
    r.symbol = "anything";
    r.class_ = SubstanceClass::AqueousSolute;
    r.formula = "H+";
    CHECK(isHydrogenIon(r));  // a record may have no charge
    r.charge = 1;
    CHECK(isHydrogenIon(r));
    r.charge = 2;
    CHECK(!isHydrogenIon(r));
    r.charge = 1;
    r.formula = "Na+";
    CHECK(!isHydrogenIon(r));
    r.formula = "H+";
    r.class_ = SubstanceClass::GasFluid;
    CHECK(!isHydrogenIon(r));
}


} // namespace

void runSubstanceMethods()
{
    testConstantProperties();
    testConstantCp();
    testCpPolynomial();
    testVolumeMethods();
    testEffects();
    testRegistry();
    testExecutionOrder();
    testConflictMessages();
    testSolvent();
    testHydrogenIon();
}
