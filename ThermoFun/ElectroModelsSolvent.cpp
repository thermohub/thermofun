// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "ElectroModelsSolvent.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "Substances/Solvent/Reaktoro/WaterElectroState.hpp"
// Models
#include "Substances/Solvent/Reaktoro/WaterUtils.hpp"
#include "Substances/Solvent/Reaktoro/WaterThermoState.hpp"
#include "Substances/Solvent/WaterHGK-JNgems.h"
#include "Substances/Solvent/WaterJN91reaktoro.h"
#include "Substances/Solvent/WaterElectroSverjensky2014.h"
#include "Substances/Solvent/WaterElectroFernandez1997.h"
#include "Substances/Solvent/Reaktoro/WaterElectroStateJohnsonNorton.hpp"

namespace ThermoFun {

//=======================================================================================================
// Calculate the electro-chemical of water using the Jhonson and Norton (1991) model as implemented in
// Reaktoro
// References: Critical phenomena in hydrothermal systems; state, thermodynamic, electrostatic, and
// transport properties of H2O in the critical region. Am J Sci, 1991 291:541-648;
// Added: DM 20.05.2016
//=======================================================================================================

struct WaterJNreaktoro::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterJNreaktoro::WaterJNreaktoro(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterJNreaktoro::electroPropertiesSolvent(double T, double P, PropertiesSolvent ps, int state) -> ElectroPropertiesSolvent
{
//    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

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

//=======================================================================================================
// Calculate the electro-chemical of water using the Jhonson and Norton (1991) model as implemented in
// GEMS
// References: Critical phenomena in hydrothermal systems; state, thermodynamic, electrostatic, and
// transport properties of H2O in the critical region. Am J Sci, 1991 291:541-648;
// Added: DM 20.05.2016
//=======================================================================================================

struct WaterJNgems::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterJNgems::WaterJNgems(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterJNgems::electroPropertiesSolvent(double T, double P, int state) -> ElectroPropertiesSolvent
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


struct WaterElectroSverjensky2014::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterElectroSverjensky2014::WaterElectroSverjensky2014(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterElectroSverjensky2014::electroPropertiesSolvent(double T, double P/*, PropertiesSolvent ps*/, int state) -> ElectroPropertiesSolvent
{
//    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return electroPropertiesWaterSverjensky2014(pass, pimpl->substance, state);
    });
}

//=======================================================================================================
// Calculate the electro-chemical of water using the electro-chemical properties of water solvent
// using the Fernandez et al. (1997) dielectric constant model
// References: D.P. Fernandez, A.R.H. Goodwin, E.W. Lemmon, J.M.H.L.Sengers, and R.C. Williams, 1997, A
// Formulation for the Static Permittivity of Water and Steam at Temperatures from 238 K to 873 K at Pressures
// up to 1200 MPa, Including Derivatives and Debye–Hückel Coefficients, J. Phys. Chem. Ref. Data 26, 1125.
// Added: DM 19.10.2016
//=======================================================================================================

struct WaterElectroFernandez1997::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterElectroFernandez1997::WaterElectroFernandez1997(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterElectroFernandez1997::electroPropertiesSolvent(double T, double P/*, PropertiesSolvent ps*/, int state) -> ElectroPropertiesSolvent
{
//    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    return twoPass(T, P, [&](const Reaktoro_::Pass& pass) {
        return electroPropertiesWaterFernandez1997(pass, pimpl->substance, state);
    });
}

} // End namespace ThermoFun

