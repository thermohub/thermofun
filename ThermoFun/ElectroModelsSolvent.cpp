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
auto WaterJNreaktoro::electroPropertiesSolvent(double T, double P, PropertiesSolvent ps) -> ElectroPropertiesSolvent
{
//    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    auto t = Reaktoro_::Temperature(T);
    auto p = Reaktoro_::Pressure(P);

    if (P==0) p = Reaktoro_::Pressure(waterSaturatedPressureWagnerPruss(t).val);

    WaterThermoState wts;

    wts.density   = ps.density;
    wts.densityT  = ps.densityT;
    wts.densityP  = ps.densityP;
    wts.densityTT = ps.densityTT;
    wts.densityTP = ps.densityTP;
    wts.densityPP = ps.densityPP;

    WaterElectroState wes = waterElectroStateJohnsonNorton(t, /*p,*/ wts);

    return electroPropertiesWaterJNreaktoro(wes);
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
auto WaterJNgems::electroPropertiesSolvent(double T, double P) -> ElectroPropertiesSolvent
{
    WaterHGKgems water_hgk; T -= C_to_K; P /= bar_to_Pa;

    water_hgk.calculateWaterHGKgems(T, P);

    return water_hgk.electroPropertiesWaterJNgems(0); // state 0 = liquid
}

//=======================================================================================================
// Calculate the electro-chemical of water using the electro-chemical properties of water solvent
// using the Sverjensky et al. (2014) dielectric constant model together with the Zhang and Duan (2002) water
// PVT model
// References: Sverjensky et al. Water in the deep Earth: The dielectric constant and the solubilities
// of quartz and corundum to 60 kb and 1200 °C. GCA, 2014 129:125-145
// Zhang and Duan Prediction of the PVT properties of water over wide range of temperatures and pressures
// from molecular dynamics simulation. PEPI, 2002 149:335-354
// Added: DM 26.07.2016
//=======================================================================================================

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
auto WaterElectroSverjensky2014::electroPropertiesSolvent(double T, double P/*, PropertiesSolvent ps*/) -> ElectroPropertiesSolvent
{
//    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    auto t = Reaktoro_::Temperature(T); t -= C_to_K;
    auto p = Reaktoro_::Pressure(P); p /= bar_to_Pa;

    return electroPropertiesWaterSverjensky2014(/*ps,*/ t, p, pimpl->substance);
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
auto WaterElectroFernandez1997::electroPropertiesSolvent(double T, double P/*, PropertiesSolvent ps*/) -> ElectroPropertiesSolvent
{
//    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    auto t = Reaktoro_::Temperature(T); t -= C_to_K;
    auto p = Reaktoro_::Pressure(P); p /= bar_to_Pa;

    return electroPropertiesWaterFernandez1997(/*ps,*/ t, p, pimpl->substance); // t (celsius), p (bar)
}

} // End namespace ThermoFun

