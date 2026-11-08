// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include <array>
// The formulas of this file come from s_solmod2.cpp of the GEMS3K code, (c) T. Wagner, D. Kulik, S. Dmitrieva, F. Hingerl,
// S. Churakov, A. Rysin and others, under the GNU Lesser General Public License version 3 or later.

#include "Methods/Fluids/FluidCORK.h"
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/Constants.h>

namespace ThermoFun {

namespace {

// The fluids that have an equation of their own; another fluid goes by corresponding states
constexpr char CEM_GAS_ = 'G', CEM_H2O_ = 'V', CEM_CO2_ = 'C', CEM_CH4_ = 'M', CEM_N2_ = 'T', CEM_H2_ = 'H', CEM_O2_ = 'O', CEM_AR_ = 'A', CEM_PO_ = 'P', CEM_NP_ = 'Q';

/// The places of the constants of the model: the order of the catalog, and of its file in Methods/ParameterSets.
enum Constant
{
    cs_a0, cs_a1, cs_b0, cs_c0, cs_c1, cs_d0, cs_d1, co2_a0, co2_a1, co2_a2, co2_b, co2_c0, co2_c1, co2_d0, co2_d1, co2_e0, co2_p0, h2o_a0, h2o_a1, h2o_a2, h2o_a3, h2o_a4, h2o_a5, h2o_a6, h2o_a7, h2o_a8, h2o_a9, h2o_b, h2o_c0, h2o_c1, h2o_d0, h2o_d1, h2o_e0, h2o_p0, h2o_psat0, h2o_psat2, h2o_psat3, h2o_psat5, h2o_T0, h2o_Tsuper
};

/// The state of one calculation. Fugpure: fugacity coefficient, G, H, S of the residual, volume, Cp of the residual.
struct Work
{
    real Tk, Pbar, R_CONST = R_CONSTANT, RR = R_CONSTANT/1000., Pkb;
    const double* k = nullptr;  ///< the constants of the model
    char EosCode = 'G';
    bool gems = false;  ///< the formulas of GEMS; else H = -T^2 d(G/T)/dT and Cp = dH/dT
    real Eosparm[2];
    real Fugpure[6];
    real Rho[11];
};


long int FugacityPT(Work& w, real *EoSparam);
long int FugacityCorresponding(Work& w);
long int FugacityCO2(Work& w);
long int VolumeFugacity(Work& w, long int phState, real pp, real p0, real a, real b, real c, real d, real e, real &vol, real &fc, real* vmrkOut = nullptr);

/// The residual H (J/mol) and Cp (J/(mol K)) of the MRK part at the MRK volume v (kJ/kbar), and dv/dT, dv/dP (kJ/kbar/K, kJ/kbar2).
struct MRK
{
    real h, cp, vT, vP;
};

auto mrkResidual(const Work& w, const real& a, const real& da, const real& d2a, const real& b, const real& p, const real& v) -> MRK
{
    const real z = p*v/(w.RR*w.Tk);
    MRK out;
    out.h = ((z-1.) - 1./(b*w.RR*w.Tk)*(sqrt(w.Tk)*da-(3./2.)*a/sqrt(w.Tk))*log(v/(v+b)))*w.R_CONST*w.Tk;
    const real cv = (-1./(b*w.RR)*(-da/sqrt(w.Tk)+sqrt(w.Tk)*d2a+(3.*a)/(4.*pow(w.Tk,1.5))))*log(v/(v+b))*w.R_CONST;
    const real dpt = w.RR/(v-b) - da/(sqrt(w.Tk)*v*(v+b)) + 0.5*a/(pow(w.Tk,1.5)*v*(v+b));
    const real dpv = - w.RR*w.Tk/pow(v-b,2.) + a/(pow(v,2.)*(v+b)*sqrt(w.Tk)) + a/(v*pow(v+b,2.)*sqrt(w.Tk));
    out.vT = -dpt/dpv;
    out.vP = 1./dpv;
    out.cp = cv + 1000.*w.Tk*dpt*out.vT - w.R_CONST;
    return out;
}

/// The virial term above p0 (kbar), in J/mol and J/(mol K): G = int V dP, with c = c0 + c1 T, d = d0 + d1 T and e constant. Its Cp is 0.
struct Virial
{
    real g = 0.0, s = 0.0, h = 0.0;
};

auto virialTerm(const Work& w, const real& c, const real& d, const real& e, const real& c1, const real& d1, const real& p, const real& p0) -> Virial
{
    Virial out;
    if (p > p0)
    {
        const real x = p - p0;
        out.g = 1000.*((2./3.)*c*x*sqrt(x) + d/2.*pow(x,2.) + (4./5.)*e*x*sqrt(sqrt(x)));
        out.s = -1000.*((2./3.)*c1*x*sqrt(x) + d1/2.*pow(x,2.));
        out.h = out.g + w.Tk*out.s;
    }
    return out;
}
long int Cardano(Work& w, real cb, real cc, real cd, real &v1, real &v2, real &v3);
long int FugacityH2O(Work& w);

long int FugacityPT(Work& w, real *EoSparam)
{
    long int iErr = 0;

    // reads EoS parameters from database into work array
    if( !EoSparam )
        return -1;  // Memory alloc error
    w.Eosparm[0] = EoSparam[0];  // critical temperature in K
    w.Eosparm[1] = EoSparam[1];  // critical pressure in bar

    // select subroutines for different fluid types
    switch ( w.EosCode )
    {
            // case DC_GAS_H2O_:  // H2O
            case CEM_H2O_:  // H2O
                    iErr = FugacityH2O(w);
                    break;
            // case DC_GAS_CO2_:  // CO2
            case CEM_CO2_: // CO2
                    iErr = FugacityCO2(w);
                    break;
            // case DC_GAS_COMP_:  // other fluids
            // case DC_GAS_H2_:
            // case DC_GAS_N2_:
            case CEM_GAS_:  // other fluids
            case CEM_CH4_:
            case CEM_N2_:
            case CEM_H2_:
            case CEM_O2_:
            case CEM_AR_:
            case CEM_PO_:
            case CEM_NP_:
                    iErr = FugacityCorresponding(w);
                    break;
            default:
                    iErr = 3;
                    break;
    }

    return iErr;
}

long int FugacityCorresponding(Work& w)
{
    real a0, a1, a, b0, b, c0, c1, c, d0, d1, d, tcr, pcr, da, dc, dd, vc, fc, rtlnf, rho;
    real grs, hrs, srs, cprs, drt, drtt, drp, drpp, drtp,
            dvt, dvtt, dvp, dvpp, dvtp;

    a0 = w.k[cs_a0];
    a1 = w.k[cs_a1];
    b0 = w.k[cs_b0];
    c0 = w.k[cs_c0];
    c1 = w.k[cs_c1];
    d0 = w.k[cs_d0];
    d1 = w.k[cs_d1];
    tcr = w.Eosparm[0];
    pcr = w.Eosparm[1]/1000.;  // kbar
    a = a0*pow(tcr,2.)*sqrt(tcr)/pcr + a1*tcr*sqrt(tcr)/pcr*w.Tk;
    b = b0*tcr/pcr;
    c = c0*tcr/(pcr*sqrt(pcr)) + c1/(pcr*sqrt(pcr))*w.Tk;
    d = d0*tcr/pow(pcr,2.) + d1/pow(pcr,2.)*w.Tk;
    da = a1*pow(tcr,1.5)/pcr;
    dc = c1/pow(pcr,1.5);
    dd = d1/pow(pcr,2.);

    // molar volume and fugacity coefficient
    vc = w.RR*w.Tk/w.Pkb + b - a*w.RR*sqrt(w.Tk)/((w.RR*w.Tk+b*w.Pkb)*(w.RR*w.Tk+2.*b*w.Pkb)) + c*sqrt(w.Pkb) + d*w.Pkb;
    rtlnf = w.RR*w.Tk*log(1000*w.Pkb) + b*w.Pkb + a/(b*sqrt(w.Tk))*(log(w.RR*w.Tk+b*w.Pkb) - log(w.RR*w.Tk+2*b*w.Pkb))
            + (2./3.)*c*w.Pkb*sqrt(w.Pkb) + (d/2.)*pow(w.Pkb,2.);
    fc = exp(rtlnf/(w.RR*w.Tk))/(1000*w.Pkb);

    // volume and density derivatives
    dvt = ( w.RR/w.Pkb - (da*w.RR*pow(w.Tk,0.5))/((w.RR*w.Tk+b*w.Pkb)*(w.RR*w.Tk+2.*b*w.Pkb))
            - (a*w.RR)/(2.*pow(w.Tk,0.5)*(w.RR*w.Tk+b*w.Pkb)*(w.RR*w.Tk+2.*b*w.Pkb))
            + (a*pow(w.RR,2.)*pow(w.Tk,0.5))/(pow((w.RR*w.Tk+b*w.Pkb),2.)*(w.RR*w.Tk+2.*b*w.Pkb))
            + (a*pow(w.RR,2.)*pow(w.Tk,0.5))/((w.RR*w.Tk+b*w.Pkb)*pow((w.RR*w.Tk+2.*b*w.Pkb),2.))
            + dc*pow(w.Pkb,0.5) + dd*w.Pkb );
    dvtt = ( - (da*w.RR)/(pow(w.Tk,0.5)*(w.RR*w.Tk+b*w.Pkb)*(w.RR*w.Tk+2.*b*w.Pkb))
            + (2.*da*pow(w.RR,2.)*pow(w.Tk,0.5))/(pow((w.RR*w.Tk+b*w.Pkb),2.)*(w.RR*w.Tk+2.*b*w.Pkb))
            + (2.*da*pow(w.RR,2.)*pow(w.Tk,0.5))/((w.RR*w.Tk+b*w.Pkb)*pow((w.RR*w.Tk+2.*b*w.Pkb),2.))
            + (a*w.RR)/(4.*pow(w.Tk,1.5)*(w.RR*w.Tk+b*w.Pkb)*(w.RR*w.Tk+2.*b*w.Pkb))
            + (a*pow(w.RR,2.))/(pow(w.Tk,0.5)*pow((w.RR*w.Tk+b*w.Pkb),2.)*(w.RR*w.Tk+2.*b*w.Pkb))
            + (a*pow(w.RR,2.))/(pow(w.Tk,0.5)*(w.RR*w.Tk+b*w.Pkb)*pow((w.RR*w.Tk+2.*b*w.Pkb),2.))
            - (2.*a*pow(w.RR,3.)*pow(w.Tk,0.5))/(pow((w.RR*w.Tk+b*w.Pkb),3.)*(w.RR*w.Tk+2.*b*w.Pkb))
            - (2.*a*pow(w.RR,3.)*pow(w.Tk,0.5))/(pow((w.RR*w.Tk+b*w.Pkb),2.)*pow((w.RR*w.Tk+2.*b*w.Pkb),2.))
            - (2.*a*pow(w.RR,3.)*pow(w.Tk,0.5))/((w.RR*w.Tk+b*w.Pkb)*pow((w.RR*w.Tk+2.*b*w.Pkb),3.)) );
    dvp = ( - w.RR*w.Tk/pow(w.Pkb,2.) + (a*w.RR*pow(w.Tk,0.5)*b)/(pow((w.RR*w.Tk+b*w.Pkb),2.)*(w.RR*w.Tk+2.*b*w.Pkb))
            + (2.*a*w.RR*pow(w.Tk,0.5)*b)/((w.RR*w.Tk+b*w.Pkb)*pow((w.RR*w.Tk+2.*b*w.Pkb),2.))
            + c/(2.*pow(w.Pkb,0.5)) + d ) / (1000.);
    dvpp = ( (2.*w.RR*w.Tk)/pow(w.Pkb,3.) - (2.*a*w.RR*pow(w.Tk,0.5)*pow(b,2.))/(pow((w.RR*w.Tk+b*w.Pkb),3.)*(w.RR*w.Tk+2.*b*w.Pkb))
            - (4.*a*w.RR*pow(w.Tk,0.5)*pow(b,2.))/(pow((w.RR*w.Tk+b*w.Pkb),2.)*pow((w.RR*w.Tk+2.*b*w.Pkb),2.))
            - (8.*a*w.RR*pow(w.Tk,0.5)*pow(b,2.))/((w.RR*w.Tk+b*w.Pkb)*pow((w.RR*w.Tk+2.*b*w.Pkb),3.))
            - c/(4.*pow(w.Pkb,1.5)) ) / (1.0e6);
    dvtp = ( - w.RR/pow(w.Pkb,2.) + (da*w.RR*pow(w.Tk,0.5)*b)/(pow((w.RR*w.Tk+b*w.Pkb),2.)*(w.RR*w.Tk+2.*b*w.Pkb))
            + (a*w.RR*b)/(2.*pow(w.Tk,0.5)*pow((w.RR*w.Tk+b*w.Pkb),2.)*(w.RR*w.Tk+2.*b*w.Pkb))
            - (2.*a*pow(w.RR,2.)*pow(w.Tk,0.5)*b)/(pow((w.RR*w.Tk+b*w.Pkb),3.)*(w.RR*w.Tk+2.*b*w.Pkb))
            - (3.*a*pow(w.RR,2.)*pow(w.Tk,0.5)*b)/(pow((w.RR*w.Tk+b*w.Pkb),2.)*pow((w.RR*w.Tk+2.*b*w.Pkb),2.))
            + (2.*da*w.RR*pow(w.Tk,0.5)*b)/((w.RR*w.Tk+b*w.Pkb)*pow((w.RR*w.Tk+2.*b*w.Pkb),2.))
            + (a*w.RR*b)/(pow(w.Tk,0.5)*(w.RR*w.Tk+b*w.Pkb)*pow((w.RR*w.Tk+2.*b*w.Pkb),2.))
            - (4.*a*pow(w.RR,2.)*pow(w.Tk,0.5)*b)/((w.RR*w.Tk+b*w.Pkb)*pow((w.RR*w.Tk+2.*b*w.Pkb),3.))
            + dc/(2.*pow(w.Pkb,2.)) + dd ) / (1000.);
    rho = (1./vc);
    drt = - dvt*pow(rho,2.);
    drp = - dvp*pow(rho,2.);
    drtt = - (dvtt*pow(rho,3.)-2.*pow(drt,2.))/rho;
    drpp = - (dvpp*pow(rho,3.)-2.*pow(drp,2.))/rho;
    drtp = - (dvtp*pow(rho,3.)-2.*drt*drp)/rho;

    // thermodynamic properties
    grs = w.R_CONST*w.Tk*log(fc);
    hrs = ( (pow(w.Tk,0.5)*da*log(w.RR*w.Tk+2.*b*w.Pkb))/b - (pow(w.Tk,0.5)*da*log(w.RR*w.Tk+b*w.Pkb))/b
            - (3.*a*log(w.RR*w.Tk+2.*b*w.Pkb))/(2.*pow(w.Tk,0.5)*b) + (3.*a*log(w.RR*w.Tk+b*w.Pkb))/(2.*pow(w.Tk,0.5)*b)
            - (pow(w.Tk,0.5)*a*w.RR)/(b*(w.RR*w.Tk+b*w.Pkb)) + (pow(w.Tk,0.5)*a*w.RR)/(b*(w.RR*w.Tk+2.*b*w.Pkb))
            - (2.*w.Tk*dc*pow(w.Pkb,1.5))/3. - (w.Tk*dd*pow(w.Pkb,2.))/2. + b*w.Pkb
            + (2.*c*pow(w.Pkb,1.5))/3. + (d*pow(w.Pkb,2.))/2. )*1000.;
    srs = (hrs-grs)/w.Tk;
    cprs = ( - (2.*pow(w.Tk,0.5)*da*w.RR)/(b*(w.RR*w.Tk+b*w.Pkb)) + (pow(w.Tk,0.5)*a*pow(w.RR,2.))/(b*pow((w.RR*w.Tk+b*w.Pkb),2.))
            + (2.*pow(w.Tk,0.5)*da*w.RR)/(b*(w.RR*w.Tk+2.*b*w.Pkb)) - (a*w.RR)/(pow(w.Tk,0.5)*b*(w.RR*w.Tk+2.*b*w.Pkb))
            - (da*log(w.RR*w.Tk+2.*b*w.Pkb))/(pow(w.Tk,0.5)*b) + (da*log(w.RR*w.Tk+b*w.Pkb))/(pow(w.Tk,0.5)*b)
            + (3.*a*log(w.RR*w.Tk+2.*b*w.Pkb))/(4.*pow(w.Tk,1.5)*b) - (3.*a*log(w.RR*w.Tk+b*w.Pkb))/(4.*pow(w.Tk,1.5)*b)
            - (pow(w.Tk,0.5)*a*pow(w.RR,2.))/(b*pow((w.RR*w.Tk+2.*b*w.Pkb),2.)) + (a*w.RR)/(pow(w.Tk,0.5)*b*(w.RR*w.Tk+b*w.Pkb)) )*1000.;

    // copy results
    w.Fugpure[0] = fc;     // fugacity coefficient
    w.Fugpure[1] = grs;
    w.Fugpure[2] = hrs;
    w.Fugpure[3] = srs;
    w.Fugpure[4] = vc;
    w.Fugpure[5] = cprs;
    w.Rho[0] = rho*(0.1);   // mol cm-3
    w.Rho[1] = drt*(0.1);   // mol cm-3 K-1
    w.Rho[2] = drtt*(0.1);  // mol cm-3 K-2
    w.Rho[3] = drp;         // mol cm-3 MPa-1
    w.Rho[4] = drpp*(10.);  // mol cm-3 MPa-2
    w.Rho[5] = drtp;        // mol cm-3 K-1 MPa-1
    w.Rho[6] = 1.0;         // MPa cm3 mol-1
    w.Rho[7] = 1.0;         // MPa cm6 mol-2
    w.Rho[8] = 1.0;         // MPa K-1
    w.Rho[9] = 1.0;         // MPa K-2
    w.Rho[10] = 1.0;        // MPa cm3 mol-1 K-1

    return 0;
}

long int FugacityCO2(Work& w)
{
    long int phState;
    real a0, a1, a2, a, b, c0, c1, c, d0, d1, d, e0, e, da, d2a, p0, vc, fc, rho, z;
    real grs, hrs, srs, cprs, cvrs, dpr, dprr, dpt, dptt, dprt,
            drt, drtt, drp, drpp, drtp, dpv, dvt;

    a0 = w.k[co2_a0];
    a1 = w.k[co2_a1];
    a2 = w.k[co2_a2];
    b = w.k[co2_b];
    c0 = w.k[co2_c0];
    c1 = w.k[co2_c1];
    d0 = w.k[co2_d0];
    d1 = w.k[co2_d1];
    e0 = w.k[co2_e0];
    a = a0 + a1*w.Tk + a2*pow(w.Tk,2.);
    da = a1 + 2.*a2*w.Tk;
    d2a = 2.*a2;
    c = c0 + c1*w.Tk;
    d = d0 + d1*w.Tk;
    e = e0;
    p0 = w.k[co2_p0];

    // molar volume and fugacity coefficient
    phState = 1;
    real vmrk = 0.0;
    VolumeFugacity(w, phState, w.Pkb, p0, a, b, c, d, e, vc, fc, &vmrk);

    // pressure and density derivatives (MRK part)
    rho = (1./vc);
    dpr = (w.RR*w.Tk)/(pow((1./rho-b),2.)*pow(rho,2.)) - a/((1./rho+b)*pow(w.Tk,0.5))
            - a/(rho*pow((1./rho+b),2.)*pow(w.Tk,0.5));
    dprr = (2.*w.RR*w.Tk)/(pow((1./rho-b),3.)*pow(rho,4.)) - (2.*w.RR*w.Tk)/(pow((1./rho-b),2.)*pow(rho,3.))
            - (2.*a)/(pow(rho,3.)*pow((1./rho+b),3.)*pow(w.Tk,0.5));
    dpt = w.RR/(1./rho-b) - (da*rho)/((1./rho+b)*pow(w.Tk,0.5)) + (0.5*a*rho)/((1./rho+b)*pow(w.Tk,1.5));
    dptt = - (d2a*rho)/((1./rho+b)*pow(w.Tk,0.5)) + (da*rho)/((1./rho+b)*pow(w.Tk,1.5))
            - (0.75*a*rho)/((1./rho+b)*pow(w.Tk,2.5));
    dprt = w.RR/(pow((1./rho-b),2.)*pow(rho,2.)) - da/((1./rho+b)*pow(w.Tk,0.5))
            - da/(rho*pow((1./rho+b),2.)*pow(w.Tk,0.5)) + (0.5*a)/((1./rho+b)*pow(w.Tk,1.5))
            + (0.5*a)/(rho*pow((1./rho+b),2.)*pow(w.Tk,1.5));
    drp = (1./dpr);
    drpp = (-1.)*dprr*pow(dpr,-3.);
    drt = (-1.)*(1./dpr)*dpt;
    drtt = (dpt - dptt + dpt*(dprt-dpr)*pow(dpr,-1.))*pow(dpr,-1.)
            + (dprt - dpt*dprr*pow(dpr,-1.))*pow(dpr,-2.)*dpt;
    drtp = - pow(dpr,-1.) - (dprt-dpr)*pow(dpr,-2.) + dprr*dpt*pow(dpr,-3.);

    // thermodynamic properties (MRK part)
    z = (w.Pkb*vc)/(w.RR*w.Tk);
    grs = w.R_CONST*w.Tk*log(fc);
    hrs = ( (z-1.) - 1./(b*w.RR*w.Tk)*(pow(w.Tk,0.5)*da-(3./2.)*a/pow(w.Tk,0.5))*log(vc/(vc+b)) )*w.R_CONST*w.Tk;
    srs = (hrs-grs)/w.Tk;
    cvrs = ( -1./(b*w.RR)*(-da/pow(w.Tk,0.5)+pow(w.Tk,0.5)*d2a+(3.*a)/(4.*pow(w.Tk,1.5))))*log(vc/(vc+b) )*w.R_CONST;
    dpv = (- w.RR*w.Tk/pow((vc-b),2.) + a/(pow(vc,2.)*(vc+b)*pow(w.Tk,0.5))
            + a/(vc*pow((vc+b),2.)*pow(w.Tk,0.5)))*(1000.);
    dvt = (-1.)*(1./dpv)*(dpt*1000.);
    cprs = cvrs + w.Tk*(dpt*1000.)*dvt - w.R_CONST;
    if (!w.gems)  // the MRK part at the MRK volume, and the virial term above p0, which GEMS left out of H, S and Cp
    {
        const auto mrk = mrkResidual(w, a, da, d2a, b, w.Pkb, vmrk);
        hrs = mrk.h + virialTerm(w, c, d, e, c1, d1, w.Pkb, p0).h;
        srs = (hrs-grs)/w.Tk;
        cprs = mrk.cp;
    }

    // copy results
    w.Fugpure[0] = fc;     // fugacity coefficient
    w.Fugpure[1] = grs;
    w.Fugpure[2] = hrs;
    w.Fugpure[3] = srs;
    w.Fugpure[4] = vc;
    w.Fugpure[5] = cprs;
    w.Rho[0] = rho*(0.1);       // mol cm-3
    w.Rho[1] = drt*(0.1);       // mol cm-3 K-1
    w.Rho[2] = drtt*(0.1);      // mol cm-3 K-2
    w.Rho[3] = drp*(0.001);     // mol cm-3 MPa-1
    w.Rho[4] = drpp*(0.00001);  // mol cm-3 MPa-2
    w.Rho[5] = drtp*(0.001);    // mol cm-3 K-1 MPa-1
    w.Rho[6] = dpr*(1000.);     // MPa cm3 mol-1
    w.Rho[7] = dprr*(10000.);   // MPa cm6 mol-2
    w.Rho[8] = dpt*(100.);      // MPa K-1
    w.Rho[9] = dptt*(100.);     // MPa K-2
    w.Rho[10] = dprt*(1000.);   // MPa cm3 mol-1 K-1

    return 0;
}

long int VolumeFugacity(Work& w, long int phState, real pp, real p0, real a, real b, real c, real d, real e, real &vol, real &fc, real* vmrkOut)
{
    real cb, cc, cd, v1, v2, v3, vmrk, vvir, lng, lnvir;

    cb = (-1.)*w.RR*w.Tk/pp;
    cc = (-1.)*(b*w.RR*w.Tk+pow(b,2.)*pp-a/sqrt(w.Tk))/pp;
    cd = (-1.)*a*b/(sqrt(w.Tk)*pp);
    Cardano(w, cb, cc, cd, v1, v2, v3);

    if ( phState == 1 )   // vapor root
    {
        if (v1>0.)
            vmrk = v1;
        else if (v2>0.)
            vmrk = v2;
        else
            vmrk = v3;
        if (v2>vmrk)
            vmrk = v2;
        if (v3>vmrk)
            vmrk=v3;
    }

    else   // liquid root
    {
        if (v1>0.)
            vmrk = v1;
        else if (v2>0.)
            vmrk = v2;
        else
            vmrk = v3;
        if ( (v2<vmrk) && (v2>0.) )
            vmrk = v2;
        if ( (v3<vmrk) && (v2>0.) )
            vmrk = v3;
    }

    // calculate fugacity coefficient
    lng = pp*vmrk/(w.RR*w.Tk) - 1. - log((vmrk-b)*pp/(w.RR*w.Tk)) - a/(b*w.RR*w.Tk*sqrt(w.Tk))*log(1.+b/vmrk);
    if (pp>p0)
    {
        vvir = c*sqrt(pp-p0) + d*(pp-p0) + e*sqrt(sqrt(pp-p0));
        lnvir = ((2./3.)*c*(pp-p0)*sqrt(pp-p0) + d/2.*pow((pp-p0),2.) + (4./5.)*e*(pp-p0)*sqrt(sqrt(pp-p0)))/(w.RR*w.Tk);
    }
    else
    {
        vvir = 0.;
        lnvir = 0.;
    }

    vol = vmrk + vvir;
    if (vmrkOut)
        *vmrkOut = vmrk;
    lng = lng + lnvir;
    fc = exp(lng);
    return 0;
}

long int Cardano(Work& w, real cb, real cc, real cd, real &v1, real &v2, real &v3)
{
    real co, cp, cq, cr, cp2, cq3;

    cp = (2.*pow(cb,3.)-9.*cb*cc+27.*cd)/54.;
    cq = (pow(cb,2.)-3.*cc)/9.;
    cp2 = pow(cp,2.);
    cq3 = pow(cq,3.);

    if ( (cp2-cq3)>0. )  // one real root
    {
        cr = -cp/fabs(cp)*pow(fabs(cp)+sqrt(cp2-cq3),1./3.);
        if (cr!=0)
        {
            v1 = cr + cq/cr - cb/3.;
            v2 = cr + cq/cr - cb/3.;
            v3 = cr + cq/cr - cb/3.;
        }
        else
        {
            v1 = -cb/3.;
            v2 = -cb/3.;
            v3 = -cb/3.;
        }
    }

    else  //three real roots
    {
        const double pi = w.gems ? 3.1415927 : 3.14159265358979323846;  // GEMS: 8 digits, and the roots are not exact
        co = atan(sqrt(1.-cp2/(cq3))/(cp/sqrt(cq3)));
        if (co<0.)
            co = co + pi;
        v1 = (-2.)*sqrt(cq)*cos(co/3.) - cb/3.;
        v2 = (-2.)*sqrt(cq)*cos((co+2.*pi)/3.) - cb/3.;
        v3 = (-2.)*sqrt(cq)*cos((co-2.*pi)/3.) - cb/3.;
    }

    return 0;
}

long int FugacityH2O(Work& w)
{
    long int phState;   // 1: vapor, 2: liquid
    real a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a, da, d2a, b, c0, c1, c,
            d0, d1, d, e0, e, /*dc, dd,*/ p0, Psat, vc, fc, v, g1, g2, g3, rho, z;
    real grs, hrs, srs, cprs, cvrs, dpr, dprr, dpt, dptt, dprt,
            drt, drtt, drp, drpp, drtp, dpv, dvt;

    a0 = w.k[h2o_a0];
    a1 = w.k[h2o_a1];
    a2 = w.k[h2o_a2];
    a3 = w.k[h2o_a3];
    a4 = w.k[h2o_a4];
    a5 = w.k[h2o_a5];
    a6 = w.k[h2o_a6];
    a7 = w.k[h2o_a7];
    a8 = w.k[h2o_a8];
    a9 = w.k[h2o_a9];
    b = w.k[h2o_b];
    c0 = w.k[h2o_c0];
    c1 = w.k[h2o_c1];
    d0 = w.k[h2o_d0];
    d1 = w.k[h2o_d1];
    e0 = w.k[h2o_e0];
    c = c0 + c1*w.Tk;
    d = d0 + d1*w.Tk;
    e = e0;
    //dc = c1;
    //dd = d1;
    p0 = w.k[h2o_p0];
    // the states that G is made of (implementation consistent): one, or vapour and liquid at Psat and liquid at P
    struct State { real a, da, d2a, v, p; };
    State states[3];
    int nstates = 0;
    real vm = 0.0;

    // molar volume and fugacity coefficient
    if (w.Tk > w.k[h2o_Tsuper])   // supercritical fluid phase
    {
        phState = 1;
        a = a0 + a4*(w.Tk-w.k[h2o_T0]) + a5*pow((w.Tk-w.k[h2o_T0]),2.) + a6*pow((w.Tk-w.k[h2o_T0]),3.);
        if (w.gems)  // the signs of a4 and a6 are not those of da/dT
        {
            da = -a4 - 2.*a5*(w.k[h2o_T0]-w.Tk) - 3.*a6*pow((w.k[h2o_T0]-w.Tk),2.);
            d2a = 2.*a5 + 6.*a6*(w.k[h2o_T0]-w.Tk);
        }
        else
        {
            da = a4 + 2.*a5*(w.Tk-w.k[h2o_T0]) + 3.*a6*pow((w.Tk-w.k[h2o_T0]),2.);
            d2a = 2.*a5 + 6.*a6*(w.Tk-w.k[h2o_T0]);
        }
        VolumeFugacity(w, phState, w.Pkb, p0, a, b, c, d, e, vc, fc, &vm);
        states[nstates++] = {a, da, d2a, vm, w.Pkb};
    }

    else   // subcritical region
    {
        Psat = w.k[h2o_psat0] + w.k[h2o_psat2]*pow(w.Tk,2.) - w.k[h2o_psat3]*pow(w.Tk,3.) + w.k[h2o_psat5]*pow(w.Tk,5.);
        if (w.Pkb < Psat)
        {
            phState = 1;   // vapor
            if (w.Tk < w.k[h2o_T0])
            {
                a = a0 + a7*(w.k[h2o_T0]-w.Tk) + a8*pow((w.k[h2o_T0]-w.Tk),2.) + a9*pow((w.k[h2o_T0]-w.Tk),3.);
                da = -a7 - 2.*a8*(w.k[h2o_T0]-w.Tk) - 3.*a9*pow((w.k[h2o_T0]-w.Tk),2.);
                d2a = 2.*a8 + 6.*a9*(w.k[h2o_T0]-w.Tk);
            }
            else if (w.gems)  // T0 <= T < Tsuper: (T0 - T), where HP91 (Eq. 6) has (T - T0); a jumps at Tsuper
            {
                a = a0 + a4*(w.k[h2o_T0]-w.Tk) + a5*pow((w.k[h2o_T0]-w.Tk),2.) + a6*pow((w.k[h2o_T0]-w.Tk),3.);
                da = -a4 - 2.*a5*(w.k[h2o_T0]-w.Tk) - 3.*a6*pow((w.k[h2o_T0]-w.Tk),2.);
                d2a = 2.*a5 + 6.*a6*(w.k[h2o_T0]-w.Tk);
            }
            else  // Holland and Powell (1991), Eq. 6 for T > 673 K, as Perple_X: the same a as above Tsuper
            {
                a = a0 + a4*(w.Tk-w.k[h2o_T0]) + a5*pow((w.Tk-w.k[h2o_T0]),2.) + a6*pow((w.Tk-w.k[h2o_T0]),3.);
                da = a4 + 2.*a5*(w.Tk-w.k[h2o_T0]) + 3.*a6*pow((w.Tk-w.k[h2o_T0]),2.);
                d2a = 2.*a5 + 6.*a6*(w.Tk-w.k[h2o_T0]);
            }
            VolumeFugacity(w, phState, w.Pkb, p0, a, b, c, d, e, vc, fc, &vm);
            states[nstates++] = {a, da, d2a, vm, w.Pkb};
        }

        else
        {
            phState = 1;   // gaseous phase at Psat
            if (w.Tk < w.k[h2o_T0])
            {
                a = a0 + a7*(w.k[h2o_T0]-w.Tk) + a8*pow((w.k[h2o_T0]-w.Tk),2.) + a9*pow((w.k[h2o_T0]-w.Tk),3.);
                da = -a7 - 2.*a8*(w.k[h2o_T0]-w.Tk) - 3.*a9*pow((w.k[h2o_T0]-w.Tk),2.);
                d2a = 2.*a8 + 6.*a9*(w.k[h2o_T0]-w.Tk);
            }
            else if (w.gems)  // T0 <= T < Tsuper: (T0 - T), where HP91 (Eq. 6) has (T - T0); a jumps at Tsuper
            {
                a = a0 + a4*(w.k[h2o_T0]-w.Tk) + a5*pow((w.k[h2o_T0]-w.Tk),2.) + a6*pow((w.k[h2o_T0]-w.Tk),3.);
                da = -a4 - 2.*a5*(w.k[h2o_T0]-w.Tk) - 3.*a6*pow((w.k[h2o_T0]-w.Tk),2.);
                d2a = 2.*a5 + 6.*a6*(w.k[h2o_T0]-w.Tk);
            }
            else  // Holland and Powell (1991), Eq. 6 for T > 673 K, as Perple_X: the same a as above Tsuper
            {
                a = a0 + a4*(w.Tk-w.k[h2o_T0]) + a5*pow((w.Tk-w.k[h2o_T0]),2.) + a6*pow((w.Tk-w.k[h2o_T0]),3.);
                da = a4 + 2.*a5*(w.Tk-w.k[h2o_T0]) + 3.*a6*pow((w.Tk-w.k[h2o_T0]),2.);
                d2a = 2.*a5 + 6.*a6*(w.Tk-w.k[h2o_T0]);
            }
            VolumeFugacity(w, phState, Psat, p0, a, b, c, d, e, v, g1, &vm);
            states[nstates++] = {a, da, d2a, vm, Psat};

            phState = 2; // liquid phase at Psat
            if (w.Tk < w.k[h2o_T0])
            {
                a = a0 + a1*(w.k[h2o_T0]-w.Tk) + a2*pow((w.k[h2o_T0]-w.Tk),2.) + a3*pow((w.k[h2o_T0]-w.Tk),3.);
                da = -a1 - 2.*a2*(w.k[h2o_T0]-w.Tk) - 3.*a3*pow((w.k[h2o_T0]-w.Tk),2.);
                d2a = 2.*a2 + 6.*a3*(w.k[h2o_T0]-w.Tk);
            }
            else if (w.gems)  // T0 <= T < Tsuper: (T0 - T), where HP91 (Eq. 6) has (T - T0); a jumps at Tsuper
            {
                a = a0 + a4*(w.k[h2o_T0]-w.Tk) + a5*pow((w.k[h2o_T0]-w.Tk),2.) + a6*pow((w.k[h2o_T0]-w.Tk),3.);
                da = -a4 - 2.*a5*(w.k[h2o_T0]-w.Tk) - 3.*a6*pow((w.k[h2o_T0]-w.Tk),2.);
                d2a = 2.*a5 + 6.*a6*(w.k[h2o_T0]-w.Tk);
            }
            else  // Holland and Powell (1991), Eq. 6 for T > 673 K, as Perple_X: the same a as above Tsuper
            {
                a = a0 + a4*(w.Tk-w.k[h2o_T0]) + a5*pow((w.Tk-w.k[h2o_T0]),2.) + a6*pow((w.Tk-w.k[h2o_T0]),3.);
                da = a4 + 2.*a5*(w.Tk-w.k[h2o_T0]) + 3.*a6*pow((w.Tk-w.k[h2o_T0]),2.);
                d2a = 2.*a5 + 6.*a6*(w.Tk-w.k[h2o_T0]);
            }
            VolumeFugacity(w, phState, Psat, p0, a, b, c, d, e, v, g2, &vm);
            states[nstates++] = {a, da, d2a, vm, Psat};

            phState = 2; // fluid phase at w.Pkb
            if (w.Tk < w.k[h2o_T0])
            {
                a = a0 + a1*(w.k[h2o_T0]-w.Tk) + a2*pow((w.k[h2o_T0]-w.Tk),2.) + a3*pow((w.k[h2o_T0]-w.Tk),3.);
                da = -a1 - 2.*a2*(w.k[h2o_T0]-w.Tk) - 3.*a3*pow((w.k[h2o_T0]-w.Tk),2.);
                d2a = 2.*a2 + 6.*a3*(w.k[h2o_T0]-w.Tk);
            }
            else if (w.gems)  // T0 <= T < Tsuper: (T0 - T), where HP91 (Eq. 6) has (T - T0); a jumps at Tsuper
            {
                a = a0 + a4*(w.k[h2o_T0]-w.Tk) + a5*pow((w.k[h2o_T0]-w.Tk),2.) + a6*pow((w.k[h2o_T0]-w.Tk),3.);
                da = -a4 - 2.*a5*(w.k[h2o_T0]-w.Tk) - 3.*a6*pow((w.k[h2o_T0]-w.Tk),2.);
                d2a = 2.*a5 + 6.*a6*(w.k[h2o_T0]-w.Tk);
            }
            else  // Holland and Powell (1991), Eq. 6 for T > 673 K, as Perple_X: the same a as above Tsuper
            {
                a = a0 + a4*(w.Tk-w.k[h2o_T0]) + a5*pow((w.Tk-w.k[h2o_T0]),2.) + a6*pow((w.Tk-w.k[h2o_T0]),3.);
                da = a4 + 2.*a5*(w.Tk-w.k[h2o_T0]) + 3.*a6*pow((w.Tk-w.k[h2o_T0]),2.);
                d2a = 2.*a5 + 6.*a6*(w.Tk-w.k[h2o_T0]);
            }
            VolumeFugacity(w, phState, w.Pkb, p0, a, b, c, d, e, vc, g3, &vm);  // corrected from (phState, w.Pkb, psat ...)
            states[nstates++] = {a, da, d2a, vm, w.Pkb};
            fc = g1/g2*g3;  // this formula needs checking
        }
    }

    // pressure and density derivatives (MRK part)
    rho = (1./vc);
    dpr = (w.RR*w.Tk)/(pow((1./rho-b),2.)*pow(rho,2.)) - a/((1./rho+b)*pow(w.Tk,0.5))
            - a/(rho*pow((1./rho+b),2.)*pow(w.Tk,0.5));
    dprr = (2.*w.RR*w.Tk)/(pow((1./rho-b),3.)*pow(rho,4.)) - (2.*w.RR*w.Tk)/(pow((1./rho-b),2.)*pow(rho,3.))
            - (2.*a)/(pow(rho,3.)*pow((1./rho+b),3.)*pow(w.Tk,0.5));
    dpt = w.RR/(1./rho-b) - (da*rho)/((1./rho+b)*pow(w.Tk,0.5)) + (0.5*a*rho)/((1./rho+b)*pow(w.Tk,1.5));
    dptt = - (d2a*rho)/((1./rho+b)*pow(w.Tk,0.5)) + (da*rho)/((1./rho+b)*pow(w.Tk,1.5))
            - (0.75*a*rho)/((1./rho+b)*pow(w.Tk,2.5));
    dprt = w.RR/(pow((1./rho-b),2.)*pow(rho,2.)) - da/((1./rho+b)*pow(w.Tk,0.5))
            - da/(rho*pow((1./rho+b),2.)*pow(w.Tk,0.5)) + (0.5*a)/((1./rho+b)*pow(w.Tk,1.5))
            + (0.5*a)/(rho*pow((1./rho+b),2.)*pow(w.Tk,1.5));
    drp = (1./dpr);
    drpp = (-1.)*dprr*pow(dpr,-3.);
    drt = (-1.)*(1./dpr)*dpt;
    drtt = (dpt - dptt + dpt*(dprt-dpr)*pow(dpr,-1.))*pow(dpr,-1.)
            + (dprt - dpt*dprr*pow(dpr,-1.))*pow(dpr,-2.)*dpt;
    drtp = - pow(dpr,-1.) - (dprt-dpr)*pow(dpr,-2.) + dprr*dpt*pow(dpr,-3.);

    // thermodynamic properties (MRK part)
    z = (w.Pkb*vc)/(w.RR*w.Tk);
    grs = w.R_CONST*w.Tk*log(fc);
    hrs = ( (z-1.) - 1./(b*w.RR*w.Tk)*(pow(w.Tk,0.5)*da-(3./2.)*a/pow(w.Tk,0.5))*log(vc/(vc+b)) )*w.R_CONST*w.Tk;
    srs = (hrs-grs)/w.Tk;
    cvrs = ( -1./(b*w.RR)*(-da/pow(w.Tk,0.5)+pow(w.Tk,0.5)*d2a+(3.*a)/(4.*pow(w.Tk,1.5))))*log(vc/(vc+b) )*w.R_CONST;
    dpv = (- w.RR*w.Tk/pow((vc-b),2.) + a/(pow(vc,2.)*(vc+b)*pow(w.Tk,0.5))
            + a/(vc*pow((vc+b),2.)*pow(w.Tk,0.5)))*(1000.);
    dvt = (-1.)*(1./dpv)*(dpt*1000.);
    cprs = cvrs + w.Tk*(dpt*1000.)*dvt - w.R_CONST;
    if (!w.gems)
    {
        // G = G(vapour, Psat) - G(liquid, Psat) + G(liquid, P): H and Cp of each MRK part at its MRK volume, the virial term at P,
        // and the terms of dPsat/dT. GEMS took H and Cp of the last part only, at the volume with the virial term.
        const auto virial = virialTerm(w, c, d, e, c1, d1, w.Pkb, p0);
        if (nstates == 1)
        {
            const auto m = mrkResidual(w, states[0].a, states[0].da, states[0].d2a, b, states[0].p, states[0].v);
            hrs = m.h + virial.h;
            cprs = m.cp;
        }
        else
        {
            const auto m1 = mrkResidual(w, states[0].a, states[0].da, states[0].d2a, b, states[0].p, states[0].v);
            const auto m2 = mrkResidual(w, states[1].a, states[1].da, states[1].d2a, b, states[1].p, states[1].v);
            const auto m3 = mrkResidual(w, states[2].a, states[2].da, states[2].d2a, b, states[2].p, states[2].v);
            const real dPs = 2.*w.k[h2o_psat2]*w.Tk - 3.*w.k[h2o_psat3]*pow(w.Tk,2.) + 5.*w.k[h2o_psat5]*pow(w.Tk,4.);   // kbar/K
            const real d2Ps = 2.*w.k[h2o_psat2] - 6.*w.k[h2o_psat3]*w.Tk + 20.*w.k[h2o_psat5]*pow(w.Tk,3.);
            const real dV = states[0].v - states[1].v;   // kJ/kbar
            hrs = m1.h - m2.h + m3.h + virial.h - 1000.*w.Tk*dV*dPs;
            cprs = m1.cp - m2.cp + m3.cp
                   - 1000.*w.Tk*(2.*dPs*(m1.vT - m2.vT) + pow(dPs,2.)*(m1.vP - m2.vP) + dV*d2Ps);
        }
        srs = (hrs-grs)/w.Tk;
    }

    // copy results
    w.Fugpure[0] = fc;  // fugacity coefficient
    w.Fugpure[1] = grs;
    w.Fugpure[2] = hrs;
    w.Fugpure[3] = srs;
    w.Fugpure[4] = vc;
    w.Fugpure[5] = cprs;
    w.Rho[0] = rho*(0.1);       // mol cm-3
    w.Rho[1] = drt*(0.1);       // mol cm-3 K-1
    w.Rho[2] = drtt*(0.1);      // mol cm-3 K-2
    w.Rho[3] = drp*(0.001);     // mol cm-3 MPa-1
    w.Rho[4] = drpp*(0.00001);  // mol cm-3 MPa-2
    w.Rho[5] = drtp*(0.001);    // mol cm-3 K-1 MPa-1
    w.Rho[6] = dpr*(1000.);     // MPa cm3 mol-1
    w.Rho[7] = dprr*(10000.);   // MPa cm6 mol-2
    w.Rho[8] = dpt*(100.);      // MPa K-1
    w.Rho[9] = dptt*(100.);     // MPa K-2
    w.Rho[10] = dprt*(1000.);   // MPa cm3 mol-1 K-1

    return 0;
}

/// The fugacity and the residual functions of a pure fluid. Outside the range of the model, the fluid is an ideal gas.
void fugacityProperties(const real& Tk, const real& Pbar, double Tmin, char code, real* parameters, const double* constants, real* out, bool gems)
{
    if ((Tk >= Tmin) && (Tk < 1e4) && (Pbar >= 1e-5) && (Pbar < 1e5))
    {
        Work w;
        w.k = constants;
        w.Tk = Tk;
        w.Pbar = Pbar;
        w.Pkb = Pbar/1000.;
        w.EosCode = code;
        w.gems = gems;
        FugacityPT(w, parameters);
        for (int i = 0; i < 6; i++)
            out[i] = w.Fugpure[i];
    }
    else
    {
        for (int i = 1; i < 6; i++)
            out[i] = 0.;
        out[0] = 1.;
        out[4] = R_CONSTANT*Tk/Pbar;
    }
}

} // namespace

auto thermoPropertiesFluidCORK(real TK, real Pbar, const FluidInputs& subst, ThermoPropertiesSubstanceAD tps, bool cancel_fugacity) -> ThermoPropertiesSubstanceAD
{
    real FugProps[6];
    char Eos_Code = 'G';
    if (Pbar.val() == 0.0)
        Pbar += 1e-5;

    if (subst.formula == "CO2") Eos_Code = 'C';
    if (subst.formula == "H2O") Eos_Code = 'V';
    double TClow = subst.lower_T;
    std::array<real, 7> CPg;
    for (unsigned int i = 0; i < 7; i++)
    {
        CPg[i] = i < subst.parameters.size() ? subst.parameters[i] : real(0.0);
    }

    fugacityProperties(TK, Pbar, TClow, Eos_Code, CPg.data(), subst.constants.data(), FugProps, subst.gems);

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (TK) * log( FugProps[0] ); // from ideal gas at 1 bar and givent T to pure real gas/fluid at T and P
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    if (!subst.gems)
        tps.heat_capacity_cp += FugProps[5];  // the residual Cp (GEMS did not add it)
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (Pbar); //FugProps[0] - fugacity coefficient
    // the standard state has no fugacity coefficient: remove RT ln(phi) added above, unless the caller keeps it
    if (cancel_fugacity)
        tps.gibbs_energy -= R_CONSTANT * (TK) * log(Fug/Pbar);

    return tps;
}
}
