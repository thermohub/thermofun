// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include "ElectroModelsSolvent.h"
#include "Substance.h"
#include "Database.h"
#include "ThermoEngine.h"
#include "Methods/Water/WaterMethods.h"
#include "ThermoProperties.h"
#include "Methods/Water/Reaktoro/WaterElectroState.hpp"
// Models
#include "Methods/Water/Reaktoro/WaterUtils.hpp"
#include "Methods/Water/Reaktoro/WaterThermoState.hpp"
#include "Methods/Water/WaterHGK-JNgems.h"
#include "Methods/Water/WaterJN91reaktoro.h"
#include "Methods/Water/WaterElectroSverjensky2014.h"
#include "Methods/Water/WaterElectroFernandez1997.h"
#include "Methods/Water/Reaktoro/WaterElectroStateJohnsonNorton.hpp"

namespace ThermoFun {

namespace {

// The properties of the solvent that a dielectric model needs, from the methods of its record.
auto solventOf(const Substance& substance, double T, double P, int state) -> PropertiesSolvent
{
    Database db;
    db.addSubstance(substance);
    ThermoEngine engine(db);
    return engine.propertiesSolvent(T, P, substance.symbol(), state);
}

} // namespace

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
    return v2::JohnsonNorton91Reaktoro().properties(T, P, ps, state);
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
    return v2::JohnsonNorton91Gems().properties(T, P, PropertiesSolvent{}, state);
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
    return v2::SverjenskyEtAl14().properties(T, P, solventOf(pimpl->substance, T, P, state), state);
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
    return v2::FernandezEtAl97().properties(T, P, solventOf(pimpl->substance, T, P, state), state);
}

} // End namespace ThermoFun

