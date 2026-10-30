// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "ThermoModelsSolvent.h"
#include "Methods/Water/WaterMethods.h"
#include "ThermoProperties.h"
#include "Substance.h"

//#include "Substance.h"
#include "Methods/Water/WaterHGK-JNgems.h"
#include "Methods/Water/WaterHGKreaktoro.h"
#include "Methods/Water/WaterWP95reaktoro.h"
#include "Methods/Water/WaterZhangDuan2005.h"
#include "Methods/Water/Reaktoro/WaterUtils.hpp"
#include "Methods/Water/Reaktoro/WaterThermoState.hpp"
#include "Methods/Water/Reaktoro/WaterThermoStateUtils.hpp"
#include "Common/Exception.h"

namespace ThermoFun {

//=======================================================================================================
// Calculate the properties of water using the Haar-Gallagher-Kell (1984) equation of state
// References: HAAR L., GALLAGHER J. S., and KELL G. S. Steam Tables, Thermodynamic and Transport Properties
// and Computer Programs for Vapor and Liquid States of Wafer in SI Unites. 1984, Hemisphere Publishing Co.
// Added: DM 08.05.2016
//=======================================================================================================
struct WaterHGK::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterHGK::WaterHGK(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterHGK::propertiesSolvent(double T, double &P, int state, std::string triple) -> PropertiesSolvent
{
    return v2::WaterHGK84Gems().properties(T, P, state, triple);
}

auto WaterHGK::thermoPropertiesSubstance(double T, double &P, int state, std::string triple) -> ThermoPropertiesSubstance
{
    return v2::WaterHGK84Gems().substanceProperties(T, P, state, triple);
}

//=======================================================================================================
// Calculate the properties of water using the Haar-Gallagher-Kell (1984) equation of state as
// implemented in Reaktoro
// References:
// Added: DM 12.05.2016
//=======================================================================================================

struct WaterHGKreaktoro::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterHGKreaktoro::WaterHGKreaktoro(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterHGKreaktoro::propertiesSolvent(double T, double &P, int state) -> PropertiesSolvent
{
    return v2::WaterHGK84Reaktoro().properties(T, P, state);
}

auto WaterHGKreaktoro::thermoPropertiesSubstance(double T, double &P, int state, std::string tripple) -> ThermoPropertiesSubstance
{
    return v2::WaterHGK84Reaktoro().substanceProperties(T, P, state, tripple);
}

//=======================================================================================================
// Calculate the properties of water using the Wagner and Pruss (1995) equation of state as
// implemented in Reaktoro
// References: Wagner, W., Prub, A. The IAPWS formulation 1995 for the thermodynamic properties of ordinary
// water substance for general and scientific use. J. Phys. Chem. Ref. Data, 2002 31(2):387–535.
// Added: DM 12.05.2016
//=======================================================================================================

struct WaterWP95reaktoro::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterWP95reaktoro::WaterWP95reaktoro(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterWP95reaktoro::propertiesSolvent(double T, double &P, int state) -> PropertiesSolvent
{
    return v2::WaterIAPWS95Reaktoro().properties(T, P, state);
}

auto WaterWP95reaktoro::thermoPropertiesSubstance(double T, double &P, int state, std::string tripple) -> ThermoPropertiesSubstance
{
    return v2::WaterIAPWS95Reaktoro().substanceProperties(T, P, state, tripple);
}

//=======================================================================================================
// Calculate the properties of water using the Zhang and Duan (2005) equation of state
// References:
// Added: DM 21.07.2016
//=======================================================================================================

struct WaterZhangDuan2005::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterZhangDuan2005::WaterZhangDuan2005(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto WaterZhangDuan2005::propertiesSolvent(double T, double P, int state) -> PropertiesSolvent
{
    double p = P;
    return v2::WaterZhangDuan05().properties(T, p, state);
}

auto WaterZhangDuan2005::thermoPropertiesSubstance(double T, double P, int state) -> ThermoPropertiesSubstance
{
    double p = P;
    return v2::WaterZhangDuan05().substanceProperties(T, p, state, "");
}

} // namespace ThermoFun

