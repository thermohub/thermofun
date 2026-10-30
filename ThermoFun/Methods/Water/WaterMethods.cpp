// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "WaterMethods.h"

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

} // namespace

auto WaterHGK84Gems::spec() const -> const MethodSpec& { return specOf("WaterHGK84"); }

auto WaterHGK84Gems::properties(double T, double& P, int state) const -> PropertiesSolvent
{
    return properties(T, P, state, "NEA_HGK");
}

auto WaterHGK84Gems::properties(double T, double& P, int state, const std::string& triple) const -> PropertiesSolvent
{
    WaterTripleProperties wtr = waterTripleData.at(triple);

    double Pout = P;
    auto ps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        WaterHGKgems water_hgk;
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

auto WaterHGK84Gems::substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance
{
    WaterTripleProperties wtr = waterTripleData.at(triple);

    double Pout = P;
    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        WaterHGKgems water_hgk;
        real t = pass.T - C_to_K;
        real p = pass.P / bar_to_Pa;
        water_hgk.calculateWaterHGKgems(t, p, wtr);
        Pout = p.val() * bar_to_Pa;
        return water_hgk.thermoPropertiesWaterHGKgems(state);
    });
    P = Pout;

    return tps;
}

auto WaterHGK84Reaktoro::spec() const -> const MethodSpec& { return specOf("WaterHGK84"); }

auto WaterHGK84Reaktoro::properties(double T, double& P, int state) const -> PropertiesSolvent
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T)).val();
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return propertiesWaterHGKreaktoro(waterThermoStateHGK(pass.T, pass.P, state));
    });
}

auto WaterHGK84Reaktoro::substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T)).val();
    WaterTripleProperties wtr = waterTripleData.at(triple);
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesWaterHGKreaktoro(pass.T, waterThermoStateHGK(pass.T, pass.P, state), wtr);
    });
}

auto WaterIAPWS95Reaktoro::spec() const -> const MethodSpec& { return specOf("WaterIAPWS95"); }

auto WaterIAPWS95Reaktoro::properties(double T, double& P, int state) const -> PropertiesSolvent
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T)).val();
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return propertiesWaterWP95reaktoro(waterThermoStateWagnerPruss(pass.T, pass.P, state));
    });
}

auto WaterIAPWS95Reaktoro::substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance
{
    if (P==0) P = waterSaturatedPressureWagnerPruss(real(T)).val();
    WaterTripleProperties wtr = waterTripleData.at(triple);
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return thermoPropertiesWaterWP95reaktoro(pass.T, waterThermoStateWagnerPruss(pass.T, pass.P, state), wtr);
    });
}

auto WaterZhangDuan05::spec() const -> const MethodSpec& { return specOf("WaterZhangDuan05"); }

auto WaterZhangDuan05::properties(double T, double& P, int /*state*/) const -> PropertiesSolvent
{
    checkZhangDuan(T, P);
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return propertiesWaterZhangDuan2005(pass);
    });
}

auto WaterZhangDuan05::substanceProperties(double T, double& P, int /*state*/, const std::string& /*triple*/) const -> ThermoPropertiesSubstance
{
    checkZhangDuan(T, P);
    auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        auto out = thermoPropertiesWaterZhangDuan2005(pass.T, pass.P / bar_to_Pa); // pressure in bar
        if (dew_gibbs_)
            out.gibbs_energy = waterGibbsEnergyDEW(pass.T, pass.P / bar_to_Pa, coefficients_);
        return out;
    });
    // the volume is calculated by this model, and the Gibbs energy of DEW if it is asked for
    const Reaktoro_::StatusMessage undefined = {Reaktoro_::Status::notdefined, ""};
    for (auto* x : {&tps.helmholtz_energy, &tps.internal_energy, &tps.enthalpy,
                    &tps.entropy, &tps.heat_capacity_cp, &tps.heat_capacity_cv})
        x->sta = undefined;
    if (dew_gibbs_)
        tps.gibbs_energy.asCalculated();
    else
        tps.gibbs_energy.sta = undefined;
    return tps;
}

auto JohnsonNorton91Reaktoro::spec() const -> const MethodSpec& { return specOf("JohnsonNorton91"); }

auto JohnsonNorton91Reaktoro::properties(double T, double P, const PropertiesSolvent& ps, int state) const -> ElectroPropertiesSolvent
{
    auto eps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        WaterThermoState wts;
        wts.density   = pass(ps.density);
        wts.densityT  = pass(ps.densityT);
        wts.densityP  = pass(ps.densityP);
        wts.densityTT = pass(ps.densityTT);
        wts.densityTP = pass(ps.densityTP);
        wts.densityPP = pass(ps.densityPP);
        return electroPropertiesWaterJNreaktoro(waterElectroStateJohnsonNorton(pass.T, /*pass.P,*/ wts, state));
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

auto JohnsonNorton91Gems::spec() const -> const MethodSpec& { return specOf("JohnsonNorton91"); }

auto JohnsonNorton91Gems::properties(double T, double P, const PropertiesSolvent& /*solvent*/, int state) const -> ElectroPropertiesSolvent
{
    WaterTripleProperties wtr = waterTripleData.at("NEA_HGK");
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        WaterHGKgems water_hgk;
        real t = pass.T - C_to_K;
        real p = pass.P / bar_to_Pa;
        water_hgk.calculateWaterHGKgems(t, p, wtr);
        return water_hgk.electroPropertiesWaterJNgems(state); // state 0 = liquid
    });
}

auto SverjenskyEtAl14::spec() const -> const MethodSpec& { return specOf("SverjenskyEtAl14"); }

auto SverjenskyEtAl14::properties(double T, double P, const PropertiesSolvent& solvent, int /*state*/) const -> ElectroPropertiesSolvent
{
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return electroPropertiesWaterSverjensky2014(pass, solvent);
    });
}

auto FernandezEtAl97::spec() const -> const MethodSpec& { return specOf("FernandezEtAl97"); }

auto FernandezEtAl97::properties(double T, double P, const PropertiesSolvent& solvent, int /*state*/) const -> ElectroPropertiesSolvent
{
    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return electroPropertiesWaterFernandez1997(pass, solvent);
    });
}

} // namespace ThermoFun::v2
