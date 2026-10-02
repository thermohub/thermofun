#ifndef WATERHGKGEMS_H
#define WATERHGKGEMS_H
#include "ThermoProperties.h"
#include "Common/Real.hpp"

#include <memory>
#include <cstring>

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
    real q0, q5;
}
QQQQ;

typedef struct
{ /*  fcts  */
    real ad, gd, sd, ud, hd, cvd, cpd,
    dpdt, dvdt, dpdd, cjtt, cjth;
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
    betaw, alphw, heat, Speed;
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

class WaterHGKgems
{
public:
    WaterHGKgems();

private:

    HGK_SPECS aSpc; HGK_STATES aSta;

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
    THERM th;
    PARAM par;
    DERI2 d2;
    DERIV dv;
    IO_Y io;
    //
    ACONST   *ac;
    NCONST   *nc;
    ELLCON   *el;
    BCONST   *bcn;
    ADDCON   *ad;
    HGK_CRT  *hc;
    TOLERS   *to;
    HGK_BND  *hb;
    LIQICE   *li;
    TPOINT   *tt;
    CRITS    *cr;
    COEFS    *co;

    auto unit(int it, int id, int ip, int ih, int itripl, WaterTripleProperties wtr) -> void;
    auto tpset(WaterTripleProperties wtr ) -> void;
    auto valid(int it, int id, int ip, int ih, int itripl, int isat,
               int iopt, int useLVS, int epseqn, real Temp, real *Pres,
               real *Dens0, int *eR) -> void;
    auto valspc(int it, int id, int ip, int ih, int itripl,
                int isat, int iopt, int useLVS, int epseqn) -> int;
    auto valTD(real T, real D, int isat, int epseqn) -> int;
    auto pcorr(int itripl, real t, real *p, real *dL,real *dV, int epseqn) -> void;
    auto PsHGK(real t) -> real;
    auto corr(int itripl, real t, real *p, real *dL,
              real *dV, real *delg, int epseqn) -> void;
    auto bb(real t) -> void;
    auto resid(real t, real *d) -> void;
    auto base(real *d, real t) -> void;
    auto denHGK(real *d, real *p, real dguess, real t, real *dpdd) -> void;
    auto ideal(real t) -> void;
    auto thmHGK(real *d, real t) -> void;
    auto dalHGK(real *d, real t, real alpha) -> real;
    auto viscos(real Tk, real Pbars, real Dkgm3, real betaPa) -> real;
    auto thcond(real Tk, real Pbars, real Dkgm3, real alph, real betaPa) -> real;
    auto surten(real Tsatur) -> real;
    auto JN91(real T, real D, real beta, real *alpha, real *daldT, real *eps, real *dedP, real *dedT, real *d2edT2) -> void;
    auto epsBrn(real *eps, real dedP, real dedT,real d2edT2,
                         real *Z, real *Q, real *Y, real *X) -> void;
    auto Born92(real TK, real Pbars, real Dgcm3, real betab,
                         real *alphaK, real *daldT, real *eps, real *Z,
                         real *Q, real *Y, real *X, int epseqn) -> void;
    auto triple(real T, WPROPS  *wr) -> void;
    auto dimHGK(int isat,int itripl, real t, real *p, real *d, int epseqn) -> void;
    auto crtreg(int isat, int iopt, int it, real *T, real *P,
                real *D, int *eR) -> void;
    auto LVSeqn(int isat, int iopt, int itripl, real TC,
                real *P, real *Dens0, int epseqn) -> void;
    auto HGKeqn(int isat, int iopt, int itripl, real Temp,
                real *Pres, real *Dens0, int epseqn) ->void;
    auto valTP(real T, real P) -> int;
    auto Pfind(int isat, real T, real DD) -> real;
    auto conver(real *rho,
                          real Tee, real rho1s, real *rhodi, real error1) -> void;
    auto ss(real r, real th, real *s, real *sd) -> void;
    auto rtheta(real *r, real *theta, real rho, real Tee) -> void;
    auto aux(real r1, real th1, real *d2PdT2, real *d2PdMT,
                       real *d2PdM2, real aa, real *xk, real *sd, real Cvcoex) -> void;
    auto denLVS(int isat, real T, real P) -> void;
    auto Psublm(real Temp) -> real;
    auto TsLVS(int isat, real Pres) -> real;
    auto LVSsat(int iopt, int isat, real *T, real *P, real *D)  -> void;
    auto thmLVS(int isat, real T, real r1, real th1)  -> void;
    auto dalLVS(real D, real T, real P, real alpha) -> real;
    auto dimLVS(int isat, int itripl, real theta, real T, real *Pbars,
                         real *dL, real *dV, WPROPS *www, int epseqn) -> void;
    auto cpswap() -> void;
    auto HGKsat(int& isat, int iopt, int itripl, real Temp,
                         real *Pres, real *Dens, int epseqn) -> void;
    auto calcv3(int iopt, int itripl, real Temp, real *Pres,
                         real *Dens, int epseqn) -> void;
    auto calcv2(int iopt, int itripl, real Temp, real *Pres,
                         real *Dens, int epseqn) -> void;
    auto TdegUS(int it, real t) -> real;
    auto tcorr(int itripl, real *t, real *p, real *dL, real *dV,
                        int epseqn) -> void;
    auto TdPsdT(real t_) -> real;
    auto TsHGK(real Ps_) -> real;
    auto TdegK(int it, real t) -> real;
    auto errorHKFH2OValidity(std::string type, real P, real T, std::string name, int line) -> void;
    auto backup( struct STORE &sto ) -> void;
    auto restor( struct STORE sto ) -> void;

public:

    /// Calculate the properties of water; T in degrees Celsius and P in bar (changed to the saturation pressure if it is zero)
    auto calculateWaterHGKgems(real T, real &P, WaterTripleProperties wtr) -> void;

    auto propertiesWaterHGKgems(int state) -> PropertiesSolventAD;

    auto thermoPropertiesWaterHGKgems(int state) -> ThermoPropertiesSubstanceAD;

    auto electroPropertiesWaterJNgems(int state) -> ElectroPropertiesSolventAD;

};

}

#endif // WATERHGKGEMS_H
