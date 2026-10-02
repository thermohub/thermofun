#include "WaterElectroFernandez1997.h"
#include "ThermoEngine.h"
#include "Database.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "Common/Jets.hpp"

namespace ThermoFun {

const double I[] = {1, 1, 1, 2, 3, 3, 4, 5, 6, 7, 10};
const double J[] = {0.25, 1, 2.5, 1.5, 1.5, 2.5, 2, 2, 5, 0.5, 10};
const double n[] = {0.978224486826, -0.957771379375, 0.237511794148, 0.714692244396,
     -0.298217036956, -0.108863472196, .949327488264e-1, -.980469816509e-2,
     .165167634970e-4, .937359795772e-4, -.12317921872e-9, .196096504426e-2};
const double Tc = 647.096;     // K
const double Pc = 22.064;      // MPa
const double rhoc = 322.;      // kg/m³



/// The dielectric constant as a function of the temperature (K) and of the density (kg/m3)
template<class Number>
auto epsilonF (Number T, Number rho) -> Number
{
    auto k        = 1.380658e-23;
    auto Na       = 6.0221367e23;
    auto alfa     = 1.636e-40;
    auto epsilon0 = 8.854187817e-12;
    auto mu       = 6.138e-30;
    auto M        = 0.018015268;

    Number g = 1+n[11]*(rho/rhoc)/(pow((Tc/228/(Tc/T)-1), 1.2));
    for (unsigned i=0; i<11; i++)
    {
        g += n[i]*pow((rho/rhoc),I[i])*pow((Tc/T),J[i]);
    }

    const Number A = Na*(mu*mu)*rho*g/M/epsilon0/k/T;
    const Number B = Na*alfa*rho/3/M/epsilon0;
    const Number eps = (1+A+5*B+pow((9+2*A+18*B+pow(A,2)+10*A*B+9*pow(B,2)),0.5))/4/(1-B);

    return eps;
}


auto electroPropertiesWaterFernandez1997(const Reaktoro_::Pass& pass, Substance substance, int state) -> ElectroPropertiesSolventAD
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
        const jets::Dual density = jets::taylor(rho, T - T0, P - P0);   // kg/m3
        return epsilonF<jets::Dual>(T, density);                         // T in K
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
