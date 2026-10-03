#include "WaterElectroSverjensky2014.h"
#include "ThermoEngine.h"
#include "Database.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "Substances/Solvent/SolventDensityDerivatives.hpp"

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

/// The dielectric constant of water of Sverjensky et al. (2014): eps = exp(u(T)) rho^w(T), with u = b1 t + b2 sqrt(t) + b3 and
/// w = a1 t + a2 sqrt(t) + a3 (t in deg C, rho in g/cm3). With L = ln eps = u + w ln(rho) the derivatives are analytical:
///   L_T = u' + w' ln(rho) + w rho_T/rho,   L_P = w rho_P/rho,
///   L_TT = u'' + w'' ln(rho) + 2 w' rho_T/rho + w (rho_TT/rho - (rho_T/rho)^2),
///   L_TP = w' rho_P/rho + w (rho_TP/rho - rho_T rho_P/rho^2),   L_PP = w (rho_PP/rho - (rho_P/rho)^2),
///   eps_T = eps L_T, eps_P = eps L_P, eps_TT = eps (L_T^2 + L_TT), eps_TP = eps (L_T L_P + L_TP), eps_PP = eps (L_P^2 + L_PP)
/// where the derivatives of the density are those of the solvent model (their own derivatives are the third-order ones).
auto electroPropertiesWaterSverjensky2014(const Reaktoro_::Pass& pass, Substance substance, int state) -> ElectroPropertiesSolventAD
{
    ElectroPropertiesSolventAD wep;

    Database db; db.addSubstance(substance);
    ThermoEngine   th(db);

    double P1 = pass.P.val();  // propertiesSolvent takes the pressure by reference
    const auto psol = th.propertiesSolvent(pass.T.val(), P1, substance.symbol(), state);
    const auto r = densityDerivatives(pass, psol);

    const real TC  = pass.T - 273.15;      // temperature in celsius
    const real sT  = sqrt(TC);
    const real u   = b[1]*TC + b[2]*sT + b[3];
    const real up  = b[1] + 0.5*b[2]/sT;
    const real upp = -0.25*b[2]/(sT*TC);
    const real w   = a[1]*TC + a[2]*sT + a[3];
    const real wp  = a[1] + 0.5*a[2]/sT;
    const real wpp = -0.25*a[2]/(sT*TC);

    const real lnrho = log(r.rho/1000.0);
    const real dT  = r.T/r.rho,  dP  = r.P/r.rho;
    const real dTT = r.TT/r.rho - dT*dT, dTP = r.TP/r.rho - dT*dP, dPP = r.PP/r.rho - dP*dP;

    const real LT  = up + wp*lnrho + w*dT;
    const real LP  = w*dP;
    const real LTT = upp + wpp*lnrho + 2.0*wp*dT + w*dTT;
    const real LTP = wp*dP + w*dTP;
    const real LPP = w*dPP;

    const real eps = exp(u + w*lnrho);
    const real epsilon2 = eps * eps;

    // the pressure derivatives per bar (the derivatives with respect to P in Pa multiplied by bar_to_Pa)
    wep.epsilon   = eps;
    wep.epsilonT  = eps*LT;
    wep.epsilonP  = eps*LP*bar_to_Pa;
    wep.epsilonTT = eps*(LT*LT + LTT);
    wep.epsilonTP = eps*(LT*LP + LTP)*bar_to_Pa;
    wep.epsilonPP = eps*(LP*LP + LPP)*bar_to_Pa*bar_to_Pa;
    wep.bornZ = -1.0/wep.epsilon;
    wep.bornY = wep.epsilonT/epsilon2;
    wep.bornQ = wep.epsilonP/epsilon2*1e-05; // from 1/bar to 1/Pa
//	we.bornU = we.epsilonTP/epsilon2 - 2.0*we.bornY*we.bornQ*we.epsilon;
//	we.bornN = we.epsilonPP/epsilon2 - 2.0*we.bornQ*we.bornQ*we.epsilon;
    wep.bornX = wep.epsilonTT/epsilon2 - 2.0*wep.bornY*wep.bornY*wep.epsilon;

    return wep;
}

}
