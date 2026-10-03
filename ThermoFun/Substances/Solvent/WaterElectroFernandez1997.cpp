#include "WaterElectroFernandez1997.h"
#include "ThermoEngine.h"
#include "Database.h"
#include "Substance.h"
#include "ThermoProperties.h"

namespace ThermoFun {

const double I[] = {1, 1, 1, 2, 3, 3, 4, 5, 6, 7, 10};
const double J[] = {0.25, 1, 2.5, 1.5, 1.5, 2.5, 2, 2, 5, 0.5, 10};
const double n[] = {0.978224486826, -0.957771379375, 0.237511794148, 0.714692244396,
     -0.298217036956, -0.108863472196, .949327488264e-1, -.980469816509e-2,
     .165167634970e-4, .937359795772e-4, -.12317921872e-9, .196096504426e-2};
const double Tc = 647.096;     // K
const double Pc = 22.064;      // MPa
const double rhoc = 322.;      // kg/m³



auto epsilonF (real T, real rho) -> real
{
    auto k        = 1.380658e-23;
    auto Na       = 6.0221367e23;
    auto alfa     = 1.636e-40;
    auto epsilon0 = 8.854187817e-12;
    auto mu       = 6.138e-30;
    auto M        = 0.018015268;

    real eps;

    auto g = 1+n[11]*(rho/rhoc)/(pow((Tc/228/(Tc/T)-1), 1.2));
    for (unsigned i=0; i<11; i++)
    {
        g += n[i]*pow((rho/rhoc),I[i])*pow((Tc/T),J[i]);
    }

    auto A = Na*pow(mu,2)*rho*g/M/epsilon0/k/T;
    auto B = Na*alfa*rho/3/M/epsilon0;
    eps    = (1+A+5*B+pow((9+2*A+18*B+pow(A,2)+10*A*B+9*pow(B,2)),0.5))/4/(1-B);

    return eps;
}


auto electroPropertiesWaterFernandez1997(const Reaktoro_::Pass& pass, Substance substance, int state) -> ElectroPropertiesSolventAD
{
    ElectroPropertiesSolventAD wep;

    Database db; db.addSubstance(substance);
    ThermoEngine   th(db);

    const real TC   = pass.T - 273.15;      // temperature in celsius
    const real Pbar = pass.P / 1e05;        // pressure in bar

    real TK = TC + 273.15;
    double P1 = pass.P.val();  // propertiesSolvent takes the pressure by reference
    auto psol = th.propertiesSolvent(TK.val(), P1, substance.symbol(), state);
    const real RHO = pass(psol.density);

    const auto eps = epsilonF(TK, RHO);
    const auto epsilon2 = eps * eps;

    // numerical approximation of epsilonT and epsilonTT
    real T_plus = TC + TC.val()*0.001;
    real TK_plus = T_plus + 273.15;
    double P2 = pass.P.val();  // propertiesSolvent takes the pressure by reference
    real RHO_plus = pass(th.propertiesSolvent(TK_plus.val(), P2, substance.symbol(), state).density);
    auto eps_plus = epsilonF(TK_plus, RHO_plus);

    real T_minus = TC - TC.val()*0.001;
    real TK_minus = T_minus + 273.15;
    double P3 = pass.P.val();  // propertiesSolvent takes the pressure by reference
    real RHO_minus = pass(th.propertiesSolvent(TK_minus.val(), P3, substance.symbol(), state).density);
    auto eps_minus = epsilonF(TK_minus, RHO_minus);

    const auto epsilonT  = (eps_plus - eps_minus) / ((TK_plus-TK_minus));
    const auto epsilonTT = (eps_plus + eps_minus - 2*eps)/pow(((TK_plus-TK_minus)*0.5),2);

    // numerical approximation of epsilonP and epsilonPP
    real P_plus = Pbar + Pbar.val()*0.001;
    double P4 = P_plus.val()*1e05;  // propertiesSolvent takes the pressure by reference
    RHO_plus = pass(th.propertiesSolvent(TK.val(), P4, substance.symbol(), state).density);
    eps_plus = epsilonF(TK, RHO_plus);

    real P_minus = Pbar - Pbar.val()*0.001;
    double P5 = P_minus.val()*1e05;  // propertiesSolvent takes the pressure by reference
    RHO_minus = pass(th.propertiesSolvent(TK.val(), P5, substance.symbol(), state).density);
    eps_minus = epsilonF(TK, RHO_minus);

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
