// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "WaterMethods.h"

#include <functional>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/Methods/Water/Reaktoro/WaterElectroState.hpp>
#include <ThermoFun/Methods/Water/Reaktoro/WaterElectroStateJohnsonNorton.hpp>
#include <ThermoFun/Methods/Water/Reaktoro/WaterThermoState.hpp>
#include <ThermoFun/Methods/Water/Reaktoro/WaterThermoStateUtils.hpp>
#include <ThermoFun/Methods/Water/Reaktoro/WaterUtils.hpp>
#include <ThermoFun/Methods/Water/WaterElectroFernandez1997.h>
#include <ThermoFun/Methods/Water/WaterElectroSverjensky2014.h>
#include <ThermoFun/Methods/Water/WaterHGK-JNgems.h>
#include <ThermoFun/Model/BuiltinParameterSets.h>
#include <ThermoFun/Methods/Water/WaterHGKreaktoro.h>
#include <ThermoFun/Methods/Water/WaterJN91reaktoro.h>
#include <ThermoFun/Methods/Water/WaterWP95reaktoro.h>
#include <ThermoFun/Methods/Water/WaterZhangDuan2005.h>

namespace ThermoFun::v2 {

namespace {

auto specOf(const char* id) -> const MethodSpec&
{
    return *MethodCatalog::instance().find(id, OperatesOn::Substance);
}

auto checkModelValidity(double T, double P, double Tmax, double /*Tmin*/, double Pmax, double Pmin, std::string model) -> void
{
    // Check if given temperature is within the allowed range
    if(T < 0 /*Tmin*/ || T > Tmax)
    {
        Exception exception;
        exception.error << "Out of T bound in model " << model;
        exception.reason << "The provided temperature, " << T << " K,"  << "is either negative "
              "or greater than the maximum allowed, " << Tmax << " K.";
        RaiseError(exception);
    }

    // Check if given pressure is within the allowed range
    if( P > Pmax)
    {
        Exception exception;
        exception.error << "Out of P bound in model " << model;
        exception.reason << "The provided pressure, " << P << " Pa,"  << "is greater than the maximum allowed, " << Pmax << " Pa.";
        RaiseError(exception);
    }

    // Check if given pressure is within the allowed range
    if( P < Pmin)
    {
        Exception exception;
        exception.error << "Out of P bound in model " << model;
        exception.reason << "The provided pressure, " << P << " Pa,"  << "is lower than the minimum allowed, " << Pmin << " Pa.";
        RaiseError(exception);
    }
}
// The limits of Zhang and Duan (2005): from 100 MPa and 273.15 K up to about 20-30 GPa and 2000 K.
auto checkZhangDuan(double T, double P) -> void
{
    checkModelValidity(T, P, 2000.0, 273.15, 3e10, 1e8, "Zhang and Duan (2005) H2O model.");
}

auto hgk84GemsSubstance(double T, double& P, int state, const std::string& triple, const WaterHGKgemsConstants& constants) -> ThermoPropertiesSubstance
{
    WaterTripleProperties wtr = waterTripleData().at(triple);

    double Pout = P;
    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        WaterHGKgems water_hgk(constants);
        real t = pass.T - C_to_K;
        real p = pass.P / bar_to_Pa;
        water_hgk.calculateWaterHGKgems(t, p, wtr);
        Pout = p.val() * bar_to_Pa;
        return water_hgk.thermoPropertiesWaterHGKgems(state);
    });
    P = Pout;

    return tps;
}

auto hgk84ReaktoroProperties(double T, double& P, int state, const HGKConstants& constants) -> PropertiesSolvent
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T), constants.saturation).val();
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return propertiesWaterHGKreaktoro(waterThermoStateHGK(pass.T, pass.P, state, constants));
    });
}

auto hgk84ReaktoroSubstance(double T, double& P, int state, const std::string& triple, const HGKConstants& constants) -> ThermoPropertiesSubstance
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T), constants.saturation).val();
    WaterTripleProperties wtr = waterTripleData().at(triple);
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesWaterHGKreaktoro(pass.T, waterThermoStateHGK(pass.T, pass.P, state, constants), wtr, constants.Ttr, constants.saturation.M);
    });
}

auto iapws95ReaktoroProperties(double T, double& P, int state, const WagnerPrussConstants& constants) -> PropertiesSolvent
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T), constants.saturation).val();
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return propertiesWaterWP95reaktoro(waterThermoStateWagnerPruss(pass.T, pass.P, state, constants));
    });
}

auto iapws95ReaktoroSubstance(double T, double& P, int state, const std::string& triple, const WagnerPrussConstants& constants) -> ThermoPropertiesSubstance
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T), constants.saturation).val();
    WaterTripleProperties wtr = waterTripleData().at(triple);
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesWaterWP95reaktoro(pass.T, waterThermoStateWagnerPruss(pass.T, pass.P, state, constants), wtr, constants.Ttr, constants.saturation.M);
    });
}

auto zhangDuan05Properties(double T, double& P, const ZhangDuanConstants& constants) -> PropertiesSolvent
{
    checkZhangDuan(T, P);
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return propertiesWaterZhangDuan2005(pass, constants);
    });
}

auto zhangDuan05Substance(double T, double& P, bool dew_gibbs, const ZhangDuanConstants& constants) -> ThermoPropertiesSubstance
{
    checkZhangDuan(T, P);
    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        auto out = thermoPropertiesWaterZhangDuan2005(pass.T, pass.P / bar_to_Pa, constants); // pressure in bar
        if (dew_gibbs)
            out.gibbs_energy = waterGibbsEnergyDEW(pass.T, pass.P / bar_to_Pa, constants);
        return out;
    });
    // the volume is calculated by this model, and the Gibbs energy of DEW if it is asked for
    const Reaktoro_::StatusMessage undefined = {Reaktoro_::Status::notdefined, ""};
    for (auto* x : {&tps.helmholtz_energy, &tps.internal_energy, &tps.enthalpy,
                    &tps.entropy, &tps.heat_capacity_cp, &tps.heat_capacity_cv})
        x->sta = undefined;
    if (dew_gibbs)
        tps.gibbs_energy.asCalculated();
    else
        tps.gibbs_energy.sta = undefined;
    return tps;
}

auto johnsonNorton91Reaktoro(double T, double P, const PropertiesSolvent& ps, int state, const std::vector<double>& constants) -> ElectroPropertiesSolvent
{
    auto eps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        WaterThermoState wts;
        wts.density   = pass(ps.density);
        wts.densityT  = pass(ps.densityT);
        wts.densityP  = pass(ps.densityP);
        wts.densityTT = pass(ps.densityTT);
        wts.densityTP = pass(ps.densityTP);
        wts.densityPP = pass(ps.densityPP);
        return electroPropertiesWaterJNreaktoro(waterElectroStateJohnsonNorton(pass.T, /*pass.P,*/ wts, constants, state));
    });
    // the calculation only keeps the values and derivatives of the density, so the errors and statuses of
    // the density inputs are propagated onto each result, according to the derivatives it is calculated from
    const auto& d = ps.density; const auto& dT = ps.densityT; const auto& dP = ps.densityP;
    const auto& dTT = ps.densityTT; const auto& dTP = ps.densityTP; const auto& dPP = ps.densityPP;
    eps.epsilon.propagateFrom(d);
    eps.bornZ.propagateFrom(d);
    eps.epsilonT.propagateFrom(d, dT);
    eps.bornY.propagateFrom(d, dT);
    eps.epsilonP.propagateFrom(d, dP);
    eps.bornQ.propagateFrom(d, dP);
    eps.epsilonTT.propagateFrom(d, dT, dTT);
    eps.bornX.propagateFrom(d, dT, dTT);
    eps.epsilonTP.propagateFrom(d, dT, dP, dTP);
    eps.bornU.propagateFrom(d, dT, dP, dTP);
    eps.epsilonPP.propagateFrom(d, dP, dPP);
    eps.bornN.propagateFrom(d, dP, dPP);
    return eps;
}

auto johnsonNorton91Gems(double T, double P, const PropertiesSolvent& ps, const WaterHGKgemsConstants& constants) -> ElectroPropertiesSolvent
{
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        WaterHGKgems water_hgk(constants);
        return water_hgk.electroPropertiesWaterJNgems(pass.T, pass.P / bar_to_Pa, pass(ps.density) / 1000.0, pass(ps.Beta) * bar_to_Pa, pass(ps.Alpha), pass(ps.dAldT));
    });
}

/// An equation of state of water, made of functions.
class FunctionSolventMethod final : public SolventMethod
{
public:
    using Properties = std::function<PropertiesSolvent(double, double&, int, const std::string&)>;
    using Substance = std::function<ThermoPropertiesSubstance(double, double&, int, const std::string&)>;

    FunctionSolventMethod(const char* id, const char* implementation, Properties properties, Substance substance, bool containsRTlnP)
    : spec_(specOf(id)), implementation_(implementation), properties_(std::move(properties)), substance_(std::move(substance)), contains_(containsRTlnP) {}

    auto spec() const -> const MethodSpec& override { return spec_; }
    auto implementation() const -> std::string override { return implementation_; }
    auto properties(double T, double& P, int state) const -> PropertiesSolvent override { return properties_(T, P, state, "NEA_HGK"); }
    auto substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance override { return substance_(T, P, state, triple); }
    auto gibbsEnergyContainsRTlnP() const -> bool override { return contains_; }

private:
    const MethodSpec& spec_;
    std::string implementation_;
    Properties properties_;
    Substance substance_;
    bool contains_;
};

/// A dielectric model of water, made of a function.
class FunctionDielectricMethod final : public DielectricMethod
{
public:
    using Function = std::function<ElectroPropertiesSolvent(double, double, const PropertiesSolvent&, int)>;

    FunctionDielectricMethod(const char* id, const char* implementation, Function function) : spec_(specOf(id)), implementation_(implementation), function_(std::move(function)) {}

    auto spec() const -> const MethodSpec& override { return spec_; }
    auto implementation() const -> std::string override { return implementation_; }
    auto properties(double T, double P, const PropertiesSolvent& solvent, int state) const -> ElectroPropertiesSolvent override { return function_(T, P, solvent, state); }

private:
    const MethodSpec& spec_;
    std::string implementation_;
    Function function_;
};

} // namespace

auto waterHGK84GemsProperties(double T, double& P, int state, const std::string& triple, const WaterHGKgemsConstants& constants) -> PropertiesSolvent
{
    WaterTripleProperties wtr = waterTripleData().at(triple);

    double Pout = P;
    auto ps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        WaterHGKgems water_hgk(constants);
        real t = pass.T - C_to_K;
        real p = pass.P / bar_to_Pa;
        water_hgk.calculateWaterHGKgems(t, p, wtr);
        Pout = p.val() * bar_to_Pa;
        return water_hgk.propertiesWaterHGKgems(state);
    });
    P = Pout;

    // The GEMS model gives densityT = -alpha rho, densityP = beta rho, densityTT = rho (alpha^2 - dalpha/dT) and (analytical,
    // from the third derivative of P with respect to the density in the LVS and HGK regions) densityPP = -rho beta^2 Gamma, with
    // Gamma = rho P_rhorho/P_rho. The mixed second derivative is the exact derivative of the analytical alpha with respect to P:
    // densityTP = d(densityT)/dP (= d(densityP)/dT), and the mixed third derivative is d(densityTT)/dP.
    if (Reaktoro_::kWithAutodiff)
    {
        ps.densityTP.val = ps.densityT.ddp;
        ps.densityTP.ddt = ps.densityTT.ddp;
        ps.densityTP.ddp = ps.densityPP.ddt; // the mixed third derivative d3(rho)/dT dP2
        ps.densityTP.sta = ps.densityT.sta;
    }
    else
        ps.densityTP.sta = {Reaktoro_::Status::notdefined, "densityTP is the derivative of densityT: not calculated (built without autodiff)"};

    return ps;
}

auto waterHGK84GemsProperties(double T, double& P, int state, const std::string& triple) -> PropertiesSolvent
{
    static const auto constants = makeWaterHGKgemsConstants(builtinConstants("WaterHGK84", "gems")->parameters, builtinConstants("JohnsonNorton91")->parameters);
    return waterHGK84GemsProperties(T, P, state, triple, *constants);
}

auto waterHGK84GemsMethod(std::shared_ptr<const WaterHGKgemsConstants> constants) -> std::shared_ptr<const SolventMethod>
{
    return std::make_shared<const FunctionSolventMethod>("WaterHGK84", "gems",
        [constants](double T, double& P, int state, const std::string& triple) { return waterHGK84GemsProperties(T, P, state, triple, *constants); },
        [constants](double T, double& P, int state, const std::string& triple) { return hgk84GemsSubstance(T, P, state, triple, *constants); }, true);
}

namespace {

/// The values name_1, name_2, ... of a method.
template <typename Number = double>
auto listOf(const MethodApplication& a, const std::string& name) -> std::vector<Number>
{
    std::vector<Number> out;
    for (int i = 1;; ++i)
    {
        const auto it = a.parameters.find(name + "_" + std::to_string(i));
        if (it == a.parameters.end())
            break;
        out.push_back(static_cast<Number>(it->second.value));
    }
    if (out.empty())
        funError("Cannot make the method " + a.method, "It has no values of " + name + ".", __LINE__, __FILE__);
    return out;
}

auto valueOf(const MethodApplication& a, const char* name) -> double
{
    const auto it = a.parameters.find(name);
    if (it == a.parameters.end())
        funError("Cannot make the method " + a.method, std::string("It has no value of ") + name + ".", __LINE__, __FILE__);
    return it->second.value;
}

} // namespace

namespace {

auto saturationOf(const MethodApplication& a) -> WaterSaturationConstants
{
    WaterSaturationConstants s;
    for (int i = 1; i <= 6; ++i)
    {
        s.a.push_back(valueOf(a, ("sat_a" + std::to_string(i)).c_str()));
        s.b.push_back(valueOf(a, ("sat_b" + std::to_string(i)).c_str()));
        s.c.push_back(valueOf(a, ("sat_c" + std::to_string(i)).c_str()));
    }
    s.Tcr = valueOf(a, "Tcr"); s.Pcr = valueOf(a, "Pcr"); s.Dcr = valueOf(a, "Dcr"); s.M = valueOf(a, "M");
    return s;
}

} // namespace

auto waterHGK84ReaktoroMethod(const MethodApplication& a) -> std::shared_ptr<const SolventMethod>
{
    HGKConstants k;
    k.saturation = saturationOf(a); k.Ttr = valueOf(a, "Ttr");
    k.A0 = listOf(a, "A0"); k.A1 = listOf(a, "A1"); k.yc = listOf(a, "yc"); k.A3 = listOf(a, "A3"); k.alpha = listOf(a, "alpha"); k.beta = listOf(a, "beta");
    k.ri = listOf(a, "ri"); k.ti = listOf(a, "ti"); k.A4 = listOf(a, "A4");
    k.ki = listOf<int>(a, "ki"); k.li = listOf<int>(a, "li"); k.mi = listOf<int>(a, "mi"); k.ni = listOf<int>(a, "ni");
    k.referenceTemperature = valueOf(a, "referenceTemperature"); k.referenceDensity = valueOf(a, "referenceDensity"); k.referenceHelmholtz = valueOf(a, "referenceHelmholtz");
    k.referenceVolume = 1.0 / k.referenceDensity; k.A20 = valueOf(a, "A20"); k.z0 = valueOf(a, "z0");
    return std::make_shared<const FunctionSolventMethod>("WaterHGK84", "reaktoro",
        [k](double T, double& P, int state, const std::string&) { return hgk84ReaktoroProperties(T, P, state, k); },
        [k](double T, double& P, int state, const std::string& triple) { return hgk84ReaktoroSubstance(T, P, state, triple, k); }, true);
}

auto waterIAPWS95ReaktoroMethod(const MethodApplication& a) -> std::shared_ptr<const SolventMethod>
{
    WagnerPrussConstants k;
    k.saturation = saturationOf(a); k.Ttr = valueOf(a, "Ttr");
    k.no = listOf(a, "no"); k.gammao = listOf(a, "gammao"); k.c = listOf(a, "c"); k.d = listOf(a, "d"); k.t = listOf(a, "t"); k.n = listOf(a, "n");
    k.alpha = listOf(a, "alpha"); k.beta = listOf(a, "beta"); k.gamma = listOf(a, "gamma"); k.epsilon = listOf(a, "epsilon");
    k.a = listOf(a, "a"); k.b = listOf(a, "b"); k.A = listOf(a, "A"); k.B = listOf(a, "B"); k.C = listOf(a, "C"); k.F = listOf(a, "F"); k.E = listOf(a, "E");
    return std::make_shared<const FunctionSolventMethod>("WaterIAPWS95", "reaktoro",
        [k](double T, double& P, int state, const std::string&) { return iapws95ReaktoroProperties(T, P, state, k); },
        [k](double T, double& P, int state, const std::string& triple) { return iapws95ReaktoroSubstance(T, P, state, triple, k); }, true);
}

auto waterZhangDuan05Method(bool dew_gibbs, ZhangDuanConstants constants) -> std::shared_ptr<const SolventMethod>
{
    return std::make_shared<const FunctionSolventMethod>("WaterZhangDuan05", "",
        [constants](double T, double& P, int, const std::string&) { return zhangDuan05Properties(T, P, constants); },
        [dew_gibbs, constants](double T, double& P, int, const std::string&) { return zhangDuan05Substance(T, P, dew_gibbs, constants); }, false);
}

auto johnsonNorton91ReaktoroMethod(std::vector<double> constants) -> std::shared_ptr<const DielectricMethod>
{
    return std::make_shared<const FunctionDielectricMethod>("JohnsonNorton91", "reaktoro", [constants = std::move(constants)](double T, double P, const PropertiesSolvent& solvent, int state) {
        return johnsonNorton91Reaktoro(T, P, solvent, state, constants);
    });
}

auto johnsonNorton91GemsMethod(std::shared_ptr<const WaterHGKgemsConstants> constants) -> std::shared_ptr<const DielectricMethod>
{
    return std::make_shared<const FunctionDielectricMethod>("JohnsonNorton91", "gems", [constants](double T, double P, const PropertiesSolvent& solvent, int) {
        return johnsonNorton91Gems(T, P, solvent, *constants);
    });
}

auto sverjenskyEtAl14Method(std::vector<double> constants) -> std::shared_ptr<const DielectricMethod>
{
    return std::make_shared<const FunctionDielectricMethod>("SverjenskyEtAl14", "", [constants = std::move(constants)](double T, double P, const PropertiesSolvent& solvent, int) {
        return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
            return electroPropertiesWaterSverjensky2014(pass, solvent, constants);
        });
    });
}

auto fernandezEtAl97Method(std::vector<double> constants) -> std::shared_ptr<const DielectricMethod>
{
    return std::make_shared<const FunctionDielectricMethod>("FernandezEtAl97", "", [constants = std::move(constants)](double T, double P, const PropertiesSolvent& solvent, int) {
        return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
            return electroPropertiesWaterFernandez1997(pass, solvent, constants);
        });
    });
}

} // namespace ThermoFun::v2
