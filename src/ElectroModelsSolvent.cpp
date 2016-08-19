#include "ElectroModelsSolvent.h"

#include "Solvent/Reaktoro/WaterUtils.hpp"
#include "Solvent/Reaktoro/WaterThermoState.hpp"

#include "Solvent/WaterHGK-JNgems.h"
#include "Solvent/WaterJN91reaktoro.h"
#include "Solvent/WaterElectroSverjensky2014.h"

namespace TCorrPT {

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

    auto t = Reaktoro_::Temperature(T + C_to_K);
    auto p = Reaktoro_::Pressure(P * bar_to_Pa);

    if (P==0) p = Reaktoro_::Pressure(Reaktoro_::waterSaturatedPressureWagnerPruss(t).val);

    Reaktoro_::WaterThermoState wts;

    wts.density   = ps.density;
    wts.densityT  = ps.densityT;
    wts.densityP  = ps.densityP;
    wts.densityTT = ps.densityTT;
    wts.densityTP = ps.densityTP;
    wts.densityPP = ps.densityPP;

    Reaktoro_::WaterElectroState wes = Reaktoro_::waterElectroStateJohnsonNorton(t, /*p,*/ wts);

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
    WaterHGKgems water_hgk;

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
auto WaterElectroSverjensky2014::electroPropertiesSolvent(double T, double P, PropertiesSolvent ps) -> ElectroPropertiesSolvent
{
//    if (P==0) P = saturatedWaterVaporPressureHGK(T+C_to_K);

    auto t = Reaktoro_::Temperature(T/* + C_to_K*/);
    auto p = Reaktoro_::Pressure(P /* * bar_to_Pa*/);

    return electroPropertiesWaterSverjensky2014(ps, t, p);
}

} // End namespace TCorrPT

