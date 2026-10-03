#include "WaterElectroSverjensky2014.h"
#include "ThermoEngine.h"
#include "Database.h"
#include "Substance.h"
#include "ThermoProperties.h"

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

auto epsilonS(real T, real RHO) -> real
{
   return exp(b[1]*T + b[2]*pow(T,0.5) + b[3])*pow(RHO,(a[1]*T + a[2]*pow(T,0.5) + a[3]));
}

auto electroPropertiesWaterSverjensky2014(const Reaktoro_::Pass& pass, Substance substance, int state) -> ElectroPropertiesSolventAD
{
    ElectroPropertiesSolventAD wep;

    Database db; db.addSubstance(substance);
    ThermoEngine   th(db);

    const real TC   = pass.T - 273.15;      // temperature in celsius
    const real Pbar = pass.P / 1e05;        // pressure in bar

    real TK = TC + 273.15;

    double P1 = pass.P.val();  // propertiesSolvent takes the pressure by reference
    auto psol = th.propertiesSolvent(TK.val(), P1, substance.symbol(), state);

    const real RHO = pass(psol.density) /1000;
    const auto eps = epsilonS(TC, RHO);
    const auto epsilon2 = eps * eps;

    // numerical approximation of epsilonT and epsilonTT
    real T_plus = TC + TC.val()*0.001;
    double P5 = pass.P.val();  // propertiesSolvent takes the pressure by reference
    real RHO_plus = pass(th.propertiesSolvent(T_plus.val()+273.15, P5, substance.symbol(), state).density) / 1000;
    auto eps_plus = epsilonS(T_plus, RHO_plus);

    real T_minus = TC - TC.val()*0.001;
    double P6 = pass.P.val();  // propertiesSolvent takes the pressure by reference
    real RHO_minus  = pass(th.propertiesSolvent(T_minus.val()+273.15, P6, substance.symbol(), state).density) / 1000;
    auto eps_minus = epsilonS(T_minus, RHO_minus);

    const auto epsilonT  = (eps_plus - eps_minus) / ((T_plus-T_minus));
    const auto epsilonTT = (eps_plus + eps_minus - 2*eps)/pow(((T_plus-T_minus)*0.5),2);

    // numerical approximation of epsilonP and epsilonPP
    real P_plus = Pbar + Pbar.val()*0.001;
    double P2 = P_plus.val()*1e05;  // propertiesSolvent takes the pressure by reference
    RHO_plus = pass(th.propertiesSolvent(TK.val(), P2, substance.symbol(), state).density) / 1000;
    eps_plus = epsilonS(TC, RHO_plus);

    real P_minus = Pbar - Pbar.val()*0.001;
    double P3 = P_minus.val()*1e05;  // propertiesSolvent takes the pressure by reference
    RHO_minus = pass(th.propertiesSolvent(TK.val(), P3, substance.symbol(), state).density) / 1000;
    eps_minus = epsilonS(TC, RHO_minus);

    const auto epsilonP  = (eps_plus - eps_minus) / ((P_plus-P_minus));
    const auto epsilonPP = (eps_plus + eps_minus - 2*eps)/pow(((P_plus-P_minus)*0.5),2);

    wep.epsilon   = eps;
    wep.epsilonP  = epsilonP;
    wep.epsilonPP = epsilonPP;
    wep.epsilonT  = epsilonT;
    wep.epsilonTT = epsilonTT;
    wep.bornZ = -1.0/wep.epsilon;
    wep.bornY = wep.epsilonT/epsilon2;
    wep.bornQ = wep.epsilonP/epsilon2*1e-05; // from 1/bar to 1/Pa
//	we.bornU = we.epsilonTP/epsilon2 - 2.0*we.bornY*we.bornQ*we.epsilon;
//	we.bornN = we.epsilonPP/epsilon2 - 2.0*we.bornQ*we.bornQ*we.epsilon;
    wep.bornX = wep.epsilonTT/epsilon2 - 2.0*wep.bornY*wep.bornY*wep.epsilon;

    return wep;
}

}
