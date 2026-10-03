#include "WaterElectroFernandez1997.h"
#include "ThermoEngine.h"
#include "Database.h"
#include "Substance.h"
#include "ThermoProperties.h"
#include "Substances/Solvent/SolventDensityDerivatives.hpp"

namespace ThermoFun {

const double I[] = {1, 1, 1, 2, 3, 3, 4, 5, 6, 7, 10};
const double J[] = {0.25, 1, 2.5, 1.5, 1.5, 2.5, 2, 2, 5, 0.5, 10};
const double n[] = {0.978224486826, -0.957771379375, 0.237511794148, 0.714692244396,
     -0.298217036956, -0.108863472196, .949327488264e-1, -.980469816509e-2,
     .165167634970e-4, .937359795772e-4, -.12317921872e-9, .196096504426e-2};
const double Tc = 647.096;     // K
const double Pc = 22.064;      // MPa
const double rhoc = 322.;      // kg/m³



/// The dielectric constant of water of Fernandez et al. (1997) and its partial derivatives with respect to T (K) and the density
/// (kg/m3), analytical:
///   g = 1 + n11 r w^-1.2 + sum n_i r^I_i (Tc/T)^J_i,  r = rho/rhoc, w = T/228 - 1
///   A = cA rho g/T,  B = cB rho,  eps = (1 + A + 5B + S)/(4 (1 - B)),  S = sqrt(9 + 2A + 18B + A^2 + 10AB + 9B^2)
/// the derivatives of eps with respect to A and B follow from those of S, and the partial derivatives with respect to (T, rho) by the chain rule.
struct EpsilonF
{
    real eps, T, rho, TT, Trho, rhorho;
};

static auto epsilonF(const real& T, const real& rho) -> EpsilonF
{
    const double k        = 1.380658e-23;
    const double Na       = 6.0221367e23;
    const double alfa     = 1.636e-40;
    const double epsilon0 = 8.854187817e-12;
    const double mu       = 6.138e-30;
    const double M        = 0.018015268;
    const double cA = Na*mu*mu/M/epsilon0/k;
    const double cB = Na*alfa/3/M/epsilon0;

    const real r = rho/rhoc;
    const real w = T/228 - 1;
    const real w12 = pow(w, -1.2);

    // g and its partial derivatives (power laws)
    real g = 1 + n[11]*r*w12;
    real g_rho = n[11]*w12/rhoc;
    real g_rr = 0.0*r;
    real g_T = n[11]*r*(-1.2)*pow(w, -2.2)/228;
    real g_TT = n[11]*r*(1.2*2.2)*pow(w, -3.2)/(228.0*228.0);
    real g_rT = n[11]*(-1.2)*pow(w, -2.2)/228/rhoc;
    for (unsigned i = 0; i < 11; i++)
    {
        const real rI = pow(r, I[i]);
        const real tJ = pow(Tc/T, J[i]);
        g     += n[i]*rI*tJ;
        g_rho += n[i]*I[i]*pow(r, I[i] - 1)*tJ/rhoc;
        g_rr  += n[i]*I[i]*(I[i] - 1)*pow(r, I[i] - 2)*tJ/(rhoc*rhoc);
        g_T   += n[i]*rI*tJ*(-J[i]/T);
        g_TT  += n[i]*rI*tJ*J[i]*(J[i] + 1)/(T*T);
        g_rT  += n[i]*I[i]*pow(r, I[i] - 1)*tJ*(-J[i]/T)/rhoc;
    }

    // A = cA rho g/T and B = cB rho
    const real A = cA*rho*g/T;
    const real A_rho = cA*(g + rho*g_rho)/T;
    const real A_T = cA*rho*(g_T/T - g/(T*T));
    const real A_rr = cA*(2*g_rho + rho*g_rr)/T;
    const real A_rT = cA*((g_T + rho*g_rT)/T - (g + rho*g_rho)/(T*T));
    const real A_TT = cA*rho*(g_TT/T - 2*g_T/(T*T) + 2*g/(T*T*T));
    const real B = cB*rho;
    const double B_rho = cB;

    // S = sqrt(Q) and eps = N u, N = 1 + A + 5B + S, u = 1/(4 (1 - B))
    const real Q = 9 + 2*A + 18*B + A*A + 10*A*B + 9*B*B;
    const real S = sqrt(Q);
    const real a1 = 1 + A + 5*B, b1 = 9 + 5*A + 9*B;
    const real S_A = a1/S, S_B = b1/S;
    const real S_AA = 1/S - a1*a1/(S*S*S);
    const real S_AB = 5/S - a1*b1/(S*S*S);
    const real S_BB = 9/S - b1*b1/(S*S*S);
    const real N = a1 + S;
    const real N_A = 1 + S_A, N_B = 5 + S_B;
    const real u = 1/(4*(1 - B));
    const real u_B = u/(1 - B), u_BB = 2*u/((1 - B)*(1 - B));

    const real eps = N*u;
    const real eps_A = N_A*u;
    const real eps_B = N_B*u + N*u_B;
    const real eps_AA = S_AA*u;
    const real eps_AB = S_AB*u + N_A*u_B;
    const real eps_BB = S_BB*u + 2*N_B*u_B + N*u_BB;

    EpsilonF e;
    e.eps = eps;
    e.T = eps_A*A_T;
    e.rho = eps_A*A_rho + eps_B*B_rho;
    e.TT = eps_AA*A_T*A_T + eps_A*A_TT;
    e.Trho = eps_AA*A_rho*A_T + eps_AB*B_rho*A_T + eps_A*A_rT;
    e.rhorho = eps_AA*A_rho*A_rho + 2*eps_AB*A_rho*B_rho + eps_BB*B_rho*B_rho + eps_A*A_rr;
    return e;
}


auto electroPropertiesWaterFernandez1997(const Reaktoro_::Pass& pass, Substance substance, int state) -> ElectroPropertiesSolventAD
{
    ElectroPropertiesSolventAD wep;

    Database db; db.addSubstance(substance);
    ThermoEngine   th(db);

    double P1 = pass.P.val();  // propertiesSolvent takes the pressure by reference
    const auto psol = th.propertiesSolvent(pass.T.val(), P1, substance.symbol(), state);
    const auto r = densityDerivatives(pass, psol);

    // the dielectric constant and its total derivatives with respect to T and P (Pa): the chain rule with the derivatives of the density
    const auto e = epsilonF(pass.T, r.rho);
    const real eps_T = e.T + e.rho*r.T;
    const real eps_P = e.rho*r.P;
    const real eps_TT = e.TT + 2*e.Trho*r.T + e.rhorho*r.T*r.T + e.rho*r.TT;
    const real eps_TP = e.Trho*r.P + e.rhorho*r.T*r.P + e.rho*r.TP;
    const real eps_PP = e.rhorho*r.P*r.P + e.rho*r.PP;

    const real eps = e.eps;
    const real epsilon2 = eps * eps;

    // the pressure derivatives per bar (the derivatives with respect to P in Pa multiplied by bar_to_Pa)
    wep.epsilon   = eps;
    wep.epsilonT  = eps_T;
    wep.epsilonP  = eps_P*bar_to_Pa;
    wep.epsilonTT = eps_TT;
    wep.epsilonTP = eps_TP*bar_to_Pa;
    wep.epsilonPP = eps_PP*bar_to_Pa*bar_to_Pa;
    wep.bornZ = -1.0/wep.epsilon;
    wep.bornY = wep.epsilonT/epsilon2;
    wep.bornQ = wep.epsilonP/epsilon2*1e-05; // from 1/bar to 1/Pa
//	we.bornU = we.epsilonTP/epsilon2 - 2.0*we.bornY*we.bornQ*we.epsilon;
//	we.bornN = we.epsilonPP/epsilon2 - 2.0*we.bornQ*we.bornQ*we.epsilon;
    wep.bornX = wep.epsilonTT/epsilon2 - 2.0*wep.bornY*wep.bornY*wep.epsilon;

    return wep;
}


}
