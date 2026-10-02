#include "WaterElectroSverjensky2014.h"
#include "ThermoEngine.h"
#include "Database.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "Common/Jets.hpp"

namespace ThermoFun {

const double a[] =
{
     0.0000000000,
    -1.576377e-03,
     6.810288e-02,
     7.548755e-01
};

const double b[] =
{
     0.0000000000,
    -8.016651e-05,
    -6.871618e-02,
     4.747973
};

/// The dielectric constant as a function of the temperature (deg C) and of the density (g/cm3)
template<class Number>
auto epsilonS(Number T, Number RHO) -> Number
{
   return exp(b[1]*T + b[2]*pow(T,0.5) + b[3])*pow(RHO,(a[1]*T + a[2]*pow(T,0.5) + a[3]));
}

auto electroPropertiesWaterSverjensky2014(const Reaktoro_::Pass& pass, Substance substance, int state) -> ElectroPropertiesSolventAD
{
    ElectroPropertiesSolventAD wep;

    Database db; db.addSubstance(substance);
    ThermoEngine   th(db);

    const double T0 = pass.T.val();   // K
    const double P0 = pass.P.val();   // Pa

    double P1 = P0;  // propertiesSolvent takes the pressure by reference
    const auto psol = th.propertiesSolvent(T0, P1, substance.symbol(), state);

    // The exact derivatives (no finite differences): the density is the Taylor polynomial of its derivatives given by
    // the solvent model, and the dielectric constant (a closed form function of T and the density) is evaluated with
    // higher-order autodiff numbers. The third-order derivatives are the derivatives (ddt, ddp) of the second-order ones.
    const auto rho = jets::densityJet(psol);
    auto epsilon = [&](jets::Dual T, jets::Dual P) -> jets::Dual
    {
        const jets::Dual density = jets::taylor(rho, T - T0, P - P0) / 1000.0;   // g/cm3
        return epsilonS<jets::Dual>(T - 273.15, density);                         // T in deg C
    };
    const auto e = jets::jet(epsilon, T0, P0);

    // the pressure in bar (the derivatives with respect to P in Pa are divided by bar_to_Pa)
    const double b1 = bar_to_Pa, b2 = bar_to_Pa*bar_to_Pa;

    const real eps = jets::along(pass, e.f, e.fT, e.fP);
    const real epsilon2 = eps * eps;
    wep.epsilon   = eps;
    wep.epsilonT  = jets::along(pass, e.fT,       e.fTT,       e.fTP);
    wep.epsilonP  = jets::along(pass, e.fP*b1,    e.fTP*b1,    e.fPP*b1);
    wep.epsilonTT = jets::along(pass, e.fTT,      e.fTTT,      e.fTTP);
    wep.epsilonTP = jets::along(pass, e.fTP*b1,   e.fTTP*b1,   e.fTPP*b1);
    wep.epsilonPP = jets::along(pass, e.fPP*b2,   e.fTPP*b2,   e.fPPP*b2);
    wep.bornZ = -1.0/wep.epsilon;
    wep.bornY = wep.epsilonT/epsilon2;
    wep.bornQ = wep.epsilonP/epsilon2*1e-05; // from 1/bar to 1/Pa
//	we.bornU = we.epsilonTP/epsilon2 - 2.0*we.bornY*we.bornQ*we.epsilon;
//	we.bornN = we.epsilonPP/epsilon2 - 2.0*we.bornQ*we.bornQ*we.epsilon;
    wep.bornX = wep.epsilonTT/epsilon2 - 2.0*wep.bornY*wep.bornY*wep.epsilon;

    return wep;
}

}
