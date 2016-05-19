#ifndef WATERHGK_H
#define WATERHGK_H

// TCorrPT includes
//#include "ThermoModelsSolvent.h"
//#include "ThermoProperties.h"

namespace TCorrPT {

// Forward declarations
struct ThermoPropertiesSolvent;

typedef struct
{ //work structure  t/d properties of water-solution
    // ( see at WATERPARAM)
    double Aw, Gw, Sw, Uw, Hw, Cvw, Cpw, Speedw, Alphaw,
    Betaw, Dielw, Viscw, Tcondw, Surtenw, Tdiffw,
    Prndtlw, Visckw, Albew;
    double ZBorn, YBorn, QBorn, dAldT, XBorn;
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
};

typedef struct
{ // set local parametres to fact scales
    double ft;
    double fd;
    double fvd;
    double fvk;
    double fs;
    double fp;
    double fh;
    double fst;
    double fc;
}
UNITS;

typedef struct
{ /*  abc2    */
    double r, th;
}
ABC2;

typedef struct
{ /*  satur   */
    int iphase;
    double Dliq, Dvap, DH2O;
}
SATUR;

typedef struct
{ /* qqqq */
    double q0, q5;
}
QQQQ;

typedef struct
{ /*  fcts  */
    double ad, gd, sd, ud, hd, cvd, cpd,
    dpdt, dvdt, dpdd, cjtt, cjth;
}
FCTS;

typedef struct
{ /*  basef */
    double ab, gb, sb, ub, hb, cvb, pb, dpdtb;
}
BASEF;

typedef struct
{ /*  resf */
    double ar, gr, sr, ur, hr, cvr, dpdtr;
}
RESF;

typedef struct
{ /*  idf  */
    double ai, gi, si, ui, hi, cvi, cpi;
}
IDF;

typedef struct
{ /*  abc1,  abc3,   RTcurr  */
    double dPdM, dPdTcd, rt;
}
ABC1;

typedef struct
{ /* param  */
    double r1, th1;
}
PARAM;

typedef struct
{ /*  deri2  */
    double dPdD, dPdT;
}
DERI2;

typedef struct
{ /*  deriv  */
    double amu, s[2], sd[2], Pw, Tw, dTw,
    dM0dT, dP0dT, d2PdM2, d2PdMT,
    d2PdT2, p0th, p1th, xk[2];
}
DERIV;

typedef struct
{ /*  tpoint   */
    double Utri, Stri, Htri, Atri, Gtri,
    Ttr, Ptripl, Dltrip, Dvtrip;
}
TPOINT;

typedef struct
{ /*  aconst   */
    double wm, gascon, tz, aa, uref, sref, zb, dzb, yb;
}
ACONST;

typedef struct
{ /*  nconst  */
    double g[40];
    int    ii[40], jj[40], nc;
}
NCONST;

typedef struct
{ /*  addcon   */
    double atz[4], adz[4], aat[4], aad[4];
}
ADDCON;

typedef struct
{ /*  ellcon   */
    double g1, g2, gf, b1, b2, b1t, b2t, b1tt, b2tt;
}
ELLCON;

typedef struct
{ /*  bconst   */
    double bp[10], bq[10];
}
BCONST;

typedef struct
{ /*  crits    */
    double Tc, rhoC, Pc, Pcon, Ucon, Scon, dPcon;
}
CRITS;

typedef struct
{ /*  coefs   */
    double a[20], q[20], x[11];
}
COEFS;

typedef struct
{ /*  tolers   */
    double TTOL, PTOL, DTOL, XTOL, EXPTOL, FPTOL;
}
TOLERS;

typedef struct
{ /*  HGKbnd   */
    double Ttop, Tbtm, Ptop, Pbtm, Dtop, Dbtm;
}
HGK_BND;

typedef struct
{ /*  liqice   */
    double sDli1, sPli1, sDli37, sPli37,
    sDIB30, Tli13, Pli13, Dli13, TnIB30, DnIB30;
}
LIQICE;

typedef struct
{// local values
    double T;
    double P;
    double Ps;
    double D;
    double Dv;
    double Dl;
    double delg;  /* (Gl-Gv)/RT                     */
}
TERM_PR;


struct  HGK_GEMS_PARAM // description of thermodinamic properties of water
{
    // working parametera
    // WLIQUID wl;
    WPROPS wl;
    WPROPS wr;
    ABC2 a2;
    ABC1 a1;
    TERM_PR trp;
    UNITS un;
    SATUR sa;
    QQQQ qq;
    FCTS fct;
    BASEF ba;
    RESF res;
    IDF id;
//    THERM th;
    PARAM par;
    DERI2 d2;
    DERIV dv;
//    IO_Y io;
    //
    ACONST   ac;
    NCONST   nc;
    ELLCON   el;
    BCONST   bcn;
    ADDCON   ad;
//    HGK_CRT  *hc;
    TOLERS   to;
    HGK_BND  hb;
    LIQICE   li;
    TPOINT   tt;
    CRITS    cr;
    COEFS    co;
};

auto WaterHGKProperties (double T, double &P) -> ThermoPropertiesSolvent;

auto unit(HGK_SPECS aSpc, HGK_GEMS_PARAM &hgk_gems_param) -> void;

auto tpset(HGK_GEMS_PARAM &hgk_param) -> void;

auto valid(HGK_SPECS aSpc, HGK_GEMS_PARAM &hgk_param, double T, double &P, double &Dens, int &eR) -> void;

auto valspc(HGK_SPECS aSpc) -> int;

auto TdegK(int it, double t) -> double;

auto valTD(double T, double D, HGK_GEMS_PARAM &hgk_param, HGK_SPECS &aSpc, int epseqn) -> int;

auto pcorr(int itripl, double t, double &p, double &dL, double &dV, int epseqn, HGK_GEMS_PARAM &hgk_param) -> void;

auto PsHGK(double t) -> double;

auto corr(int itripl, double t, double &p, double &dL, double &dV, double &delg, int epseqn, HGK_GEMS_PARAM &hgk_param) -> void;

auto bb(double t, HGK_GEMS_PARAM &hgk_param) -> void;

auto resid(double t, double &d, HGK_GEMS_PARAM &hgk_param) -> void;

auto base(double &d, double t, HGK_GEMS_PARAM &hgk_param) -> void;

auto denHGK(double &d, double &p, double dguess, double t, double &dpdd, HGK_GEMS_PARAM &hgk_param) -> void;

auto ideal(double t, HGK_GEMS_PARAM &hgk_param) -> void;

auto thmHGK(double &d, double t, HGK_GEMS_PARAM &hgk_param) -> void;

auto dalHGK(double &d, double t, double alpha, HGK_GEMS_PARAM &hgk_param) -> double;

auto viscos(double Tk, double Pbars, double Dkgm3, double betaPa) -> double;

auto thcond(double Tk, double Pbars, double Dkgm3, double alph, double betaPa) -> double;

auto surten(double Tsatur) -> double;

auto JN91(double T, double D, double beta, double &alpha, double &daldT, double &eps, double &dedP, double &dedT, double &d2edT2) -> void;

auto epsBrn(double &eps, double dedP, double dedT,double d2edT2,
                     double &Z, double &Q, double &Y, double &X) -> void;

auto Born92(double TK, double Pbars, double Dgcm3, double betab,
                     double &alphaK, double &daldT, double &eps, double &Z,
                     double &Q, double &Y, double &X, int epseqn) -> void;

auto triple(double T, WPROPS  &wr, HGK_GEMS_PARAM &hgk_param) -> void;

auto dimHGK(int isat, int itripl, double t, double &p, double &d, int epseqn, HGK_GEMS_PARAM &hgk_param) -> void;







} // namespace TCorrPT

#endif // WATERHGK_H
