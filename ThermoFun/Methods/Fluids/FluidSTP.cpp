// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

// The formulas of this file come from s_solmod2.cpp of the GEMS3K code, (c) T. Wagner, D. Kulik, S. Dmitrieva, F. Hingerl,
// S. Churakov, A. Rysin and others, under the GNU Lesser General Public License version 3 or later.

#include "Methods/Fluids/FluidSTP.h"
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/Constants.h>

namespace ThermoFun {

namespace {

// The fluids that have an equation of their own; another fluid goes by corresponding states
constexpr char CEM_GAS_ = 'G', CEM_H2O_ = 'V', CEM_CO2_ = 'C', CEM_CH4_ = 'M', CEM_N2_ = 'T', CEM_H2_ = 'H', CEM_O2_ = 'O', CEM_AR_ = 'A', CEM_PO_ = 'P', CEM_NP_ = 'Q';

/// The places of the constants of the model: the order of the catalog, and of its file in Methods/ParameterSets.
enum Constant
{
    TMIN, TMAX, PMIN, PMAX, cfh_1, cfh_2, cfh_3, cfh_4, cfh_5, cfh_6, cfh_7, cfh_8, cfh_9, cfh_10, cfh_11, cfh_12, cfh_13, cfh_14, cfh_15, cfh_16, cfh_17, cfh_18, cfh_19, cfh_20, cfh_21, cfh_22, cfh_23, cfh_24, cfh_25, cfh_26, cfh_27, cfh_28, cfh_29, cfh_30, cfh_31, cfh_32, cfh_33, cfh_34, cfh_35, cfh_36, cfh_37, cfh_38, cfh_39, cfh_40, cfh_41, cfh_42, cfh_43, cfh_44, cfh_45, cfh_46, cfh_47, cfh_48, cfh_49, cfh_50, cfh_51, cfh_52, cfh_53, cfh_54, cfh_55, cfh_56, cfh_57, cfh_58, cfh_59, cfh_60, cfc_1, cfc_2, cfc_3, cfc_4, cfc_5, cfc_6, cfc_7, cfc_8, cfc_9, cfc_10, cfc_11, cfc_12, cfc_13, cfc_14, cfc_15, cfc_16, cfc_17, cfc_18, cfc_19, cfc_20, cfc_21, cfc_22, cfc_23, cfc_24, cfc_25, cfc_26, cfc_27, cfc_28, cfc_29, cfc_30, cfc_31, cfc_32, cfc_33, cfc_34, cfc_35, cfc_36, cfc_37, cfc_38, cfc_39, cfc_40, cfc_41, cfc_42, cfc_43, cfc_44, cfc_45, cfc_46, cfc_47, cfc_48, cfc_49, cfc_50, cfc_51, cfc_52, cfc_53, cfc_54, cfc_55, cfc_56, cfc_57, cfc_58, cfc_59, cfc_60, cs_a0, cs_a1, cs_b0, cs_c0, cs_c1, cs_d0, cs_d1, psat_co2_Rhoc, psat_co2_a1, psat_co2_a2, psat_co2_a3, psat_co2_a4, psat_co2_b1, psat_co2_b2, psat_co2_b3, psat_co2_b4, psat_co2_c1, psat_co2_c2, psat_co2_c3, psat_co2_c4, psat_co2_c5, psat_h2o_Rhoc, psat_h2o_a1, psat_h2o_a2, psat_h2o_a3, psat_h2o_a4, psat_h2o_a5, psat_h2o_a6, psat_h2o_b1, psat_h2o_b2, psat_h2o_b3, psat_h2o_b4, psat_h2o_b5, psat_h2o_b6, psat_h2o_c1, psat_h2o_c2, psat_h2o_c3, psat_h2o_c4, psat_h2o_c5, psat_h2o_c6, Mw_h2o, Mw_co2
};

/// The state of one calculation. Fugpure: fugacity coefficient, G, H, S of the residual, volume, Cp of the residual.
struct Work
{
    real Tk, Pbar, R_CONST = 8.31451, RC, RR, TMIN, TMAX, PMIN, PMAX, Pkbar, Pkb, Pmpa;
    const double* k = nullptr;  ///< the constants of the model
    char EosCode = 'G';
    real Tc, Pc, Psat, Rhol, Rhov, Mw;
    real Phi, dPhiD, dPhiDD, dPhiT, dPhiTT, dPhiDT, dPhiDDD, dPhiDDT, dPhiDTT;
    real Fugpure[7];
    real Rho[11];
    real cfh[10][6], cfc[10][6];
};


long int UpdateTauP(Work& w);
void set_internal(Work& w);
long int FugacityPT(Work& w, real *EoSparam);
long int FugacityCorresponding(Work& w);
long int FugacityCO2(Work& w);
long int Helmholtz(Work& w, real rho, const real (*cf)[6]);
long int Pressure(Work& w, real rho, real &p, real &dpdrho, const real (*cf)[6]);
long int DensityGuess(Work& w, real &Rhoguess);
long int PsatCO2(Work& w);
long int PsatH2O(Work& w);
long int FugacityH2O(Work& w);

long int UpdateTauP(Work& w)
{
    w.Pkbar = w.Pbar/1000.;
    w.Pkb = w.Pbar/1000.;
    w.Pmpa = w.Pbar/10.;

    return 0;
}

void set_internal(Work& w)
{
    long int j, k;

    w.RC = w.R_CONST;  // gas constant (bar)
    w.RR = 0.00831451;  // gas constant (kbar)
    w.TMIN = w.k[TMIN];
    w.TMAX = w.k[TMAX];
    w.PMIN = w.k[PMIN];
    w.PMAX = w.k[PMAX];

    // EoS coefficients (temperature dependence)

    // transfer coefficients
    for (j=0; j<10; j++)
    {
            for (k=0; k<6; k++)
            {
                    w.cfh[j][k] = w.k[cfh_1 + 6*j + k];
                    w.cfc[j][k] = w.k[cfc_1 + 6*j + k];
            }
    }
}

long int FugacityPT(Work& w, real *EoSparam)
{
    long int iErr = 0;

    // reads EoS parameters from database into work array
    if( !EoSparam )
        return -1;  // Memory alloc error
    w.Tc = EoSparam[0];  // critical temperature (K)
    w.Pc = EoSparam[1]/(10.);  // critical pressure (MPa)

    // check for temperature/pressure limits
    if ( (w.Tk<w.TMIN) || (w.Tk>w.TMAX) || (w.Pmpa<w.PMIN) || (w.Pmpa>w.PMAX) )
    {
        iErr = 1;
        return iErr;
    }

    // select subroutines for different fluid types
    switch ( w.EosCode )
    {
            case CEM_H2O_:  // H2O
                    iErr = FugacityH2O(w);
                    break;
            case CEM_CO2_: // CO2
                    iErr = FugacityCO2(w);
                    break;
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
    tcr = w.Tc;
    // pcr = Eosparm[j][1]/1000.;  // kbar
    pcr = w.Pc*(10.)/(1000.);  // kbar (convert from MPa)
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
    long int k, maxit, iErr = 0;
    real pmpa, tol, rhoguess, rho, pnew, pgrad, rhoold, rhonew, errnew, /*errold,*/ step,
                    rhomin, rhomax, vol, lnfug, fug, fugc;
    real /*ar,*/ dard, dardd, darddd, dart, dartt, dardt, darddt, dardtt, pig,
                    g, h, s, cp, cv, /*u, a,*/ dpr, dprr, dpt, dptt, dprt, drt, drtt, drp, drpp, drtp;

    pmpa = w.Pbar/10.;  // MPa
    tol = 1.e-10;
    maxit = 1000;
    rhomin = (1.e-20)/1000./w.Mw;  // 1.e-20 kg m-3
    rhomax = (2400.)/1000./w.Mw;   // 2400 kg m-3

    // find initial density guess
    DensityGuess(w, rhoguess );
    Pressure(w, rhoguess, pnew, pgrad, w.cfc );

    rhoold = rhoguess;
    rhonew = rhoguess;
    errnew = pnew - pmpa;
    //errold = errnew;

    k = 0;
    do
    {
            k++;

            if ( k>= maxit )
            {
                    iErr = 2;
                    return iErr;
            }

            rhoold = rhonew;
            //errold = errnew;
            Pressure(w, rhonew, pnew, pgrad, w.cfc );
            errnew = pnew - pmpa;
            step = - errnew/pgrad;
            rhonew = rhoold + step;

            if ( rhonew < rhomin )
                    rhonew = rhomin ;
            if ( rhonew > rhomax )
                    rhonew = rhomax;

    } while ( fabs(1.-pnew/pmpa) > tol );

    // calculate thermodynamic properties
    rho = rhonew;
    Helmholtz(w, rho, w.cfc );
    //ar = w.RC*w.Tk*w.Phi;
    dard = w.RC*w.Tk*w.dPhiD;
    dardd = w.RC*w.Tk*w.dPhiDD;
    darddd = w.RC*w.Tk*w.dPhiDDD;
    dart = w.RC*(w.dPhiT*w.Tk+w.Phi);
    dartt = w.RC*(w.dPhiTT*w.Tk+2.*w.dPhiT);
    dardt = w.RC*(w.dPhiDT*w.Tk+w.dPhiD);
    darddt = w.RC*(w.dPhiDDT*w.Tk+w.dPhiDD);
    dardtt = w.RC*(w.dPhiDTT*w.Tk+2.*w.dPhiDT);

    vol = (1./rho)/10.;  // J bar-1
    lnfug = ( log(rho) + w.Phi + pmpa/(rho*w.RC*w.Tk) ) + log(w.RC*w.Tk) - 1.;
    fug = exp(lnfug);
    fugc = fug/pmpa;
    pig = rho*w.RC*w.Tk;
    g = log(fugc)*w.RC*w.Tk;
    s = - dart + w.RC*log(pmpa/pig);
    h = g + w.Tk*s;
    //a = ar - w.RC*w.Tk*log(pmpa/pig);
    cv = - w.Tk*dartt - w.RC;
    cp = cv + w.RC*pow((1.+(rho/w.RC)*dardt),2.) / (1.+(2.*rho)/(w.RC*w.Tk)*dard + pow(rho,2.)/(w.RC*w.Tk)*dardd);
    dpr = w.RC*w.Tk + 2.*rho*dard + pow(rho,2.)*dardd;
    dprr = 2.*dard + 4.*rho*dardd + pow(rho,2.)*darddd;
    dpt = rho*w.RC + pow(rho,2.)*dardt;
    dptt = pow(rho,2.)*dardtt;
    dprt = w.RC + 2.*rho*dardt + pow(rho,2.)*darddt;
    drp = (1./dpr);
    drpp = (-1.)*dprr*pow(dpr,-3.);
    drt = (-1.)*(1./dpr)*dpt;
    drtt = (dpt - dptt + dpt*(dprt-dpr)*pow(dpr,-1.))*pow(dpr,-1.)
            + (dprt - dpt*dprr*pow(dpr,-1.))*pow(dpr,-2.)*dpt;
    drtp = - pow(dpr,-1.) - (dprt-dpr)*pow(dpr,-2.) + dprr*dpt*pow(dpr,-3.);

    // copy results
    w.Fugpure[0] = fugc;  // fugacity coef.
    w.Fugpure[1] = g;     // Gres
    w.Fugpure[2] = h;     // Hres
    w.Fugpure[3] = s;     // Sres
    w.Fugpure[4] = vol;   // V
    w.Fugpure[5] = cp;    // CPres
    w.Fugpure[6] = cv;    // CVres
    w.Rho[0] = rho;      // mol cm-3
    w.Rho[1] = drt;      // mol cm-3 K-1
    w.Rho[2] = drtt;     // mol cm-3 K-2
    w.Rho[3] = drp;      // mol cm-3 MPa-1
    w.Rho[4] = drpp;     // mol cm-3 MPa-2
    w.Rho[5] = drtp;     // mol cm-3 K-1 MPa-1
    w.Rho[6] = dpr;      // MPa cm3 mol-1
    w.Rho[7] = dprr;     // MPa cm6 mol-2
    w.Rho[8] = dpt;      // MPa K-1
    w.Rho[9] = dptt;     // MPa K-2
    w.Rho[10] = dprt;    // MPa cm3 mol-1 K-1

    return iErr;
}

long int Helmholtz(Work& w, real rho, const real (*cf)[6])
{
    long int k;
    real c[10], dc[10], d2c[10];

    for (k=0; k<10; k++)
    {
            c[k] = cf[k][0]*pow(w.Tk,-4.) + cf[k][1]*pow(w.Tk,-2.) + cf[k][2]*pow(w.Tk,-1.) + cf[k][3]
                    + cf[k][4]*w.Tk + cf[k][5]*pow(w.Tk,2.);
            dc[k] = - 4.*cf[k][0]*pow(w.Tk,-5.) - 2.*cf[k][1]*pow(w.Tk,-3.) - cf[k][2]*pow(w.Tk,-2.)
                    + cf[k][4] + 2.*cf[k][5]*w.Tk;
            d2c[k] = 20.*cf[k][0]*pow(w.Tk,-6.) + 6.*cf[k][1]*pow(w.Tk,-4.) + 2.*cf[k][2]*pow(w.Tk,-3.)
                    + 2.*cf[k][5];
    }

    w.Phi = c[0]*rho + ( 1./(c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)) - 1./c[1] )
            - (c[6]/c[7])*(exp(-c[7]*rho)-1.) - (c[8]/c[9])*(exp(-c[9]*rho)-1.);
    w.dPhiD = c[0] - 1./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),2.)
            * (c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.))
            + c[6]*exp(-c[7]*rho) + c[8]*exp(-c[9]*rho);
    w.dPhiDD = 2./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),3.)
            * pow((c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.)),2.)
            - 1./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),2.)
            * (2.*c[3]+6.*c[4]*rho+12.*c[5]*pow(rho,2.))
            - c[6]*c[7]*exp(-c[7]*rho) - c[8]*c[9]*exp(-c[9]*rho);
    w.dPhiT = dc[0]*rho - 1./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),2.)
            * (dc[1]+dc[2]*rho+dc[3]*pow(rho,2.)+dc[4]*pow(rho,3.)+dc[5]*pow(rho,4.)) + 1./pow(c[1],2.)*dc[1]
            - dc[6]/c[7]*(exp(-c[7]*rho)-1.) + c[6]/pow(c[7],2.)*(exp(-c[7]*rho)-1.)*dc[7]
            + c[6]/c[7]*dc[7]*rho*exp(-c[7]*rho) - dc[8]/c[9]*(exp(-c[9]*rho)-1)
            + c[8]/pow(c[9],2.)*(exp(-c[9]*rho)-1.)*dc[9] + c[8]/c[9]*dc[9]*rho*exp(-c[9]*rho);
    w.dPhiTT = - 2./pow(c[1],3.)*pow(dc[1],2.)
            + 2./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),3.)
            * pow((dc[1]+dc[2]*rho+dc[3]*pow(rho,2.)+dc[4]*pow(rho,3.)+dc[5]*pow(rho,4.)),2.)
            - d2c[8]/c[9]*(exp(-c[9]*rho)-1.) - d2c[6]/c[7]*(exp(-c[7]*rho)-1.)
            + 2.*dc[6]/pow(c[7],2.)*(exp(-c[7]*rho)-1)*dc[7] + d2c[0]*rho
            - 1./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),2.)
            * (d2c[1]+d2c[2]*rho+d2c[3]*pow(rho,2.)+d2c[4]*pow(rho,3.)+d2c[5]*pow(rho,4.))
            - 2.*c[8]/pow(c[9],2.)*pow(dc[9],2.)*rho*exp(-c[9]*rho) + c[8]/pow(c[9],2.)*(exp(-c[9]*rho)-1.)*d2c[9]
            - 2.*c[8]/pow(c[9],3.)*(exp(-c[9]*rho)-1.)*pow(dc[9],2.) + 2.*dc[8]/c[9]*dc[9]*rho*exp(-c[9]*rho)
            + 2.*dc[8]/pow(c[9],2.)*(exp(-c[9]*rho)-1.)*dc[9] - c[6]/c[7]*pow(dc[7],2.)*pow(rho,2.)*exp(-c[7]*rho)
            + c[6]/c[7]*d2c[7]*rho*exp(-c[7]*rho) - 2.*c[6]/pow(c[7],2.)*pow(dc[7],2.)*rho*exp(-c[7]*rho)
            + c[6]/pow(c[7],2.)*(exp(-c[7]*rho)-1.)*d2c[7] - 2.*c[6]/pow(c[7],3.)*(exp(-c[7]*rho)-1.)*pow(dc[7],2.)
            + 2.*dc[6]/c[7]*dc[7]*rho*exp(-c[7]*rho) + 1./pow(c[1],2.)*d2c[1]
            - c[8]/c[9]*pow(dc[9],2.)*pow(rho,2.)*exp(-c[9]*rho) + c[8]/c[9]*d2c[9]*rho*exp(-c[9]*rho);
    w.dPhiDT = dc[0] + 2./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),3.)
            * (dc[1]+dc[2]*rho+dc[3]*pow(rho,2.)+dc[4]*pow(rho,3.)+dc[5]*pow(rho,4.))
            * (c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.))
            - 1./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),2.)
            * (dc[2]+2.*dc[3]*rho+3.*dc[4]*pow(rho,2.)+4.*dc[5]*pow(rho,3.))
            + dc[6]*exp(-c[7]*rho) - c[6]*dc[7]*rho*exp(-c[7]*rho)
            + dc[8]*exp(-c[9]*rho) - c[8]*dc[9]*rho*exp(-c[9]*rho);
    w.dPhiDDD = - 6./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),4.)
            * pow((c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.)),3.)
            + 6./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),3.)
            * (c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.))*(2.*c[3]+6.*c[4]*rho+12.*c[5]*pow(rho,2.))
            - 1./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),2.) * (6.*c[4]+24.*c[5]*rho)
            + c[6]*pow(c[7],2.)*exp(-c[7]*rho)+c[8]*pow(c[9],2.)*exp(-c[9]*rho);
    w.dPhiDDT = - 6./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),4.)
            * (dc[1]+dc[2]*rho+dc[3]*pow(rho,2.)+dc[4]*pow(rho,3.)+dc[5]*pow(rho,4.))
            * pow((c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.)),2.)
            + 4./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),3.)
            * (dc[2]+2.*dc[3]*rho+3.*dc[4]*pow(rho,2.)+4.*dc[5]*pow(rho,3.))
            * (c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.))
            + 2./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),3.)
            * (dc[1]+dc[2]*rho+dc[3]*pow(rho,2.)+dc[4]*pow(rho,3.)+dc[5]*pow(rho,4.))
            * (2.*c[3]+6.*c[4]*rho+12.*c[5]*pow(rho,2.))
            - 1./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),2.)
            * (2.*dc[3]+6.*dc[4]*rho+12.*dc[5]*pow(rho,2.))
            - dc[6]*c[7]*exp(-c[7]*rho) - c[6]*dc[7]*exp(-c[7]*rho)
            + c[6]*dc[7]*rho*c[7]*exp(-c[7]*rho) - dc[8]*c[9]*exp(-c[9]*rho)
            - c[8]*dc[9]*exp(-c[9]*rho) + c[8]*dc[9]*rho*c[9]*exp(-c[9]*rho);
    w.dPhiDTT = 2./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),3.)
            * (d2c[1]+d2c[2]*rho+d2c[3]*pow(rho,2.)+d2c[4]*pow(rho,3.)+d2c[5]*pow(rho,4.))
            * (c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.)) + d2c[0]
            - 2.*dc[8]*dc[9]*rho*exp(-c[9]*rho) - c[6]*d2c[7]*rho*exp(-c[7]*rho)
            + c[6]*pow(dc[7],2.)*pow(rho,2.)*exp(-c[7]*rho)
            - 6./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),4.)
            * pow((dc[1]+dc[2]*rho+dc[3]*pow(rho,2.)+dc[4]*pow(rho,3.)+dc[5]*pow(rho,4.)),2.)
            * (c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.))
            + 4./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),3.)
            * (dc[1]+dc[2]*rho+dc[3]*pow(rho,2.)+dc[4]*pow(rho,3.)+dc[5]*pow(rho,4.))
            * (dc[2]+2.*dc[3]*rho+3.*dc[4]*pow(rho,2.)+4.*dc[5]*pow(rho,3.)) + d2c[6]*exp(-c[7]*rho)
            - 1./pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),2.)
            * (d2c[2]+2.*d2c[3]*rho+3.*d2c[4]*pow(rho,2.)+4.*d2c[5]*pow(rho,3.))
            - 2.*dc[6]*dc[7]*rho*exp(-c[7]*rho) + c[8]*pow(dc[9],2.)*pow(rho,2.)*exp(-c[9]*rho)
            - c[8]*d2c[9]*rho*exp(-c[9]*rho) + d2c[8]*exp(-c[9]*rho);

    return 0;
}

long int Pressure(Work& w, real rho, real &p, real &dpdrho, const real (*cf)[6])
{
    long int k;
    real pred, dpred;
    real c[10];

    for (k=0; k<10; k++)
    {
        c[k] = cf[k][0]*pow(w.Tk,-4.) + cf[k][1]*pow(w.Tk,-2.) + cf[k][2]*pow(w.Tk,-1.) + cf[k][3]
            + cf[k][4]*w.Tk + cf[k][5]*pow(w.Tk,2.);
    }

    pred = rho + c[0]*pow(rho,2.) - pow(rho,2.) * ( (c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.))
            / pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),2.) )
            + c[6]*pow(rho,2.)*exp(-c[7]*rho) + c[8]*pow(rho,2.)*exp(-c[9]*rho);
    dpred = 1. + 2.*c[0]*rho - 2.*rho*(c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.))
            / pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),2.)
            - pow(rho,2.)*(2.*c[3]+6.*c[4]*rho+12.*c[5]*pow(rho,2.))
            / pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),2.)
            + 2.*pow(rho,2.)*pow((c[2]+2.*c[3]*rho+3.*c[4]*pow(rho,2.)+4.*c[5]*pow(rho,3.)),2.)
            / pow((c[1]+c[2]*rho+c[3]*pow(rho,2.)+c[4]*pow(rho,3.)+c[5]*pow(rho,4.)),3.)
            + 2.*c[6]*rho*exp(-c[7]*rho) - c[6]*pow(rho,2.)*c[7]*exp(-c[7]*rho)
            + 2.*c[8]*rho*exp(-c[9]*rho) - c[8]*pow(rho,2.)*c[9]*exp(-c[9]*rho);

    p = pred*(w.RC*w.Tk);
    dpdrho = dpred*(w.RC*w.Tk);

    return 0;
}

long int DensityGuess(Work& w, real &Rhoguess)
{
    real tred, pred, rhomin, rhomax, rhoguess;

    tred = w.Tk/w.Tc;
    pred = w.Pmpa/w.Pc;

    if ( w.EosCode == CEM_H2O_ )  // H2O
    {
            rhomin = (1.e-20/1000.)/w.Mw;  // 1.e-20 kg m-3
            rhomax = (1800./1000.)/w.Mw;   // 1800 kg m-3

            if ( w.Tk < w.Tc )
            {
                    PsatH2O(w);
                    if ( w.Pmpa < w.Psat )
                            rhoguess = w.Rhov;
                    else
                            rhoguess = w.Rhol;
            }

            else
            {
                    if ( w.Pmpa < 400. )
                            rhoguess = 1000./(w.Tk-273.15)*2.*w.Pmpa;
                    else
                            rhoguess = 800.;
            }

            Rhoguess = (rhoguess/1000.)/w.Mw;  // mol cm-3

            if ( Rhoguess < rhomin )
                    Rhoguess = rhomin;
            if ( Rhoguess > rhomax )
                    Rhoguess = rhomax;
    }

    else  // CO2 (other fluids)
    {
            rhomin = (1.e-20)/1000./w.Mw;  // 1.e-20 kg m-3
            rhomax = (2400.)/1000./w.Mw;   // 2400 kg m-3

            if ( w.Tk < w.Tc )
            {
                    PsatCO2(w);
                    if ( w.Pmpa < w.Psat )
                            rhoguess = w.Rhov;  // kg m-3
                    else
                            rhoguess = w.Rhol;
            }

            else
            {
                    rhoguess = (pred*w.Pc*1000.)/((0.1889241)*tred*w.Tc);  // kg m-3
            }

            Rhoguess = (rhoguess/1000.)/w.Mw;  // mol cm-3

            if ( Rhoguess < rhomin )
                    Rhoguess = rhomin;
            if ( Rhoguess > rhomax )
                    Rhoguess = rhomax;
    }

    return 0;
}

long int PsatCO2(Work& w)
{
    real Rhoc, a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, c5, tau, ppc, psat,
                    rhol, rhov, rholc, rhovc;

    Rhoc = w.k[psat_co2_Rhoc];   // kg m-3
    a1 = w.k[psat_co2_a1];
    a2 = w.k[psat_co2_a2];
    a3 = w.k[psat_co2_a3];
    a4 = w.k[psat_co2_a4];
    b1 = w.k[psat_co2_b1];
    b2 = w.k[psat_co2_b2];
    b3 = w.k[psat_co2_b3];
    b4 = w.k[psat_co2_b4];
    c1 = w.k[psat_co2_c1];
    c2 = w.k[psat_co2_c2];
    c3 = w.k[psat_co2_c3];
    c4 = w.k[psat_co2_c4];
    c5 = w.k[psat_co2_c5];

    tau = 1. - w.Tk/w.Tc;
    ppc = (w.Tc/w.Tk)*( a1*pow(tau,1.) + a2*pow(tau,1.5) + a3*pow(tau,2.) + a4*pow(tau,4.) );
    psat = exp(ppc)*w.Pc;
    rholc = b1*pow(tau,0.34)+ b2*pow(tau,0.5) + b3*pow(tau,(10./6.)) + b4*pow(tau,(11./6.));
    rhol = exp(rholc)*Rhoc;
    rhovc = c1*pow(tau,0.34) + c2*pow(tau,0.5) + c3*pow(tau,1.) + c4*pow(tau,(7./3.)) + c5*pow(tau,(14./3.));
    rhov = exp(rhovc)*Rhoc;

    // copy results
    w.Psat = psat;  // MPa
    w.Rhol = rhol;  // kg m-3
    w.Rhov = rhov;  // kg m-3

    return 0;
}

long int PsatH2O(Work& w)
{
    real Rhoc, a1, a2, a3, a4, a5, a6, b1, b2, b3, b4, b5, b6, c1, c2, c3, c4, c5, c6,
                    tau, psat, rhol, rhov, ppc, rrc;

    Rhoc = w.k[psat_h2o_Rhoc];   // kg m-3
    a1 = w.k[psat_h2o_a1];
    a2 = w.k[psat_h2o_a2];
    a3 = w.k[psat_h2o_a3];
    a4 = w.k[psat_h2o_a4];
    a5 = w.k[psat_h2o_a5];
    a6 = w.k[psat_h2o_a6];
    b1 = w.k[psat_h2o_b1];
    b2 = w.k[psat_h2o_b2];
    b3 = w.k[psat_h2o_b3];
    b4 = w.k[psat_h2o_b4];
    b5 = w.k[psat_h2o_b5];
    b6 = w.k[psat_h2o_b6];
    c1 = w.k[psat_h2o_c1];
    c2 = w.k[psat_h2o_c2];
    c3 = w.k[psat_h2o_c3];
    c4 = w.k[psat_h2o_c4];
    c5 = w.k[psat_h2o_c5];
    c6 = w.k[psat_h2o_c6];

    tau = 1 - w.Tk/w.Tc;
    ppc = (w.Tc/w.Tk)*( a1*tau + a2*pow(tau,1.5) + a3*pow(tau,3.) + a4*pow(tau,3.5)
            + a5*pow(tau,4.) + a6*pow(tau,7.5) );
    psat = exp(ppc) * w.Pc;
    rhol = ( 1. + b1*pow(tau,(1./3.)) + b2*pow(tau,(2./3.)) + b3*pow(tau,(5./3.)) + b4*pow(tau,(16./3.))
            + b5*pow(tau,(43./3.)) + b6*pow(tau,(110./3.)) ) * Rhoc;
    rrc = c1*pow(tau,2./6.) + c2*pow(tau,4./6.) + c3*pow(tau,8./6.) + c4*pow(tau,18./6.)
            + c5*pow(tau,37./6.) + c6*pow(tau,71./6.);
    rhov = exp(rrc) * Rhoc;

    // copy results
    w.Psat = psat;  // MPa
    w.Rhol = rhol;
    w.Rhov = rhov;

    return 0;
}

long int FugacityH2O(Work& w)
{
    long int k, maxit, iErr = 0;
    real pmpa, tol, rhoguess, rho, pnew, pgrad, rhoold, rhonew, errnew, /*errold,*/ step,
                    rhomin, rhomax, vol, lnfug, fug, fugc;
    real /*ar,*/ dard, dardd, darddd, dart, dartt, dardt, darddt, dardtt, pig,
                    g, h, s, cp, cv, /*u, a,*/ dpr, dprr, dpt, dptt, dprt, drt, drtt, drp, drpp, drtp;

    pmpa = w.Pbar/10.;  // MPa
    tol = 1.e-10;
    maxit = 1000;
    rhomin = (1.e-20/1000.)/w.Mw;  // 1.e-20 kg m-3
    rhomax = (1800./1000.)/w.Mw;   // 1800 kg m-3

    // find initial density guess
    DensityGuess(w, rhoguess );
    Pressure(w, rhoguess, pnew, pgrad, w.cfh );

    rhoold = rhoguess;
    rhonew = rhoguess;
    errnew = pnew - pmpa;
    //errold = pnew - pmpa;

    // find density by Newton iteration
    k = 0;
    do
    {
            k++;

            if ( k>= maxit )
            {
                    iErr = 2;
                    return iErr;
            }

            rhoold = rhonew;
            //errold = errnew;
            Pressure(w, rhonew, pnew, pgrad, w.cfh );
            errnew = pnew - pmpa;
            step = - errnew/pgrad;
            rhonew = rhoold + step;

            if ( rhonew < rhomin )
                    rhonew = rhomin ;
            if ( rhonew > rhomax )
                    rhonew = rhomax;

    } while ( fabs(1.-pnew/pmpa) > tol );

    // calculate thermodynamic properties
    rho = rhonew;
    Helmholtz(w, rho, w.cfh );
    //ar = w.RC*w.Tk*w.Phi;
    dard = w.RC*w.Tk*w.dPhiD;
    dardd = w.RC*w.Tk*w.dPhiDD;
    darddd = w.RC*w.Tk*w.dPhiDDD;
    dart = w.RC*(w.dPhiT*w.Tk+w.Phi);
    dartt = w.RC*(w.dPhiTT*w.Tk+2.*w.dPhiT);
    dardt = w.RC*(w.dPhiDT*w.Tk+w.dPhiD);
    darddt = w.RC*(w.dPhiDDT*w.Tk+w.dPhiDD);
    dardtt = w.RC*(w.dPhiDTT*w.Tk+2.*w.dPhiDT);

    vol = (1./rho)/10.;  // J bar-1
    lnfug = ( log(rho) + w.Phi + pmpa/(rho*w.RC*w.Tk) ) + log(w.RC*w.Tk) - 1.;
    fug = exp(lnfug);
    fugc = fug/pmpa;
    pig = rho*w.RC*w.Tk;
    g = log(fugc)*w.RC*w.Tk;
    s = - dart + w.RC*log(pmpa/pig);
    h = g + w.Tk*s;
    //a = ar - w.RC*w.Tk*log(pmpa/pig);
    cv = - w.Tk*dartt - w.RC;
    cp = cv + w.RC*pow((1.+(rho/w.RC)*dardt),2.) / (1.+(2.*rho)/(w.RC*w.Tk)*dard + pow(rho,2.)/(w.RC*w.Tk)*dardd);
    dpr = w.RC*w.Tk + 2.*rho*dard + pow(rho,2.)*dardd;
    dprr = 2.*dard + 4.*rho*dardd + pow(rho,2.)*darddd;
    dpt = rho*w.RC + pow(rho,2.)*dardt;
    dptt = pow(rho,2.)*dardtt;
    dprt = w.RC + 2.*rho*dardt + pow(rho,2.)*darddt;
    drp = (1./dpr);
    drpp = (-1.)*dprr*pow(dpr,-3.);
    drt = (-1.)*(1./dpr)*dpt;
    drtt = (dpt - dptt + dpt*(dprt-dpr)*pow(dpr,-1.))*pow(dpr,-1.)
            + (dprt - dpt*dprr*pow(dpr,-1.))*pow(dpr,-2.)*dpt;
    drtp = - pow(dpr,-1.) - (dprt-dpr)*pow(dpr,-2.) + dprr*dpt*pow(dpr,-3.);

    // copy results
    w.Fugpure[0] = fugc;  // fugacity coef.
    w.Fugpure[1] = g;     // Gres
    w.Fugpure[2] = h;     // Hres
    w.Fugpure[3] = s;     // Sres
    w.Fugpure[4] = vol;   // V
    w.Fugpure[5] = cp;    // CPres
    w.Fugpure[6] = cv;    // CVres
    w.Rho[0] = rho;      // mol cm-3
    w.Rho[1] = drt;      // mol cm-3 K-1
    w.Rho[2] = drtt;     // mol cm-3 K-2
    w.Rho[3] = drp;      // mol cm-3 MPa-1
    w.Rho[4] = drpp;     // mol cm-3 MPa-2
    w.Rho[5] = drtp;     // mol cm-3 K-1 MPa-1
    w.Rho[6] = dpr;      // MPa cm3 mol-1
    w.Rho[7] = dprr;     // MPa cm6 mol-2
    w.Rho[8] = dpt;      // MPa K-1
    w.Rho[9] = dptt;     // MPa K-2
    w.Rho[10] = dprt;    // MPa cm3 mol-1 K-1

    return iErr;
}

/// The fugacity and the residual functions of a pure fluid. Outside the range of the model, the fluid is an ideal gas.
void fugacityProperties(const real& Tk, const real& Pbar, double Tmin, char code, real* parameters, const double* constants, real* out)
{
    if ((Tk >= Tmin) && (Tk < 1e4) && (Pbar >= 1e-5) && (Pbar < 1e5))
    {
        Work w;
        w.k = constants;
        w.Tk = Tk;
        w.Pbar = Pbar;
        UpdateTauP(w);
        set_internal(w);
        w.EosCode = code;
        if (code == CEM_H2O_)
            w.Mw = w.k[Mw_h2o];
        if (code == CEM_CO2_)
            w.Mw = w.k[Mw_co2];
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

auto thermoPropertiesFluidSTP(real TK, real Pbar, const FluidInputs& subst, ThermoPropertiesSubstanceAD tps, bool cancel_fugacity) -> ThermoPropertiesSubstanceAD
{
    real FugProps[6];
    char Eos_Code = 'G';
    if (Pbar.val() == 0.0)
        Pbar += 1e-5;

    if (subst.formula == "CO2") Eos_Code = 'C';
    if (subst.formula == "H2O") Eos_Code = 'V';
    double TClow = subst.lower_T;
    real CPg[7];
    for (unsigned int i = 0; i < 7; i++)
    {
        CPg[i] = i < subst.parameters.size() ? subst.parameters[i] : real(0.0);
    }

    fugacityProperties(TK, Pbar, TClow, Eos_Code, CPg, subst.constants.data(), FugProps);

    // increment thermodynamic properties
    tps.gibbs_energy += R_CONSTANT * (TK) * log( FugProps[0] );
    tps.enthalpy     += FugProps[2];
    tps.entropy      += FugProps[3];
    tps.volume        = FugProps[4];
    auto Fug = FugProps[0] * (Pbar);
    // the standard state has no fugacity coefficient: remove RT ln(phi) added above, unless the caller keeps it
    if (cancel_fugacity)
        tps.gibbs_energy -= R_CONSTANT * (TK) * log(Fug/Pbar);

    return tps;
}
}
