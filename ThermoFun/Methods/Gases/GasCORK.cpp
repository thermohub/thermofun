// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#include <array>
#include "Methods/Gases/GasCORK.h"
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/Constants.h>

namespace ThermoFun {

namespace {

// The fluids that have an equation of their own; another fluid goes by corresponding states
constexpr char CEM_GAS_ = 'G', CEM_H2O_ = 'V', CEM_CO2_ = 'C', CEM_CH4_ = 'M', CEM_N2_ = 'T', CEM_H2_ = 'H', CEM_O2_ = 'O', CEM_AR_ = 'A', CEM_PO_ = 'P', CEM_NP_ = 'Q';

/// The state of one calculation. Fugpure: fugacity coefficient, G, H, S of the residual, volume, Cp of the residual.
struct Work
{
    real Tk, Pbar, R_CONST = 8.31451, RR = 0.00831451, Pkb;
    char EosCode = 'G';
    real Eosparm[2];
    real Fugpure[6];
    real Rho[11];
};


long int FugacityPT(Work& w, real *EoSparam);
long int FugacityCorresponding(Work& w);
long int FugacityCO2(Work& w);
long int VolumeFugacity(Work& w, long int phState, real pp, real p0, real a, real b, real c, real d, real e, real &vol, real &fc);
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

    a0 = 5.45963e-5;
    a1 = -8.6392e-6;
    b0 = 9.18301e-4;
    c0 = -3.30558e-5;
    c1 = 2.30524e-6;
    d0 = 6.93054e-7;
    d1 = -8.38293e-8;
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

    a0 = 659.8;
    a1 = 0.21078;
    a2 = -6.3976e-4;
    b = 3.057;
    c0 = -1.78198e-1;
    c1 = 2.45317e-5;
    d0 = 5.40776e-3;
    d1 = -1.59046e-6;
    e0 = 0.;
    a = a0 + a1*w.Tk + a2*pow(w.Tk,2.);
    da = a1 + 2.*a2*w.Tk;
    d2a = 2.*a2;
    c = c0 + c1*w.Tk;
    d = d0 + d1*w.Tk;
    e = e0;
    p0 = 5.;

    // molar volume and fugacity coefficient
    phState = 1;
    VolumeFugacity(w, phState, w.Pkb, p0, a, b, c, d, e, vc, fc);

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

long int VolumeFugacity(Work& w, long int phState, real pp, real p0, real a, real b, real c, real d, real e, real &vol, real &fc)
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
        co = atan(sqrt(1.-cp2/(cq3))/(cp/sqrt(cq3)));
        if (co<0.)
            co = co + 3.1415927;
        v1 = (-2.)*sqrt(cq)*cos(co/3.) - cb/3.;
        v2 = (-2.)*sqrt(cq)*cos((co+2.*3.1415927)/3.) - cb/3.;
        v3 = (-2.)*sqrt(cq)*cos((co-2.*3.1415927)/3.) - cb/3.;
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

    a0 = 1113.4;
    a1 = -0.88517;
    a2 = 4.53e-3;
    a3 = -1.3183e-5;
    a4 = -0.22291;
    a5 = -3.8022e-4;
    a6 = 1.7791e-7;
    a7 = 5.8487;
    a8 = -2.1370e-2;
    a9 = 6.8133e-5;
    b = 1.465;
    c0 = -8.909e-2;
    c1 = 0.;
    d0 = 1.9853e-3;
    d1 = 0.;
    e0 = 8.0331e-2;
    c = c0 + c1*w.Tk;
    d = d0 + d1*w.Tk;
    e = e0;
    //dc = c1;
    //dd = d1;
    p0 = 2.;

    // molar volume and fugacity coefficient
    if (w.Tk > 695.)   // supercritical fluid phase
    {
        phState = 1;
        a = a0 + a4*(w.Tk-673.) + a5*pow((w.Tk-673.),2.) + a6*pow((w.Tk-673.),3.);
        da = -a4 - 2.*a5*(673.-w.Tk) - 3.*a6*pow((673.-w.Tk),2.);
        d2a = 2.*a5 + 6.*a6*(673.-w.Tk);
        VolumeFugacity(w, phState, w.Pkb, p0, a, b, c, d, e, vc, fc);
    }

    else   // subcritical region
    {
        Psat = (-13.627e-3) + (7.29395e-7)*pow(w.Tk,2.) - (2.34622e-9)*pow(w.Tk,3.) + (4.83607e-15)*pow(w.Tk,5.);
        if (w.Pkb < Psat)
        {
            phState = 1;   // vapor
            if (w.Tk < 673.)
            {
                a = a0 + a7*(673.-w.Tk) + a8*pow((673.-w.Tk),2.) + a9*pow((673.-w.Tk),3.);
                da = -a7 - 2.*a8*(673.-w.Tk) - 3.*a9*pow((673.-w.Tk),2.);
                d2a = 2.*a8 + 6.*a9*(673.-w.Tk);
            }
            else
            {
                a = a0 + a4*(673.-w.Tk) + a5*pow((673.-w.Tk),2.) + a6*pow((673.-w.Tk),3.);
                da = -a4 - 2.*a5*(673.-w.Tk) - 3.*a6*pow((673.-w.Tk),2.);
                d2a = 2.*a5 + 6.*a6*(673.-w.Tk);
            }
            VolumeFugacity(w, phState, w.Pkb, p0, a, b, c, d, e, vc, fc);
        }

        else
        {
            phState = 1;   // gaseous phase at Psat
            if (w.Tk < 673.)
            {
                a = a0 + a7*(673.-w.Tk) + a8*pow((673.-w.Tk),2.) + a9*pow((673.-w.Tk),3.);
                da = -a7 - 2.*a8*(673.-w.Tk) - 3.*a9*pow((673.-w.Tk),2.);
                d2a = 2.*a8 + 6.*a9*(673.-w.Tk);
            }
            else
            {
                a = a0 + a4*(673.-w.Tk) + a5*pow((673.-w.Tk),2.) + a6*pow((673.-w.Tk),3.);
                da = -a4 - 2.*a5*(673.-w.Tk) - 3.*a6*pow((673.-w.Tk),2.);
                d2a = 2.*a5 + 6.*a6*(673.-w.Tk);
            }
            VolumeFugacity(w, phState, Psat, p0, a, b, c, d, e, v, g1);

            phState = 2; // liquid phase at Psat
            if (w.Tk < 673.)
            {
                a = a0 + a1*(673.-w.Tk) + a2*pow((673.-w.Tk),2.) + a3*pow((673.-w.Tk),3.);
                da = -a1 - 2.*a2*(673.-w.Tk) - 3.*a3*pow((673.-w.Tk),2.);
                d2a = 2.*a2 + 6.*a3*(673.-w.Tk);
            }
            else
            {
                a = a0 + a4*(673.-w.Tk) + a5*pow((673.-w.Tk),2.) + a6*pow((673.-w.Tk),3.);
                da = -a4 - 2.*a5*(673.-w.Tk) - 3.*a6*pow((673.-w.Tk),2.);
                d2a = 2.*a5 + 6.*a6*(673.-w.Tk);
            }
            VolumeFugacity(w, phState, Psat, p0, a, b, c, d, e, v, g2);

            phState = 2; // fluid phase at w.Pkb
            if (w.Tk < 673.)
            {
                a = a0 + a1*(673.-w.Tk) + a2*pow((673.-w.Tk),2.) + a3*pow((673.-w.Tk),3.);
                da = -a1 - 2.*a2*(673.-w.Tk) - 3.*a3*pow((673.-w.Tk),2.);
                d2a = 2.*a2 + 6.*a3*(673.-w.Tk);
            }
            else
            {
                a = a0 + a4*(673.-w.Tk) + a5*pow((673.-w.Tk),2.) + a6*pow((673.-w.Tk),3.);
                da = -a4 - 2.*a5*(673.-w.Tk) - 3.*a6*pow((673.-w.Tk),2.);
                d2a = 2.*a5 + 6.*a6*(673.-w.Tk);
            }
            VolumeFugacity(w, phState, w.Pkb, p0, a, b, c, d, e, vc, g3);  // corrected from (phState, w.Pkb, psat ...)
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
void fugacityProperties(const real& Tk, const real& Pbar, double Tmin, char code, real* parameters, real* out)
{
    if ((Tk >= Tmin) && (Tk < 1e4) && (Pbar >= 1e-5) && (Pbar < 1e5))
    {
        Work w;
        w.Tk = Tk;
        w.Pbar = Pbar;
        w.Pkb = Pbar/1000.;
        w.EosCode = code;
        FugacityPT(w, parameters);
        for (int i = 0; i < 6; i++)
            out[i] = w.Fugpure[i];
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

auto thermoPropertiesGasCORK(real TK, real Pbar, const GasInputs& subst, ThermoPropertiesSubstanceAD tps, bool cancel_fugacity) -> ThermoPropertiesSubstanceAD
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

    fugacityProperties(TK, Pbar, TClow, Eos_Code, CPg.data(), FugProps);

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (TK) * log( FugProps[0] ); // from ideal gas at 1 bar and givent T to pure real gas/fluid at T and P
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (Pbar); //FugProps[0] - fugacity coefficient
    // the standard state has no fugacity coefficient: remove RT ln(phi) added above, unless the caller keeps it
    if (cancel_fugacity)
        tps.gibbs_energy -= R_CONSTANT * (TK) * log(Fug/Pbar);

    return tps;
}
}
