#ifndef WATERHGKGEMS_H
#define WATERHGKGEMS_H
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/Real.hpp>

#include <ThermoFun/Model/Quantity.h>
#include "WaterGemsTypes.h"

#include <map>
#include <memory>
#include <string>
#include <cstring>

namespace ThermoFun {

struct WaterHGKgemsConstants;

/// The constants from the values of the method of water and of the dielectric constant, both with all their names.
auto makeWaterHGKgemsConstants(const std::map<std::string, model::Quantity>& water, const std::map<std::string, model::Quantity>& dielectric) -> std::shared_ptr<const WaterHGKgemsConstants>;

class WaterHGKgems
{
public:
    explicit WaterHGKgems(const WaterHGKgemsConstants& constants);

private:
    const WaterHGKgemsConstants* k_;

    HGK_SPECS aSpc; HGK_STATES aSta;

    WPROPS wr;  ///< the properties of the last call of dimHGK
    /// One phase at T and P: its properties and density (g/cm3). The liquid and the vapour are kept apart; the state of the
    /// record picks one. Above the critical temperature both are the one fluid.
    struct HGKPhase { WPROPS w; real d = 0.0; };
    HGKPhase liq_, vap_;
    ABC2 a2;
    ABC1 a1;
    TERM_PR trp;
    /// The last saturation of pcorr in this object (one object is one calculation at one T)
    struct { bool valid = false; int itripl = 0; double t = 0.0; real p, dL, dV, delg; } pcorr_cache;
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
    // the constants of this object: it also writes to some of them
    ACONST ac_value;
    NCONST nc_value;
    ELLCON el_value;
    BCONST bc_value;
    ADDCON ad_value;
    HGK_CRT hc_value;
    HGK_BND hb_value;
    LIQICE li_value;
    TPOINT tt_value;
    CRITS cr_value;

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

    auto unit(int it, int id, int ip, int ih, int itripl, WaterTripleProperties wtr) -> void;
    auto tpset(WaterTripleProperties wtr ) -> void;
    auto valid(int it, int id, int ip, int ih, int itripl, int isat,
               int iopt, int useLVS, int epseqn, real Temp, real *Pres,
               real *Dens0, int *eR) -> void;
    auto valspc(int it, int id, int ip, int ih, int itripl,
                int isat, int iopt, int useLVS, int epseqn) -> int;
    auto valTD(real T, real D, int isat, int epseqn) -> int;
    auto pcorr(int itripl, real t, real *p, real *dL,real *dV, int epseqn) -> void;
    auto stateHGK(real t, real d, real& P, real& dPdd, real& g) -> void;
    auto coexistenceHGK(real t, real *p, real *dL, real *dV) -> void;
    auto coexistenceNewton(real t, real& dl, real& dv) -> bool;
    auto PsHGK(real t) -> real;
    auto corr(int itripl, real t, real *p, real *dL,
              real *dV, real *delg, int epseqn) -> void;
    auto bb(real t) -> void;
    auto resid(real t, real *d) -> void;
    auto base(real *d, real t) -> void;
    /// The density d (g/cm3) at which P(d) = p, from dguess; false if the iteration did not converge.
    auto denHGK(real *d, real *p, real dguess, real t, real *dpdd) -> bool;
    /// The root of a branch from its guess; if there is none (P beyond the spinodal of the branch), the root from the other guess.
    auto rootHGK(real *p, real t, real guess, real other) -> real;
    /// The call was at saturation: the derivative of the caller's P (kept apart), the temperature, for the pressure derivatives
    bool saturated_ = false;
    real dP_pass_ = 0.0;
    real T_sat_ = 0.0;
    real P_sat_ = 0.0;
    /// True if both densities of the last corr() converged
    bool corr_converged_ = true;
    /// The properties at density d (g/cm3), all parts computed at d.
    auto phaseHGK(int isat, int itripl, real t, real *p, real d, int epseqn) -> WPROPS;
    auto ideal(real t) -> void;
    auto thmHGK(real *d, real t) -> void;
    auto dalHGK(real *d, real t, real alpha) -> real;
    auto JN91(real T, real D, real beta, real *alpha, real *daldT, real *eps, real *dedP, real *dedT, real *d2edT2) -> void;
    auto epsBrn(real *eps, real dedP, real dedT,real d2edT2,
                         real *Z, real *Q, real *Y, real *X) -> void;
    auto Born92(real TK, real Pbars, real Dgcm3, real betab,
                         real *alphaK, real *daldT, real *eps, real *Z,
                         real *Q, real *Y, real *X, int epseqn) -> void;
    auto triple(real T, WPROPS  *wr) -> void;
    auto dimHGK(int isat,int itripl, real t, real *p, real *d, int epseqn) -> void;
    auto HGKeqn(int isat, int iopt, int itripl, real Temp,
                real *Pres, real *Dens0, int epseqn) ->void;
    auto valTP(real T, real P) -> int;
    auto Psublm(real Temp) -> real;
    auto HGKsat(int& isat, int iopt, int itripl, real Temp,
                         real *Pres, real *Dens, int epseqn) -> void;
    auto TdegUS(int it, real t) -> real;
    auto tcorr(int itripl, real *t, real *p, real *dL, real *dV,
                        int epseqn) -> void;
    auto TdPsdT(real t_) -> real;
    auto TsHGK(real Ps_) -> real;
    auto TdegK(int it, real t) -> real;
    auto errorHKFH2OValidity(std::string type, real P, real T, std::string name, int line) -> void;

public:

    /// Calculate the properties of water; T in degrees Celsius and P in bar (changed to the saturation pressure if it is zero)
    auto calculateWaterHGKgems(real T, real &P, WaterTripleProperties wtr) -> void;
    /// The saturation pressure of the model at T (deg C), bar: that of the calculation at saturation.
    auto modelSaturationPressure(real T, WaterTripleProperties wtr) const -> real;

    auto propertiesWaterHGKgems(int state) -> PropertiesSolventAD;

    auto thermoPropertiesWaterHGKgems(int state) -> ThermoPropertiesSubstanceAD;

    /// From the properties of the water of the record: density in g/cm3, compressibility in 1/bar, expansion in 1/K and its T derivative.
    auto electroPropertiesWaterJNgems(real T, real Pbars, real densityGcm3, real betaBar, real alpha, real daldT) -> ElectroPropertiesSolventAD;

};

}

#endif // WATERHGKGEMS_H
