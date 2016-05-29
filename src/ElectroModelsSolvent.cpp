#include "ElectroModelsSolvent.h"

#include "Solvent/WaterJN91reaktoro.h"
#include "Solvent/Reaktoro/WaterUtils.hpp"
#include "Solvent/Reaktoro/WaterThermoState.hpp"

#include "Solvent/WaterHGK-JNgems.h"

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

    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    if (P==0) p = Reaktoro::Pressure(Reaktoro::waterSaturatedPressureWagnerPruss(t).val);

    Reaktoro::WaterThermoState wts;

    wts.density   = ps.density;
    wts.densityT  = ps.densityT;
    wts.densityP  = ps.densityP;
    wts.densityTT = ps.densityTT;
    wts.densityTP = ps.densityTP;
    wts.densityPP = ps.densityPP;

    Reaktoro::WaterElectroState wes = Reaktoro::waterElectroStateJohnsonNorton(t, p, wts);

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

    return water_hgk.electroPropertiesWaterJNgems();
}

} // End namespace TCorrPT

