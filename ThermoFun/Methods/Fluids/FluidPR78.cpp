// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

// The formulas of this file come from s_solmod2.cpp of the GEMS3K code, (c) T. Wagner, D. Kulik, S. Dmitrieva, F. Hingerl,
// S. Churakov, A. Rysin and others, under the GNU Lesser General Public License version 3 or later.

#include "Methods/Fluids/FluidPR78.h"
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/Constants.h>

namespace ThermoFun {

namespace {

const real R_CONST = 8.31451;

/// The roots of z3 + a2 z2 + a1 z + a0.
void cardano( real a2, real a1, real a0, real &z1, real &z2, real &z3 )
{
    real q, rc, q3, rc2, theta, ac, bc;

    q = (pow(a2,2.) - 3.*a1)/9.;
    rc = (2.*pow(a2,3.) - 9.*a2*a1 + 27.*a0)/54.;
    q3 = pow(q,3.);
    rc2 = pow(rc,2.);
    if (rc2 < q3)  // three real roots
    {
        theta = acos(rc/sqrt(q3));
        z1 = (-2.)*sqrt(q)*cos(theta/3.)-a2/3.;
        z2 = (-2.)*sqrt(q)*cos(theta/3.+2./3.*3.1415927)-a2/3.;
        z3 = (-2.)*sqrt(q)*cos(theta/3.-2./3.*3.1415927)-a2/3.;
    }
    else  // one real root
    {
        ac = (-1.)*rc/fabs(rc)*pow(fabs(rc)+sqrt(rc2-q3), 1./3.);
        if (ac != 0.)
            bc = q/ac;
        else
            bc = 0.;
        z1 = ac+bc-a2/3.;
        z2 = ac+bc-a2/3.;
        z3 = ac+bc-a2/3.;
    }
}

/// The attractive (a) and repulsive (b) parameters of the equation of state, and the T derivatives of a.
void ab(const real& Tk,  real Tcrit, real Pcrit, real omg, real /*N*/, real &apure, real &bpure, real &da, real &d2a )
{
    real Tred, k, alph, ac, sqa, dsqa, d2sqa;

    Tred = Tk/Tcrit;
    if (omg <= 0.491)
            k = 0.37464 + 1.54226*omg - 0.26992*pow(omg,2.);
    else
            k = 0.379642 + 1.48503*omg - 0.164423*pow(omg,2.) + 0.0166666*pow(omg,3.);

    alph = pow(1.+k*(1.-sqrt(Tred)),2.);
    ac = (0.457235529)*pow(R_CONST,2.)*pow(Tcrit,2.) / Pcrit;
    apure = alph*ac;
    bpure = (0.0777960739)*R_CONST*Tcrit/Pcrit;
    sqa = 1.+k*(1.-sqrt(Tred));
    dsqa = -0.5*k/(sqrt(Tred)*Tcrit);
    da = 2.*ac*(sqa*dsqa);
    d2sqa = 0.25*k/(pow(Tred,1.5)*pow(Tcrit,2.));
    d2a = 2.*ac*(dsqa*dsqa + sqa*d2sqa);

}

/// The fugacity and the residual functions of a pure fluid. eos: Tcrit, Pcrit, omg, N. pure: a, b, da/dT, d2a/dT2.
/// out: fugacity coefficient, G, H, S of the residual, volume, Cp of the residual.
void fugacityPure(const real& Tk, const real& Pbar, const real* eos, const real* pure, real* out)
{
    real Tcrit, Pcrit, Tred, apr, bpr, alph, da, d2a, k, A, B, a2, a1, a0,
            z1, z2, z3, vol1, vol2, vol3, lnf1, lnf2, lnf3, z, vol, lnf;
    real /*gig, hig, sig, cpig,*/ fugpure, grs, hrs, srs, cprs,
            cv, dPdT, dPdV, dVdT;

    // ideal gas changes from 1 bar to P at T of interest
    //hig = 0.;
    //sig = (-1.)*R_CONST*log(Pbar);
    //gig = hig - Tk*sig;
    //cpig = 0.;

    // retrieve a and b terms of cubic EoS
    Tcrit = eos[0];
    Pcrit = eos[1];
    Tred = Tk/Tcrit;
    apr = pure[0];
    bpr = pure[1];
    da = pure[2];
    d2a = pure[3];

    // solve cubic equation
    A = apr*Pbar/(pow(R_CONST,2.)*pow(Tk,2.));
    B = bpr*Pbar/(R_CONST*Tk);
    a2 = B - 1.;
    a1 = A - 3.*pow(B,2.) - 2.*B;
    a0 = pow(B,3.) + pow(B,2.) - A*B;
    cardano(a2, a1, a0, z1, z2, z3);

    // find stable roots
    vol1 = z1*R_CONST*Tk/Pbar;
    vol2 = z2*R_CONST*Tk/Pbar;
    vol3 = z3*R_CONST*Tk/Pbar;
    if (z1 > B)
        lnf1 = (-1.)*log(z1-B)
            - A/(B*sqrt(8.))*log((z1+(1.+sqrt(2.))*B)/(z1+(1.-sqrt(2.))*B))+z1-1.;
    else
        lnf1 = 1000.;
    if (z2 > B)
        lnf2 = (-1.)*log(z2-B)
            - A/(B*sqrt(8.))*log((z2+(1.+sqrt(2.))*B)/(z2+(1.-sqrt(2.))*B))+z2-1.;
    else
        lnf2 = 1000.;
    if (z3 > B)
        lnf3 = (-1.)*log(z3-B)
            - A/(B*sqrt(8.))*log((z3+(1.+sqrt(2.))*B)/(z3+(1.-sqrt(2.))*B))+z3-1.;
    else
        lnf3 = 1000.;

    if (lnf2 < lnf1)
    {
        z = z2; vol = vol2; lnf = lnf2;
    }
    else
    {
        z = z1; vol = vol1; lnf = lnf1;
    }
    if (lnf3 < lnf)
    {
        z = z3; vol = vol3; lnf = lnf3;
    }
    else
    {
        z = z; vol = vol; lnf = lnf;
    }

    // calculate thermodynamic properties
    alph = apr/((0.457235529)*pow(R_CONST,2.)*pow(Tcrit,2.) / Pcrit);
    k = (sqrt(alph)-1.)/(1.-sqrt(Tred));
    grs = R_CONST*Tk*(z-1.-log(z-B)-A/(B*sqrt(8.))
                *log((z+(1+sqrt(2.))*B)/(z+(1-sqrt(2.))*B)));
    hrs = R_CONST*Tk*(z-1.-log((z+(1+sqrt(2.))*B)/(z+(1-sqrt(2.))*B))
                *A/(B*sqrt(8.))*(1+k*sqrt(Tred)/sqrt(alph)));
    srs = (hrs-grs)/Tk;

    // heat capacity part
    cv = Tk*d2a/(bpr*sqrt(8.))
             * log( (z+B*(1.+sqrt(2.)))/(z+B*(1.-sqrt(2.))) );
    dPdT = R_CONST/(vol-bpr) - da/( vol*(vol+bpr) + bpr*(vol-bpr) );
    dPdV = - R_CONST*Tk/pow((vol-bpr),2.) + 2.*apr*(vol+bpr)/pow((vol*(vol+bpr)+bpr*(vol-bpr)),2.);
    dVdT = (-1.)*(1./dPdV)*dPdT;
    cprs = cv + Tk*dPdT*dVdT - R_CONST;

    // increment thermodynamic properties
    fugpure = exp(lnf);
    out[0] = fugpure;
    out[1] = grs;
    out[2] = hrs;
    out[3] = srs;
        out[4] = vol;
        out[5] = cprs;

}

/// The same with the check of the range: outside of it, the fluid is an ideal gas.
void fugacityProperties(const real& Tk, const real& Pbar, double Tmin, const real* eos, real* out)
{
    if ((Tk >= Tmin) && (Tk < 1e4) && (Pbar >= 1e-5) && (Pbar < 1e5))
    {
        const real Tcrit = eos[0];
        const real Pcrit = eos[1];
        const real omg = eos[2];
        const real N = eos[3];
        real apure, bpure, da, d2a;
        ab(Tk, Tcrit, Pcrit, omg, N, apure, bpure, da, d2a);
        const real pure[4] = {apure, bpure, da, d2a};
        fugacityPure(Tk, Pbar, eos, pure, out);
    }
    else
    {
        for (int i = 1; i < 6; i++)
            out[i] = 0.;
        out[0] = 1.;
        out[4] = 8.31451*Tk/Pbar;
    }
}

} // namespace

auto thermoPropertiesFluidPR78(real TK, real Pbar, const FluidInputs& subst, ThermoPropertiesSubstanceAD tps, bool cancel_fugacity) -> ThermoPropertiesSubstanceAD
{
    real FugProps[6];
    if (Pbar.val() == 0.0)
        Pbar += 1e-5;
    double TClow = subst.lower_T;
    real CPg[7];
    for (unsigned int i = 0; i < 7; i++)
    {
        CPg[i] = i < subst.parameters.size() ? subst.parameters[i] : real(0.0);
    }

    fugacityProperties(TK, Pbar, TClow, CPg, FugProps);

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (TK) * log( FugProps[0] );
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (Pbar);
    // back correction
    // the standard state has no fugacity coefficient: remove RT ln(phi) added above, unless the caller keeps it
    if (cancel_fugacity)
        tps.gibbs_energy -= R_CONSTANT * (TK) * log(Fug/Pbar);

    return tps;
}

}
