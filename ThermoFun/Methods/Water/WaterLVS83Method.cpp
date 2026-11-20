// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// WaterLVS83 as a method: the equation of Levelt Sengers et al. (1983) in its region near the critical point. The results
// are made as the GEMS water code made them (units, the reference of the triple point, transport properties).

#include "WaterMethods.h"

#include <cmath>

#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Methods/MethodCatalog.h>
#include <ThermoFun/Methods/Water/WaterLVS83.h>
#include <ThermoFun/Methods/Water/WaterTransportGems.h>

namespace ThermoFun::model {

namespace {

constexpr double fh = 4.305816e0;  // kJ/kg to cal/mol (the unit factor of the GEMS code)

struct LVSContext
{
    std::shared_ptr<const WaterLVS83Constants> lvs;
    WaterTransportGemsConstants transport;
    double Ttr = 273.16;  // K
};

/// The phase of the state at T (K) and P (Pa) and whether it is saturated; P = 0 or P = Psat is the saturation, and Pout
/// gets the pressure in Pa.
auto lvsPhase(WaterLVS83& lvs, real T, real P, int state, double& Pout, bool& saturated) -> WaterLVS83Phase
{
    const real Pbar = P / bar_to_Pa;
    const real PMPa = Pbar / 10.0;
    saturated = false;
    if (lvs.coversSaturation(T.val()))
    {
        const double Ps = lvs.saturationPressure(real(T.val())).val();
        if (P.val() == 0.0 || std::fabs(PMPa.val() - Ps) <= 1e-12 * Ps)
            saturated = true;
    }
    WaterLVS83Phase ph;
    if (saturated)
        ph = lvs.saturation(T)[state == 1 ? 1 : 0];
    else
        ph = lvs.fluid(T, PMPa);
    Pout = ph.P.val() * 10.0 * bar_to_Pa;
    return ph;
}

auto lvsSolvent(const LVSContext& c, const WaterLVS83Phase& ph, bool saturatedLiquid) -> PropertiesSolventAD
{
    PropertiesSolventAD wp;
    const real rho = ph.D;
    const real bet = (ph.beta / 10.0) / 1e05;  // 1/MPa to 1/bar to 1/Pa
    wp.Alpha = ph.alpha;
    wp.Beta = bet;
    wp.dAldT = ph.dAldT;
    wp.density = rho;
    wp.densityT = -ph.alpha * rho;
    wp.densityTT = rho * (pow(ph.alpha, 2.) - ph.dAldT);
    wp.densityP = bet * rho;
    wp.densityPP = -rho * bet * bet * ph.gamma;
    wp.pressure = ph.P * 10.0 * 1e5;

    // transport properties in SI, as the GEMS code: surface tension only for the liquid at saturation
    const real pbars = ph.P * 1.0e1;
    const real betaPa = ph.beta / 1.0e6;
    const real CpJKkg = ph.Cp * 1.0e3;
    const real visc = waterViscosityGems(c.transport, ph.T, pbars, rho, betaPa);
    const real cond = waterThermalConductivityGems(c.transport, ph.T, pbars, rho, ph.alpha, betaPa);
    wp.dynamic_viscosity = visc.val();
    wp.thermal_conductivity = cond.val();
    wp.surface_tension = saturatedLiquid ? waterSurfaceTensionGems(c.transport, ph.T).val() : 0.0;
    wp.speed_of_sound = ph.speed.val();
    wp.Tdiff = (cond / (rho * CpJKkg)).val();
    wp.Prndtl = (cond.val() != 0.0) ? (visc * CpJKkg / cond).val() : 0.0;
    wp.Visck = (visc / rho).val();
    wp.Albe = ph.alpha / betaPa;
    return wp;
}

auto lvsSubstance(const LVSContext& c, const WaterLVS83Phase& ph, const WaterTripleProperties& wtr) -> ThermoPropertiesSubstanceAD
{
    // to cal/mol, then the reference of the triple point (triple() of the GEMS code)
    const double conv = fh * cal_to_J;
    const real Stri = (wtr.Str / conv) * fh, Utri = (wtr.Utr / conv) * fh, Htri = (wtr.Htr / conv) * fh, Atri = (wtr.Atr / conv) * fh, Gtri = (wtr.Gtr / conv) * fh;
    real A = ph.A * fh, G = ph.G * fh, S = ph.S * fh, U = ph.U * fh, H = ph.H * fh;
    const real Cv = ph.Cv * fh, Cp = ph.Cp * fh;
    S = S + Stri;
    const real TS = ph.T * S - c.Ttr * Stri;
    G = H - TS + Gtri;
    A = U - TS + Atri;
    H = H + Htri;
    U = U + Utri;

    ThermoPropertiesSubstanceAD wp;
    wp.helmholtz_energy = A * cal_to_J;
    wp.gibbs_energy = G * cal_to_J;
    wp.entropy = S * cal_to_J;
    wp.internal_energy = U * cal_to_J;
    wp.enthalpy = H * cal_to_J;
    wp.heat_capacity_cv = Cv * cal_to_J;
    wp.heat_capacity_cp = Cp * cal_to_J;
    wp.volume = 1 / (ph.D / 1.0e3) * gemsWaterMolarMass() * cm3_mol_to_J_bar;  // g/cm3, as the GEMS code
    return wp;
}

class WaterLVS83Method final : public SolventMethod
{
public:
    explicit WaterLVS83Method(LVSContext context) : spec_(*MethodCatalog::instance().find("WaterLVS83", OperatesOn::Substance)), c_(std::move(context)) {}

    auto spec() const -> const MethodSpec& override { return spec_; }
    auto implementation() const -> std::string override { return ""; }
    auto gibbsEnergyContainsRTlnP() const -> bool override { return true; }

    auto coversSaturation(double T) const -> bool override
    {
        WaterLVS83 lvs(c_.lvs);
        return lvs.coversSaturation(T);
    }

    // In the region, and the stable phase: the equation has no metastable states
    auto covers(double T, double P, int state) const -> bool override
    {
        WaterLVS83 lvs(c_.lvs);
        const double PMPa = P / bar_to_Pa / 10.0;
        if (!lvs.coversSaturation(T))
            return T >= lvs.criticalTemperature() && lvs.inRegion(T, PMPa);
        const double Ps = lvs.saturationPressure(real(T)).val();
        if (std::fabs(PMPa - Ps) <= 1e-12 * Ps)
            return true;
        if (!lvs.inRegion(T, PMPa))
            return false;
        return state == 1 ? PMPa < Ps : PMPa > Ps;
    }

    auto properties(double T, double& P, int state) const -> PropertiesSolvent override
    {
        double Pout = P;
        auto ps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
            WaterLVS83 lvs(c_.lvs);
            bool saturated = false;
            const auto ph = lvsPhase(lvs, pass.T, pass.P, state, Pout, saturated);
            return lvsSolvent(c_, ph, saturated && state != 1);
        });
        P = Pout;
        // the mixed derivatives as for the GEMS HGK code
        ps.densityTP.val = ps.densityT.ddp;
        ps.densityTP.ddt = ps.densityTT.ddp;
        ps.densityTP.ddp = ps.densityPP.ddt;
        ps.densityTP.sta = ps.densityT.sta;
        return ps;
    }

    auto substanceProperties(double T, double& P, int state, const std::string& triple) const -> ThermoPropertiesSubstance override
    {
        const WaterTripleProperties wtr = waterTripleData().at(triple);
        double Pout = P;
        auto tps = twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
            WaterLVS83 lvs(c_.lvs);
            bool saturated = false;
            const auto ph = lvsPhase(lvs, pass.T, pass.P, state, Pout, saturated);
            return lvsSubstance(c_, ph, wtr);
        });
        P = Pout;
        return tps;
    }

private:
    const MethodSpec& spec_;
    LVSContext c_;
};

} // namespace

auto waterLVS83Method(const std::map<std::string, Quantity>& values, const std::map<std::string, Quantity>& transport) -> std::shared_ptr<const SolventMethod>
{
    LVSContext c;
    c.lvs = makeWaterLVS83Constants(values);
    c.transport = readWaterTransportGemsConstants(transport);
    c.Ttr = values.at("Ttr").value;
    return std::make_shared<const WaterLVS83Method>(std::move(c));
}

} // namespace ThermoFun::model
