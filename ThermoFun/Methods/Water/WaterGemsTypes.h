// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_WATER_WATERGEMSTYPES_H
#define THERMOFUN_METHODS_WATER_WATERGEMSTYPES_H

// The structures of the water code of GEMS (SUPCRT92 H2O92 in C), shared by its HGK and LVS equations and the transport
// properties. Units of the code: T in K, P in MPa, density in kg/m3 (g/cm3 in the HGK part), energies per gram.

#include <cstring>

#include <ThermoFun/Common/Real.hpp>

namespace ThermoFun {

// Forward declarations
struct WaterTripleProperties;

typedef struct
{ //work structure  t/d properties of water-solution
    // ( see at WATERPARAM)
    real Aw, Gw, Sw, Uw, Hw, Cvw, Cpw, Speedw, Alphaw,
    Betaw, Dielw, Viscw, Tcondw, Surtenw, Tdiffw,
    Prndtlw, Visckw, Albew;
    real ZBorn, YBorn, QBorn, dAldT, XBorn;
    real Gamw;   // rho (d2P/drho2)/(dP/drho) at constant T (dimensionless), gives d2rho/dP2 = -rho beta^2 Gamw
}
WPROPS;

struct HGK_SPECS
{// HGK CONTROLS AND SPECIFICAIONS
    int it;
    int id;
    int ip;
    int ih;
    int itripl;
    int isat;
    int iopt;
    int useLVS;
    int epseqn;
    int icrit;
    int metastable;
};

struct HGK_STATES
{
    real Temp;    // C
    real Pres;    // bar
    real Psat;   // bar
    real Dens[2]; // g/cm3
//    STATES()
//    {
//        Temp = -274.;
//        Pres = -1;
//        Dens[0] =Dens[1] = 0.;
//    }
//    void setdef()
//    {
//        Temp = -274.;
//        Pres = -1;
//        Dens[0] =Dens[1] = 0.;
//    }
};

typedef struct
{ // set local parametres to fact scales
    real ft;
    real fd;
    real fvd;
    real fvk;
    real fs;
    real fp;
    real fh;
    real fst;
    real fc;
}
UNITS;

typedef struct
{ /*  abc2    */
    real r, th;
}
ABC2;

typedef struct
{ /*  satur   */
    int iphase;
    real Dliq, Dvap, DH2O;
}
SATUR;

typedef struct
{ /* qqqq */
    real q0, q5, q6;   // residual pressure, its first and second derivatives with respect to the density
}
QQQQ;

typedef struct
{ /*  fcts  */
    real ad, gd, sd, ud, hd, cvd, cpd,
    dpdt, dvdt, dpdd, cjtt, cjth, gam;
}
FCTS;

typedef struct
{ /*  basef */
    real ab, gb, sb, ub, hb, cvb, pb, dpdtb;
}
BASEF;

typedef struct
{ /*  resf */
    real ar, gr, sr, ur, hr, cvr, dpdtr;
}
RESF;

typedef struct
{ /*  idf  */
    real ai, gi, si, ui, hi, cvi, cpi;
}
IDF;

typedef struct
{ /*  abc1,  abc3,   RTcurr  */
    real dPdM, dPdTcd, rt;
}
ABC1;

typedef struct
{ /* param  */
    real r1, th1;
}
PARAM;

typedef struct
{ /*  deri2  */
    real dPdD, dPdT;
}
DERI2;

typedef struct
{ /*  deriv  */
    real amu, s[2], sd[2], Pw, Tw, dTw,
    dM0dT, dP0dT, d2PdM2, d2PdMT,
    d2PdT2, p0th, p1th, xk[2];
}
DERIV;

typedef struct
{ /*  tpoint   */
    real Utri, Stri, Htri, Atri, Gtri,
    Ttr, Ptripl, Dltrip, Dvtrip;
}
TPOINT;

typedef struct
{ /*  aconst   */
    real wm, gascon, tz, aa, uref, sref, zb, dzb, yb;
}
ACONST;

typedef struct
{ /*  nconst  */
    real g[40];
    int    ii[40], jj[40], nc;
}
NCONST;

typedef struct
{ /*  addcon   */
    real atz[4], adz[4], aat[4], aad[4];
}
ADDCON;

typedef struct
{ /*  ellcon   */
    real g1, g2, gf, b1, b2, b1t, b2t, b1tt, b2tt;
}
ELLCON;

typedef struct
{ /*  bconst   */
    real bp[10], bq[10];
}
BCONST;

typedef struct
{ /*  crits    */
    real Tc, rhoC, Pc, Pcon, Ucon, Scon, dPcon;
}
CRITS;

typedef struct
{ /*  coefs   */
    real a[20], q[20], x[11];
}
COEFS;

typedef struct
{ /*  tolers   */
    real TTOL, PTOL, DTOL, XTOL, EXPTOL, FPTOL;
}
TOLERS;

typedef struct
{ /*  HGKbnd   */
    real Ttop, Tbtm, Ptop, Pbtm, Dtop, Dbtm;
}
HGK_BND;

typedef struct
{ /*  liqice   */
    real sDli1, sPli1, sDli37, sPli37,
    sDIB30, Tli13, Pli13, Dli13, TnIB30, DnIB30;
}
LIQICE;

typedef struct
{ /* io  */
    int iconf;
    real rterm, wterm, reacf, pronf, tabf, plotf;
}
IO_Y;

typedef struct
{ /*  HGKcrt   */
    real tcHGK, dcHGK, pcHGK;
}
HGK_CRT;

typedef struct
{ /*  therm  */
    real AE, GE, U, H, Entrop, Cp, Cv,
    betaw, alphw, heat, Speed, gamw;
}
THERM;

typedef struct
{// local values
    real T;
    real P;
    real Ps;
    real D;
    real Dv;
    real Dl;
    real delg;  /* (Gl-Gv)/RT                     */
}
TERM_PR;

struct STORE
{ /*  store   */
    int isav1;
    real sav2, sav3, sav4, sav5, sav6, sav7,
    sav8, sav9, sav10, sav11, sav12, sav13,
    sav14, sav15, sav16, sav17, sav18, sav19;
    STORE():isav1(0)
    {
        memset( &sav2, 0, sizeof(double)*18);
    }
};


/// The unit factors of the code for its fixed choice it = 1 (degC), id = 1 (g/cm3), ip = 1 (bar), ih = 4 (cal/mol)
inline auto gemsWaterUnits() -> UNITS
{
    UNITS u;
    u.ft = 1.0;
    u.fd = 1.0;
    u.fvd = 1.e1;
    u.fvk = 1.e4;
    u.fs = 1.e2;
    u.fp = 1.e1;
    u.fh = 4.305816e0;
    u.fst = 1.e3;
    u.fc = 1.e-2 * 0.23901e0;
    return u;
}

} // namespace ThermoFun

#endif
