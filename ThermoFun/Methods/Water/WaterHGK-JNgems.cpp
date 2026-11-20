// ThermoFun includes
#include "Common/Exception.h"
#include "WaterHGK-JNgems.h"
#include <cmath>
#include <limits>
#include "WaterTransportGems.h"

#include <string>
#include <type_traits>

#include <ThermoFun/Model/BuiltinParameterSets.h>
#include <ThermoFun/Common/ThermoProperties.h>
#include <ThermoFun/Common/Constants.h>

namespace ThermoFun {


auto WaterHGKgems::thermoPropertiesWaterHGKgems(int state) -> ThermoPropertiesSubstanceAD
{
    ThermoPropertiesSubstanceAD wp;
    const HGKPhase& ph = (state == 1) ? vap_ : liq_;  // the state of the record: 0 liquid, 1 vapour

    wp.helmholtz_energy         = ph.w.Aw * cal_to_J;
    wp.gibbs_energy             = ph.w.Gw * cal_to_J;
    wp.entropy                  = ph.w.Sw * cal_to_J;
    wp.internal_energy          = ph.w.Uw * cal_to_J;
    wp.enthalpy                 = ph.w.Hw * cal_to_J;
    wp.heat_capacity_cv         = ph.w.Cvw * cal_to_J;
    wp.heat_capacity_cp         = ph.w.Cpw * cal_to_J;
    wp.volume                   = 1/ph.d * gemsWaterMolarMass() * cm3_mol_to_J_bar;

    if (saturated_)
    {
        // The pressure derivatives at constant T of the phase (P in bar, V in J/bar): dG/dP = V, dS/dP = -V alpha,
        // dH/dP = V (1 - alpha T), dV/dP = -V beta, dCp/dP = -T V (alpha^2 + dalpha/dT); U = H - PV, A = G - PV.
        // The heat capacity Cv has no pressure derivative here (it stays 0).
        const real V = wp.volume;
        const real alpha = ph.w.Alphaw, beta = ph.w.Betaw, dal = ph.w.dAldT;
        const real Pbar = P_sat_;
        const real T = T_sat_;
        const real dp = dP_pass_;
        wp.gibbs_energy = wp.gibbs_energy + V * dp;
        wp.entropy = wp.entropy - V * alpha * dp;
        wp.enthalpy = wp.enthalpy + V * (1 - alpha * T) * dp;
        wp.heat_capacity_cp = wp.heat_capacity_cp - T * V * (alpha * alpha + dal) * dp;
        wp.internal_energy = wp.internal_energy + V * (Pbar * beta - alpha * T) * dp;
        wp.helmholtz_energy = wp.helmholtz_energy + Pbar * V * beta * dp;
        wp.volume = wp.volume - V * beta * dp;
    }

    return wp;
}

auto WaterHGKgems::propertiesWaterHGKgems(int state) -> PropertiesSolventAD
{
    PropertiesSolventAD wp;
    const HGKPhase& ph = (state == 1) ? vap_ : liq_;  // the state of the record: 0 liquid, 1 vapour

    wp.surface_tension   = ph.w.Surtenw.val();
    wp.Alpha    = ph.w.Alphaw;
    wp.Beta     = ph.w.Betaw / 1e05; // from bar-1 to Pa-1
    wp.thermal_conductivity    = ph.w.Tcondw.val();
    wp.Tdiff    = ph.w.Tdiffw.val();
    wp.Prndtl   = ph.w.Prndtlw.val();
    wp.dAldT    = ph.w.dAldT;
    wp.Albe     = ph.w.Albew;
    wp.speed_of_sound    = ph.w.Speedw.val();
    wp.dynamic_viscosity     = ph.w.Viscw.val();
    wp.Visck    = ph.w.Visckw.val();
    wp.density  = ph.d * 1000; // in kg/m3

    const real rho = ph.d * 1000;
    const real alp = ph.w.Alphaw;
    const real dal = ph.w.dAldT;
    const real bet = ph.w.Betaw / 1e05; // from bar-1 to Pa-1
    const real gam = ph.w.Gamw;

    wp.densityT = - alp * rho;
    wp.densityTT = rho * ( pow(alp,2.) - dal );
    wp.densityP = bet * rho;
    // d2rho/dP2 = -rho beta^2 Gamma, Gamma = rho (d2P/drho2)/(dP/drho) at constant T (analytical, LVS and HGK)
    wp.densityPP = - rho * bet * bet * gam;
    wp.pressure = aSta.Pres*1e5;

return wp;
}

auto WaterHGKgems::electroPropertiesWaterJNgems(real T, real Pbars, real densityGcm3, real betaBar, real alpha, real daldT) -> ElectroPropertiesSolventAD
{
    // as calculateWaterHGKgems does for the water, between 0 and 0.01 degC
    if (T - 273.15 < 0.01 && T - 273.15 >= 0.0)
        T = real(0.01 + 273.15);
    real eps = 0.0, Z = 0.0, Q = 0.0, Y = 0.0, X = 0.0;
    Born92(T, Pbars, densityGcm3, betaBar, &alpha, &daldT, &eps, &Z, &Q, &Y, &X, 4);
    ElectroPropertiesSolventAD wp;
    wp.epsilon   = eps;
    wp.bornZ     = Z;
    wp.bornY     = Y;
    wp.bornQ     = Q / 1e05; // from bar-1 to Pa-1
    wp.bornX     = X;
    wp.epsilonT  = Y * pow(eps,2.);
    wp.epsilonTT = (X + 2.*eps*pow(Y,2.)) * pow(eps,2.);
    wp.epsilonP  = Q * pow(eps,2.);
    return wp;
}



/* HGKcon - Constant parameters for the H2O equation of state given by
*           Haar,  Gallagher, & Kell (1984):  bp, bq = b(j), B(j) from
*           Table A.1, p.272; g1, g2, gf = alpha, beta, gamma  from eq
*           (A.2), p.272; g, ii, jj  = g(i), k(i), l(i) from eq (A.5),
*           p.272. Note that  tz < tcHGK.
*                 Tolerence limits required  in various real & inexact
*            comparisons are set and stored in COMMON /tolers/.     */


                                           /*  nc    */

           /* bq[10] */

 /* aad[4]  */

/// The constants of the model: from Methods/ParameterSets/WaterHGK84-gems.yaml and JohnsonNorton91.yaml, or the record. A calculation copies those of the structs, which it also writes to.
struct WaterHGKgemsConstants
{
    ACONST ac{};
    NCONST nc{};
    ELLCON el{};
    BCONST bc{};
    ADDCON ad{};
    HGK_CRT hc{};
    HGK_BND hb{};
    LIQICE li{};
    TPOINT tt{};
    CRITS cr{};
    double psat_a[8] = {};
    double ideal_c[18] = {};
    WaterTransportGemsConstants transport;
    double jn_a[10] = {};
    double jn_Tr = 0.0;
    double born_TMAX = 0.0, born_PMAX = 0.0;
    double jn_Dr = 0.0;
};

namespace {

auto readConstants(const std::map<std::string, model::Quantity>& values, const std::map<std::string, model::Quantity>& dielectric) -> WaterHGKgemsConstants
{
    WaterHGKgemsConstants out;
    auto number = [&](const std::string& name) { return values.at(name).value; };
    auto one = [&](auto& to, const std::string& name) { to = static_cast<std::decay_t<decltype(to)>>(number(name)); };
    // an array of one or two dimensions, as many values as the file has
    auto list = [&](auto& to, const std::string& name) {
        using Row = std::remove_reference_t<decltype(to[0])>;
        if constexpr (std::is_array_v<Row>)
        {
            const auto columns = std::extent_v<Row>;
            for (std::size_t i = 0; i < std::extent_v<std::remove_reference_t<decltype(to)>> * columns; ++i)
                if (const auto it = values.find(name + "_" + std::to_string(i + 1)); it != values.end())
                    to[i / columns][i % columns] = it->second.value;
        }
        else
        {
            for (std::size_t i = 0; i < std::extent_v<std::remove_reference_t<decltype(to)>>; ++i)
                if (const auto it = values.find(name + "_" + std::to_string(i + 1)); it != values.end())
                    to[i] = static_cast<Row>(it->second.value);
        }
    };
    one(out.ac.wm, "ac_wm");
    one(out.ac.gascon, "ac_gascon");
    one(out.ac.tz, "ac_tz");
    one(out.ac.aa, "ac_aa");
    one(out.ac.uref, "ac_uref");
    one(out.ac.sref, "ac_sref");
    one(out.ac.zb, "ac_zb");
    one(out.ac.dzb, "ac_dzb");
    one(out.ac.yb, "ac_yb");
    list(out.nc.g, "nc_g");
    list(out.nc.ii, "nc_ii");
    list(out.nc.jj, "nc_jj");
    one(out.nc.nc, "nc_nc");
    one(out.el.g1, "el_g1");
    one(out.el.g2, "el_g2");
    one(out.el.gf, "el_gf");
    one(out.el.b1, "el_b1");
    one(out.el.b2, "el_b2");
    one(out.el.b1t, "el_b1t");
    one(out.el.b2t, "el_b2t");
    one(out.el.b1tt, "el_b1tt");
    one(out.el.b2tt, "el_b2tt");
    list(out.bc.bp, "bc_bp");
    list(out.bc.bq, "bc_bq");
    list(out.ad.atz, "ad_atz");
    list(out.ad.adz, "ad_adz");
    list(out.ad.aat, "ad_aat");
    list(out.ad.aad, "ad_aad");
    one(out.hc.tcHGK, "hc_tcHGK");
    one(out.hc.dcHGK, "hc_dcHGK");
    one(out.hc.pcHGK, "hc_pcHGK");
    one(out.hb.Ttop, "hb_Ttop");
    one(out.hb.Tbtm, "hb_Tbtm");
    one(out.hb.Ptop, "hb_Ptop");
    one(out.hb.Pbtm, "hb_Pbtm");
    one(out.hb.Dtop, "hb_Dtop");
    one(out.hb.Dbtm, "hb_Dbtm");
    one(out.li.sDli1, "li_sDli1");
    one(out.li.sPli1, "li_sPli1");
    one(out.li.sDli37, "li_sDli37");
    one(out.li.sPli37, "li_sPli37");
    one(out.li.sDIB30, "li_sDIB30");
    one(out.li.Tli13, "li_Tli13");
    one(out.li.Pli13, "li_Pli13");
    one(out.li.Dli13, "li_Dli13");
    one(out.li.TnIB30, "li_TnIB30");
    one(out.li.DnIB30, "li_DnIB30");
    one(out.tt.Utri, "tt_Utri");
    one(out.tt.Stri, "tt_Stri");
    one(out.tt.Htri, "tt_Htri");
    one(out.tt.Atri, "tt_Atri");
    one(out.tt.Gtri, "tt_Gtri");
    one(out.tt.Ttr, "tt_Ttr");
    one(out.tt.Ptripl, "tt_Ptripl");
    one(out.tt.Dltrip, "tt_Dltrip");
    one(out.tt.Dvtrip, "tt_Dvtrip");
    one(out.cr.Tc, "cr_Tc");
    one(out.cr.rhoC, "cr_rhoC");
    one(out.cr.Pc, "cr_Pc");
    one(out.cr.Pcon, "cr_Pcon");
    one(out.cr.Ucon, "cr_Ucon");
    one(out.cr.Scon, "cr_Scon");
    one(out.cr.dPcon, "cr_dPcon");
    list(out.psat_a, "psat_a");
    list(out.ideal_c, "ideal_c");
    for (std::size_t i = 0; i < 10; ++i)
        out.jn_a[i] = dielectric.at("a" + std::to_string(i + 1)).value;
    one(out.born_TMAX, "born_TMAX");
    one(out.born_PMAX, "born_PMAX");
    out.transport = readWaterTransportGemsConstants(values);
    out.jn_Tr = dielectric.at("Tr").value;
    out.jn_Dr = dielectric.at("Dr").value;
    return out;
}

} // namespace

auto makeWaterHGKgemsConstants(const std::map<std::string, model::Quantity>& water, const std::map<std::string, model::Quantity>& dielectric) -> std::shared_ptr<const WaterHGKgemsConstants>
{
    return std::make_shared<const WaterHGKgemsConstants>(readConstants(water, dielectric));
}

TOLERS  to_ = { /* TTOL, PTOL, DTOL, XTOL, EXPTOL, FPTOL */
    1.0e-6, 1.0e-6, 1.0e-9, 1.0e-5, -673.5e0, 1.0e-7};

     /* x[11] */

auto johnsonNorton91Range(const WaterHGKgemsConstants& constants) -> std::pair<double, double>
{
    return {constants.born_TMAX, constants.born_PMAX};
}

auto WaterHGKgems::errorHKFH2OValidity(std::string type, real P, real T, std::string name, int line) -> void
{
    Exception exception;
    exception.error << "Error HKF H2O: at " << type << ".";
    exception.reason << "entered values of T: "<< T.val() << " & "<< P.val() << name;
    exception.line = line;
    RaiseError(exception);
}

auto  WaterHGKgems::TdegK(int it, real t) -> real
{
    real TdegK=t;
    switch(it)
    {
    case 0:
        TdegK = t;
        break;
    case 1:
        TdegK = t + 273.15e0;
        break;
    case 2:
        TdegK = t / 1.8e0;
        break;
    case 3:
        TdegK = (t + 459.67e0) / 1.8e0;
        break;
    }
    return(TdegK);
}

WaterHGKgems::WaterHGKgems(const WaterHGKgemsConstants& constants) : k_(&constants)
{
    // set global constant
    //  npH2O++;
    const auto& k = *k_;
    ac_value = k.ac;
    nc_value = k.nc;
    el_value = k.el;
    bc_value = k.bc;
    ad_value = k.ad;
    hc_value = k.hc;
    hb_value = k.hb;
    li_value = k.li;
    tt_value = k.tt;
    cr_value = k.cr;
    ac = &ac_value;
    nc = &nc_value;
    el = &el_value;
    bcn = &bc_value;
    ad = &ad_value;
    hc = &hc_value;
    to = &to_;
    hb = &hb_value;
    li = &li_value;
    tt = &tt_value;
    cr = &cr_value;
}

auto WaterHGKgems::calculateWaterHGKgems(real T, real &P, WaterTripleProperties wtr) -> void
{
    int eR;
    real tempy;
    real Tk = T + 273.15;
    saturated_ = false;
    const double dP_in = P[1];  // the derivative of the caller's P (autodiff), before P is changed below

    if( T < 0.01 && T >= 0.0 ) // Deg. C!
        T = 0.01;
    if( P < 6.11732e-3 ) // 6.11732e-3 is P_sat at triple point of H2O in bar
        // At lower pressures HKF/HGK runs unstable or crashes
        P = 0.0;  // 06.12.2006  DK

    // in SUPCRT92 set type calculation of parametres
    aSpc.it=1;
    aSpc.id=1;
    aSpc.ip=1;
    aSpc.ih=4;
    aSpc.itripl=1; // 1 aparent molal properties convention; 0 steam tables convention

    // One saturation pressure: that of the model, the pressure that the calculation at saturation gives (HGK, LVS near the
    // critical point). The correlation PsHGK is only its first guess; it is up to 2e-5 lower, and a P between the two was vapour.
    // The model value is needed only near PsHGK: there it decides the branch and the phase, elsewhere PsHGK decides the same.
    if (Tk <= 647.067e0)
        aSta.Psat = PsHGK(Tk)*10;
    else
        aSta.Psat = 0;
    if (P != 0.0 && aSta.Psat > 0.0 && fabs(P - aSta.Psat) < 1e-3 * aSta.Psat)
        aSta.Psat = modelSaturationPressure(T, wtr);

    // P equal to Psat is the saturation branch too, to 1e-12 relative: a Psat that went to Pa and back (the engine writes it to
    // P, then calls again with it) differs by a few ulp.
    if( (fabs( P ) == 0) || (fabs(P - aSta.Psat) <= 1e-12 * fabs(aSta.Psat)) )
    { // set only T, for Psat
        aSpc.isat=1;
        aSpc.iopt=1;
        aSpc.metastable = 0;
        // At saturation P is a function of T alone, so the derivative of P of the caller is lost here. It is kept apart: the
        // properties of the phase at P + dP follow to first order from its own volume, expansivity and compressibility
        // (thermoPropertiesWaterHGKgems).
        saturated_ = true;
        dP_pass_ = real(0.0);
        dP_pass_[1] = dP_in;
        T_sat_ = Tk;
        P = PsHGK(Tk)*10;
        P_sat_ = P;
    }
    else
    { //set T and P
        aSpc.isat = 0;
        aSpc.iopt = 2;
        aSpc.metastable = 1;
    }
    aSpc.useLVS=1;
    aSpc.epseqn=4;
    aSta.Dens[1] = aSta.Dens[0] = 0.0;
    aSta.Pres = P;
    aSta.Temp = T;


    //
//    aSpc.isat = 1;
//    aSpc.iopt = 1;

    pcorr_cache.valid = false;
    liq_ = HGKPhase{};
    vap_ = HGKPhase{};
    memset( &wr, 0, sizeof(WPROPS));
    memset( &a2, 0, sizeof(ABC2));
    memset( &a1, 0, sizeof(ABC1));
    memset( &trp, 0, sizeof(TERM_PR));
    memset( &un, 0, sizeof(UNITS));
    memset( &sa, 0, sizeof(SATUR));
    memset( &qq, 0, sizeof(QQQQ));
    memset( &fct, 0, sizeof(FCTS));
    memset( &ba, 0, sizeof(BASEF));
    memset( &res, 0, sizeof(RESF));
    memset( &id, 0, sizeof(IDF));
    memset( &th, 0, sizeof(THERM));
    memset( &par, 0, sizeof(PARAM));
    memset( &d2, 0, sizeof(DERI2));
    memset( &dv, 0, sizeof(DERIV));
    memset( &io, 0, sizeof(IO_Y));

    //set local parameters with to fact scales
    unit(aSpc.it, aSpc.id, aSpc.ip, aSpc.ih, aSpc.itripl, wtr);

    // check insert data if Errir that trow
    valid(aSpc.it, aSpc.id, aSpc.ip, aSpc.ih, aSpc.itripl,
          aSpc.isat, aSpc.iopt, aSpc.useLVS, aSpc.epseqn,
          aSta.Temp, &aSta.Pres, &aSta.Dens[0], &eR);
    // T to K, P to MPa (crtreg did it). The region near the critical point is the method WaterLVS83.
    aSta.Temp = TdegK(aSpc.it, aSta.Temp);
    if (aSpc.iopt == 2)
        aSta.Pres /= un.fp;
    aSpc.icrit  = 0;
    aSpc.useLVS = 0;
    aSta.Dens[0] /= 1.0e3;
    HGKeqn(aSpc.isat, aSpc.iopt, aSpc.itripl,
           aSta.Temp, &aSta.Pres, &aSta.Dens[0], aSpc.epseqn);

//    prop_solvent = load(0);
//    if (aSpc.isat == 1)
//    {
//        tempy = aSta.Dens[0];
//        aSta.Dens[0] = aSta.Dens[1];
//        aSta.Dens[1] = tempy;
//        //        prop_solvent = load(1);
//    }
    // translate T - to users units
    aSta.Temp   = TdegUS(aSpc.it, aSta.Temp);
    aSta.Pres  *= un.fp;
    aSta.Dens[0] /= un.fd;

    if (aSpc.isat == 1)
    {
        aSta.Dens[0] /= un.fd;
        aSta.Dens[1] /= un.fd;
    }
    if (aSpc.isat == 1)
    {
        // HGK alone has no usable saturation in the last kelvin below its critical point (its liquid there lies on the
        // spinodal); SUPCRT92 and GEMS use the equation of Levelt Sengers et al. from 643 K
        if (!std::isfinite(aSta.Pres.val()))
        {
            Exception exception;
            exception.error << "WaterHGK84 (gems): no saturation pressure at " << Tk.val() << " K.";
            exception.reason << "Near the critical point the HGK equation has no usable saturation. List the method WaterLVS83 before WaterHGK84 in the record, as SUPCRT92 and GEMS do.";
            exception.line = __LINE__;
            RaiseError(exception);
        }
        P = aSta.Pres;  // the saturation pressure of the model
    }

}

auto WaterHGKgems::modelSaturationPressure(real T, WaterTripleProperties wtr) const -> real
{
    WaterHGKgems saturation(*k_);
    real P = 0.0;
    saturation.calculateWaterHGKgems(T, P, wtr);
    return P;
}

auto WaterHGKgems::unit(int it, int id, int ip, int ih, int itripl, WaterTripleProperties wtr) ->void
{
    real fft[4] =  {1.e0, 1.e0, 0.555555556e0, 0.555555556e0 };
    real ffd[4] = {1.e-3, 1.e0, 1.80152e-2, 1.6018e-2};
    real ffvd[4]= {1.e0, 1.e1, 0.555086816e0, 0.671968969e0 };
    real ffvk[4]= {1.e0, 1.e4, 1.0e4, 1.076391042e1 };
    real ffs[4] = {1.e0, 1.e2, 1.0e2, 3.280833e0 };
    real ffp[5] = {1.e0, 1.e1, 9.869232667e0, 1.45038e2, 1.01971e1};
    real ffh[6] = {1.e0, 1.e0, 1.80152e1, 2.3901e-1, 4.305816e0, 4.299226e-1};
    real ffst[4]= {1.e0, 1.e3, 0.555086816e2, 0.2205061e1 };
    real ffcd[4]= {1.e0, 1.e-2, 1.0e-2, 0.3048e0 };
    real ffch[6]= {1.e-3, 1.e0, 1.0e0, 0.23901e0, 0.23901e0, 0.947244e-3 };

    un.ft  = fft[it];
    un.fd  = ffd[id];
    un.fvd = ffvd[id];
    un.fvk = ffvk[id];
    un.fs  = ffs[id];
    un.fp  = ffp[ip];
    un.fh  = ffh[ih];
    un.fst = ffst[id];
    un.fc  = ffcd[id] * ffch[ih];
    if ( itripl == 1 )
        tpset(wtr);
}

//--------------------------------------------------------------------//
// valid insert data and scales for HKF
auto WaterHGKgems::valid(int it, int id, int ip, int ih, int itripl, int isat,
               int iopt, int useLVS, int epseqn, real Temp, real *Pres,
               real *Dens0, int *eR) -> void  /* real Temp, Pres, Dens; */
{
    real Ttripl, Tcrit, Pcrit; /*T, D, P, */
    // valid specification
    *eR = valspc(it, id, ip, ih, itripl, isat, iopt, useLVS, epseqn);

    if (!eR)
    {
        Exception exception;
        exception.error << "HKF H2O: Specification error.";
        exception.reason << "HGK controls and specifications were not set correctly.";
        exception.line = __LINE__;
        RaiseError(exception);
    }


    // translate to  degC, bars, g/cm3
    trp.T   = TdegK(it, Temp) - 273.15e0;
    trp.D   = *Dens0 * un.fd;
    trp.P   = *Pres / un.fp * 1.0e1;
    Ttripl  = tt->Ttr - 273.15e0;
    Tcrit   = cr->Tc - 273.15e0;
    Pcrit   = cr->Pc * 1.0e1;
    if (isat == 0)
    {
        if (iopt == 1)
        {
            *eR = valTD(trp.T, trp.D, isat, epseqn);
            if(!*eR)
            errorHKFH2OValidity("isat=0 & iopt=1", *Pres, Temp, " fall outside validity region of HGK EOS!", __LINE__);
        }
        else
        {
            *eR = valTP(trp.T, trp.P);
            if(!*eR)
            errorHKFH2OValidity("isat=0 & iopt=2", *Pres, Temp, " fall outside validity region of HGK EOS!", __LINE__);
        }
    }
    else
    {
        if (iopt == 1)
        {
            if ( !((trp.T + to->FPTOL  >= Ttripl) && (trp.T- to->FPTOL <= Tcrit)))
            {
                *eR = 0;
                errorHKFH2OValidity("isat=1 & iopt=1", *Pres, Temp, " input T falls outside the region between triple and critical points of H2O!\n"
                                                             " Please try with pressure P > 0.006 bar...", __LINE__);
            }
        }
        else
        {
            if ( !((trp.P+ to->FPTOL >= tt->Ptripl) && (trp.P - to->FPTOL <= Pcrit)))
            {
                *eR = 0;
                errorHKFH2OValidity("isat=1 & iopt=2", *Pres, Temp, " input P falls outside the region between triple and critical points of H2O!", __LINE__);
            }
        }
    }
}

auto WaterHGKgems::valspc(int it, int id, int ip, int ih, int itripl,
                    int isat, int iopt, int useLVS, int epseqn) -> int
{
    if ( !((0 <= it)     && (it     <= 3) &&
            (0 <= id)     && (id     <= 3) &&
            (0 <= ip)     && (ip     <= 4) &&
            (0 <= ih)     && (ih     <= 5) &&
            (0 <= itripl) && (itripl <= 1) &&
            (0 <= isat)   && (isat   <= 1) &&
            (1 <= iopt)   && (iopt   <= 2) &&
            (0 <= useLVS) && (useLVS <= 1) &&
            (1 <= epseqn) && (epseqn <= 5)))
        return(0);
    return(1);
}
auto WaterHGKgems::Psublm(real Temp) -> real
{
    real TT, PmmHg, Psublm;

    TT = Temp + 2.731e2;
    PmmHg = pow(1.0e1, (-2.4455646e3 / TT + 8.2312e0 * ln_to_lg * log(TT) -
                        1.677006e-2 * TT + 1.20514e-5 * TT * TT - 6.757169e0));
    /*** convert mmHg to bars ***/
    Psublm = PmmHg * 1.33322e-3;
    return(Psublm);
}

//--------------------------------------------------------------------//
int  WaterHGKgems::valTD(real T, real D, int /*isat*/, int epseqn)
{
    real Tcrit, Ttripl, Dlimit, Tk, /* Ps, Dl, Dv, */
    /*P,*/ PMPa, Dguess, Dsublm, dPdD;

    if ((T - to->FPTOL > hb->Ttop) || (T + to->FPTOL < hb->Tbtm) ||
            (D - to->FPTOL > hb->Dtop) || (D + to->FPTOL < hb->Dbtm))
        return(0);
    Tcrit = cr->Tc - 273.15e0;
    Ttripl = tt->Ttr - 273.15e0;
    if ((T + to->FPTOL >= Tcrit) || ((T >= li->TnIB30) && (D >= tt->Dltrip)))
    {
        Dlimit = li->sDIB30 * (T - li->TnIB30) + hb->Dtop;
        if ( !(D - to->FPTOL <= Dlimit ))
            return(0);
    }
    else
    {
        if (D - to->FPTOL <= tt->Dltrip)
        {
            if (T >= Ttripl)
            {
                Tk = T + 273.15e0;
                a1.rt = ac->gascon * Tk;
                pcorr(0,Tk,&trp.Ps,&trp.Dl,&trp.Dv,epseqn);
                if ((D >= trp.Dv) && (D <= trp.Dl))
                    aSpc.isat = 1; //isat = 1;
            }

            else
            {
                trp.P = Psublm(T);
                PMPa = trp.P / 1.0e1;
                Tk = T + 273.15e0;
                Dguess = PMPa / Tk / 0.4e0;
                a1.rt = ac->gascon * Tk;
                bb(Tk);
                denHGK(&Dsublm,&PMPa,Dguess,Tk,&dPdD);
                if ( !( D - to->FPTOL <= Dsublm ))
                    return(0);
            }
        }
        else
        {
            if (D <= li->Dli13)
            {
                Dlimit = li->sDli1 * (T - li->Tli13) + li->Dli13;
                if ( !( D + to->FPTOL >= Dlimit ))
                    return(0);
            }
            else
            {
                Dlimit = li->sDli37 * (T - li->Tli13) + li->Dli13;
                if ( !( D - to->FPTOL <= Dlimit ))
                    return(0);
            }
        }
    }
    return(1);
}

//--------------------------------------------------------------------//
// Return 1, if T-P define liquid or vaper  H2O in dimention field of HKF
// else 0.
auto  WaterHGKgems::valTP(real T, real P) -> int
{
    real  Plimit, Psubl;

    if ((T - to->FPTOL > hb->Ttop) || (T + to->FPTOL < hb->Tbtm) ||
            (P - to->FPTOL > hb->Ptop) || (P + to->FPTOL < hb->Pbtm))
        return(0);

    if (P >= li->Pli13)
    {
        Plimit = li->sPli37 * (T - li->Tli13) + li->Pli13;
        if ( !( P - to->FPTOL <= Plimit ))
            return(0);
    }
    else
    {
        if (P >= tt->Ptripl)
        {
            Plimit = li->sPli1 * (T - li->Tli13) + li->Pli13;
            if ( !( P +to->FPTOL >= Plimit ))
                return(0);
        }
        else
        {
            Psubl = Psublm(T);
            if ( !( P - to->FPTOL <= Psubl ))
                return(0);
        }
    }
    return(1);
}
auto WaterHGKgems::HGKsat(int& isat, int iopt, int itripl, real Temp,
                     real *Pres, real *Dens, int epseqn) -> void
{
    real  Ptemp=0., dltemp=0., dvtemp=0.;

    if (isat == 1)
    {
        if (iopt == 1)
            pcorr(itripl, Temp, Pres, Dens, &aSta.Dens[1], epseqn);
        else
            tcorr(itripl, &Temp, Pres, Dens, &aSta.Dens[1], epseqn);
    }
    else
    {
        if ((Temp > cr->Tc) || (Temp < tt->Ttr) ||
                ((iopt == 2) && (*Pres > cr->Pc)))
            return;                             /* ????????? */
        else
        {
            pcorr(itripl, Temp, &Ptemp, &dltemp, &dvtemp, epseqn);
            if (((iopt == 2) && (fabs(*Pres-Ptemp) <= to->PTOL)) ||
                    ((iopt == 1) && ((fabs(aSta.Dens[0]-dltemp) <= to->DTOL) ||
                                     (fabs(aSta.Dens[0] - dvtemp) <= to->DTOL))))
            {
                isat = 1;
                *Pres = Ptemp;
                aSta.Dens[0] = dltemp;
                aSta.Dens[1] = dvtemp;
            }
        }
    }
}

//--------------------------------------------------------------------//
// The properties of water at T and P by the equation of state of Haar, Gallagher and Kell (1984): the liquid and the vapour,
// each at its own density. At saturation the densities of pcorr. Elsewhere the root of each branch from its saturated
// density (the vapour from a nearly ideal gas), a metastable root if there is one, the other root beyond the spinodal of the
// branch; above tz one fluid root.
auto WaterHGKgems::HGKeqn(int isat, int iopt, int itripl, real Temp,
                     real *Pres, real *Dens0, int epseqn) -> void
{
    a1.rt = ac->gascon * Temp;
    bb(Temp);
    real dl = 0.0, dv = 0.0;
    if (isat == 1)
    {
        HGKsat(isat, iopt, itripl, Temp, Pres, Dens0, epseqn);
        dl = aSta.Dens[0];
        dv = aSta.Dens[1];
    }
    else
    {
        real ps = 0.0, dll = 0.0, dvv = 0.0;
        if (Temp < ac->tz)
            pcorr(itripl, Temp, &ps, &dll, &dvv, epseqn);
        const real gas = *Pres / Temp / 0.4e0;
        dl = rootHGK(Pres, Temp, dll, gas);
        dv = rootHGK(Pres, Temp, gas, dll);
        aSta.Dens[0] = dl;
        aSta.Dens[1] = dv;
    }
    liq_ = HGKPhase{phaseHGK(isat, itripl, Temp, Pres, dl, epseqn), dl};
    vap_ = HGKPhase{phaseHGK(isat, itripl, Temp, Pres, dv, epseqn), dv};
    vap_.w.Surtenw = real(0.0);  // surface tension is given for the saturated liquid only, as WaterLVS83 does
}

auto WaterHGKgems::rootHGK(real *p, real t, real guess, real other) -> real
{
    real d = 0.0;
    if (denHGK(&d, p, guess, t, &fct.dpdd))
        return d;
    if (denHGK(&d, p, other, t, &fct.dpdd))
        return d;
    Exception exception;
    exception.error << "Unable to calculate the density of water (HGK).";
    exception.reason << "The density did not converge at temperature " << t.val() << " K and pressure " << p->val() << " bar.";
    exception.line = __LINE__;
    RaiseError(exception);
    return d;
}

auto WaterHGKgems::phaseHGK(int isat, int itripl, real t, real *p, real d, int epseqn) -> WPROPS
{
    resid(t, &d);
    base(&d, t);
    ideal(t);
    thmHGK(&d, t);
    dimHGK(isat, itripl, t, p, &d, epseqn);
    return wr;
}

//--------------------------------------------------------------------//
// translate t from deg to users units
auto WaterHGKgems::TdegUS(int it, real t) -> real
{
    real TdegUS=t;
    switch(it)
    {
    case 0:
        TdegUS = t;
        break;
    case 1:
        TdegUS = t - 273.15e0;
        break;
    case 2:
        TdegUS = t * 1.8e0;
        break;
    case 3:
        TdegUS = t * 1.8e0 - 459.67e0;
        break;
    }
    return(TdegUS);
}
auto WaterHGKgems::TdPsdT(real t_) -> real
{
//    int i=-1;
    real v, w, y, z, q, b = 0.0e0, c = 0.0e0, TdPsdT;

    const auto& a = k_->psat_a;
    v = t_ / 647.25e0;
    w = 1.0 - v;
    for( int i=0; i<=7; i++  ) // while (i++ <= 7)
    {
        z  = i;
        y  = a[i] * pow(w,(z + 1.0e0)/2.0e0);
        c += y / w * (0.5e0 - 0.5e0 * z - 1.0e0 / v);
        b += y;
    }
    q = b / v;
    TdPsdT = 22.093e0 * exp(q) * c;
    return(TdPsdT);
}

//--------------------------------------------------------------------//
//   TsHGK - return Tsaturation(P).
auto WaterHGKgems::TsHGK(real Ps_) -> real
{
    int k=0;
    real pl, tg, pp, dp, TsHGK;

    TsHGK = 0.0e0;
    if (Ps_ > 22.05e0)
        return(TsHGK);
    pl = lg_to_ln + log(Ps_);
    tg = 372.83e0 + pl * (27.7589e0 + pl * (2.3819e0 +
                                            pl * (0.24834e0 + pl * 0.0193855e0)));
    /*  1   */
    while ( k++ <=  8)
    {
        if (tg < 273.15e0)
            tg = 273.15e0;
        if (tg > 647.00e0)
            tg = 647.00e0;
        pp = PsHGK(tg);
        dp = TdPsdT(tg);
        if( !(fabs(1.0e0 - pp/Ps_) < 1.0e-5 ) )
            tg *= (1.0e0 + (Ps_ - pp) / dp);
    }
    TsHGK = tg;
    return(TsHGK);
}

//--------------------------------------------------------------------//
/* tcorr - Calc T(P) saturation (t) and P (dL & dV) from  refinement
   of an initial approximation (TsHGK(p)) in accord with  Gl = Gv.  */
auto WaterHGKgems::tcorr(int itripl, real *t, real *p, real *dL, real *dV,
                    int epseqn) -> void
{
    real delg, dp;

    *t = TsHGK(*p);
    if (*t == 0.0e0)
        return;
    *dL = 0.0e0;
    *dV = 0.0e0;
    do
    {
        a1.rt = *t * ac->gascon;
        corr(itripl,*t,p,dL,dV,&delg,epseqn);
        dp = delg * ac->gascon * *t / (1.0e0 / *dV - 1.0e0 / *dL);
        *t *= (1.0e0 - dp / TdPsdT(*t));
    }
    while ( !(fabs(delg) > 1.0e-4) );
}

//--------------------------------------------------------------------//
// calc t/d properties H2O as ideal gas: equat see Woolley (1979).
auto WaterHGKgems::ideal(real t) -> void
{
    int i=1;
    real  tt, tl, emult;
    const auto& c = k_->ideal_c;

    tt  = t / 1.0e2;
    tl  = log(tt);
    id.gi  = - (c[0] / tt + c[1]) * tl;
    id.hi  = (c[1] + c[0] * (1.0e0 - tl) / tt);
    id.cpi = c[1] - c[0] / tt;
    while ( ++i <= 17 )
    {
        emult = pow(tt,((double)i - 5.));
        id.gi  -= c[i] * emult;
        id.hi  += c[i] * (i-5) * emult;
        id.cpi += c[i] * (i-5) * (i-4) * emult;
    }
    id.ai  = id.gi - 1.0e0;
    id.ui  = id.hi - 1.0e0;
    id.cvi = id.cpi - 1.0e0;
    id.si  = id.ui - id.ai;
}

//--------------------------------------------------------------------//
// tpset - calculate values U, S, H, A, G in 3-y point (in J/g from
// Table 2,  Helgeson & Kirkham,  1974a)
auto WaterHGKgems::tpset(WaterTripleProperties wtr) ->void
{
//    real Utr, Str, Htr, Atr, Gtr;

    real conv = 4.305816e0*cal_to_J; //ffh[4]; J_g_in_therm_cal_mol

//    Utr = -15766.55e0; //-15766.0e0; /
//    Str =  3.5158e0;//3.5144e0;
//    Htr = -15970.6e0; //-15971.0e0;
//    Atr = -12870.11e0;//-12870.0e0;
//    Gtr = -13070.67e0;//-13073.0e0;

    tt->Utri = (wtr.Utr/conv) * un.fh;
    tt->Stri = (wtr.Str/conv) * un.fh;
    tt->Htri = (wtr.Htr/conv) * un.fh;
    tt->Atri = (wtr.Atr/conv) * un.fh;
    tt->Gtri = (wtr.Gtr/conv) * un.fh;

}

auto WaterHGKgems::bb(real t) -> void
{
    int i=0;
    real v[10];
    v[0] = 1.0e0;
    while(++i <= 9)
    {
        v[i] = v[i-1] * ac->tz / t;
    }
    el->b1   = bcn->bp[0] + bcn->bp[1] * log(1.0 / v[1]);
    el->b2   = bcn->bq[0];
    el->b1t  = bcn->bp[1] * v[1] / ac->tz;
    el->b2t  = 0.0e0;
    el->b1tt = 0.0e0;
    el->b2tt = 0.0e0;
    i=1;
    while ( ++i <= 9 )
    {
        el->b1   += bcn->bp[i] * v[i-1];
        el->b2   += bcn->bq[i] * v[i-1];
        el->b1t  -= (i-1) * bcn->bp[i] * v[i-1] / t;
        el->b2t  -= (i-1) * bcn->bq[i] * v[i-1] / t;
        el->b1tt += bcn->bp[i] *(i-1)*(i-1) * v[i-1] / t / t;
        el->b2tt += bcn->bq[i] * (i-1)*(i-1) * v[i-1] / t / t;
    }
    el->b1tt -= el->b1t / t;
    el->b2tt -= el->b2t / t;
}

//--------------------------------------------------------------------//
//calc density (d in g/cm3) and dP/dD (dPdd) as f(p(MPa), t(degK))
// from an initial density guess (dguess).
auto WaterHGKgems::denHGK(real *d, real *p, real dguess, real t, real *dpdd) -> bool
{
    real pp, dp, dpdx, x;
    int  i= 0;
    *d  = dguess;
FIRST:
    do
    {
        i++;
        if (*d <= 0.0e0) *d = 1.0e-8;
        if (*d > 1.9e0)  *d = 1.9e0;
        resid(t,d);
        base(d,t);
        pp    = a1.rt * *d * ba.pb + qq.q0;
        *dpdd = a1.rt * (ac->zb + ac->yb * ac->dzb) + qq.q5;
        /*  if  dpdd < 0  assume d in 2-phase region and adjust accordingly */
        if (*dpdd > 0.0) goto OTHER;
        if (dguess >= 0.2967e0)   *d *= 1.02e0;
        if (dguess < 0.2967e0)    *d *= 0.98e0;
    }
    while (i <= 10);
    return false;
OTHER:
    {
        dpdx = *dpdd * 1.1e0;
        if (dpdx < 0.1e0)
            dpdx = 0.1e0;
        if (fabs(1.0e0 - pp / *p) < 1.0e-6)
            dpdx = *dpdd;  // near the root: a full Newton step
        // Converged to 1e-12 in P or in density, whatever the guess (before: 1e-8, 1e-7 or 1e-6 in P by the guess, so the
        // density and its derivatives depended on where the iteration started). A liquid at low P has a pressure that is a
        // small difference of large terms (noise about 1e-7 of P): the density test ends it.
        dp   = fabs(1.0e0 - pp / *p);
        x = (*p - pp) / dpdx;
        if (dp < 1.0e-12 || fabs(x) < 1.0e-12 * *d)
            return true;
        if (fabs(x)  >  0.1e0)
            x *= 0.1e0 / fabs(x);
        *d += x;
        if (*d <= 0.0e0)
            *d = 1.0e-8;
        if ( i <= 100 ) goto FIRST;
    }
    return false;
}


//--------------------------------------------------------------------//
/* Born92 - calculated Z, Q, Y, X .
*           epseqn = 1 statement Helgeson-Kirkham (1974)
*           epseqn = 2 statement Pitzer (1983)
*           epseqn = 3 statement Uematsu-Franck (1980)
*           epseqn = 4 statement Johnson-Norton (1991) !!!
*           epseqn = 5 statement Archer-Wang (1990)      */
auto WaterHGKgems::Born92(real TK, real Pbars, real Dgcm3, real betab,
                     real *alphaK, real *daldT, real *eps, real *Z,
                     real *Q, real *Y, real *X, int epseqn) -> void
{
    //int iRET = 0;
    real TMAX, PMAX, TOL, TdegC, dedP, dedT, d2edT2;

    TMAX = k_->born_TMAX;
    PMAX = k_->born_PMAX;
    TOL  = 1.0e-3;

    *eps = 0.0e0;
    *Z   = 0.0e0;
    *Y   = 0.0e0;
    *Q   = 0.0e0;
    *X   = 0.0e0;
    TdegC = TK - 273.15e0;
    // Beyond TMAX or PMAX (the range of Johnson and Norton 1991) SUPCRT92 and GEMS returned 0 for epsilon and the Born
    // functions, which silently changed the solutes by tens of kJ/mol. The values are calculated there, and the caller
    // (johnsonNorton91Gems) marks them as outside the range of the model (decision 0029).
    (void)TMAX; (void)PMAX; (void)TOL; (void)TdegC;
    if (epseqn == 4)
    {
        /* d2edT2=0.; */
        JN91(TK, Dgcm3, betab, alphaK, daldT, eps, &dedP, &dedT,
             &d2edT2);
        epsBrn(eps, dedP, dedT, d2edT2, Z, Q, Y, X);
    }
}

//--------------------------------------------------------------------//
// din. viscosity H2O in kg/m*s, if Pbars in validity region
// specified  by the  initial if  statement Watson et al. (1980);
//See  statement 3.1-2 and 4.1-5 in Tabl. 1,6,8 in Sengers and Kamgar-Parsi (1984).

//--------------------------------------------------------------------//
// thermal conductivity H2O in J/m*deg*s (=W/m*deg),
// specified by the initial IF statement equat Sengers et al. (1984);
//See  statement 3.2.14 in Tabl.2-5 and I.5-6 from the above reference.

//--------------------------------------------------------------------//
/* triple - translate values U, S, H, A, G zero triple point
*  properties (Haar et al., 1984; Levelt Sengers et al., 1983) referenced
*  to triple  point properties data in Helgeson and   Kirkham, 1974a. */
auto WaterHGKgems::triple(real T, WPROPS  *wr) -> void
{
    real TS;

    wr->Sw    = wr->Sw + tt->Stri;
    TS        = T * wr->Sw - tt->Ttr * tt->Stri;
    wr->Gw = wr->Hw - TS + tt->Gtri;
    wr->Aw = wr->Uw - TS + tt->Atri;
    wr->Hw = wr->Hw + tt->Htri;
    wr->Uw = wr->Uw + tt->Utri;
}

//--------------------------------------------------------------------//
auto WaterHGKgems::pcorr(int itripl, real t, real *p, real *dL,
                    real *dV, int epseqn) -> void
{
    // The same T in one calculation (calcv2, calcv3, HGKsat): the result of the first call
    if (pcorr_cache.valid && pcorr_cache.t == t.val() && pcorr_cache.itripl == itripl)
    {
        *p = pcorr_cache.p;
        *dL = pcorr_cache.dL;
        *dV = pcorr_cache.dV;
        trp.delg = pcorr_cache.delg;
        return;
    }
    // Newton on Gl = Gv from the correlation PsHGK, to 1e-12 in P (before: to |delg| < 1e-4, about 1e-5 in P, and the
    // densities were those of the pressure before the last step). dL and dV are those of the pressure returned.
    real dp;
    *p  = PsHGK(t);
    *dL= 0.0e0;
    *dV = 0.0e0;
    // After convergence one more step gives the exact derivatives of P (implicit function), then the densities at that P.
    for (int iter = 0; iter < 50; ++iter)
    {
        corr(itripl, t, p, dL, dV, &trp.delg, epseqn);
        dp   = trp.delg * ac->gascon * t / (1.0e0 / *dV - 1.0e0 / *dL);
        *p  += dp;
        if (fabs(dp) <= 1e-12 * fabs(*p))
            break;
    }
    corr(itripl, t, p, dL, dV, &trp.delg, epseqn);
    // Near the critical point of HGK (above about 646.6 K) the two density roots at a pressure merge and this iteration
    // fails: then both conditions of coexistence are solved for the two densities
    // A pair is accepted only if both densities converged and the two Gibbs energies are equal (|delg| < 1e-8, in RT)
    if (!std::isfinite(p->val()) || !(dL->val() > dV->val() * (1.0 + 1e-6)) || !(p->val() > 0.0) || !corr_converged_ || !(std::fabs(trp.delg.val()) < 1e-8))
        coexistenceHGK(t, p, dL, dV);
    // coexistenceHGK can return early at an intermediate temperature after coexistenceNewton set a1.rt and bb for it
    a1.rt = ac->gascon * t;
    bb(t);
    pcorr_cache = {true, itripl, t.val(), *p, *dL, *dV, trp.delg};
}

// P and G/RT of HGK at T (K) and density d (g/cm3), and dP/dd
auto WaterHGKgems::stateHGK(real t, real d, real& P, real& dPdd, real& g) -> void
{
    resid(t, &d);
    base(&d, t);
    P = a1.rt * d * ba.pb + qq.q0;
    dPdd = a1.rt * (ac->zb + ac->yb * ac->dzb) + qq.q5;
    ideal(t);
    thmHGK(&d, t);
    g = fct.gd;
}

// The saturation of HGK from P(dL) = P(dV) and G(dL) = G(dV): Newton in both densities, from the scaling law near the
// critical point of HGK (tcHGK, dcHGK), else by steps in T from 646.5 K, where the iteration of pcorr still works.
// After convergence one step more, for the derivatives with respect to T.
auto WaterHGKgems::coexistenceHGK(real t, real *p, real *dL, real *dV) -> void
{
    *p = *dL = *dV = std::numeric_limits<double>::quiet_NaN();
    if (!(t.val() < hc->tcHGK.val()))
        return;
    const real dc = hc->dcHGK / 1.0e3;  // g/cm3
    const real tau = (hc->tcHGK - t) / hc->tcHGK;
    real dl = dc * (1.0 + 2.0 * pow(tau, 0.325));
    real dv = dc * (1.0 - 2.0 * pow(tau, 0.325));
    if (!coexistenceNewton(t, dl, dv))
    {
        // continuation in T from a temperature where the guess of the scaling law converges
        double T0 = 646.5;
        real d0l = dc * (1.0 + 2.0 * pow((hc->tcHGK - T0) / hc->tcHGK, 0.325)), d0v = dc * (1.0 - 2.0 * pow((hc->tcHGK - T0) / hc->tcHGK, 0.325));
        if (t.val() <= T0 || !coexistenceNewton(real(T0), d0l, d0v))
            return;
        const int steps = 1 + static_cast<int>((t.val() - T0) / 0.01);
        for (int i = 1; i < steps; ++i)
        {
            const real Ti = T0 + (t.val() - T0) * i / steps;
            real l = d0l, v = d0v;
            if (!coexistenceNewton(Ti, l, v))
                return;
            d0l = l.val();
            d0v = v.val();
        }
        dl = d0l;
        dv = d0v;
        if (!coexistenceNewton(t, dl, dv))
            return;
    }
    real Pl, Pv, dPl, dPv, gl, gv;
    stateHGK(t, dl, Pl, dPl, gl);
    stateHGK(t, dv, Pv, dPv, gv);
    *p = Pl;
    *dL = dl;
    *dV = dv;
    trp.delg = gl - gv;
}

// Newton for P(dl) = P(dv), G(dl) = G(dv) at T; false if it does not converge to two different densities
auto WaterHGKgems::coexistenceNewton(real t, real& dl, real& dv) -> bool
{
    a1.rt = ac->gascon * t;
    bb(t);
    real Pl, Pv, dPl, dPv, gl, gv;
    bool converged = false;
    for (int iter = 0; iter < 100; ++iter)
    {
        stateHGK(t, dl, Pl, dPl, gl);
        stateHGK(t, dv, Pv, dPv, gv);
        const real F1 = Pl - Pv, F2 = gl - gv;
        const real J11 = dPl, J12 = -dPv, J21 = dPl / (dl * a1.rt), J22 = -dPv / (dv * a1.rt);
        const real det = J11 * J22 - J12 * J21;
        const real sl = (F1 * J22 - F2 * J12) / det;
        const real sv = (J11 * F2 - J21 * F1) / det;
        dl -= sl;
        dv -= sv;
        if (!std::isfinite(dl.val()) || !std::isfinite(dv.val()) || !(dv.val() > 0.0))
            return false;
        if (converged)
            break;
        converged = fabs(sl.val()) <= 1e-9 * dl.val() && fabs(sv.val()) <= 1e-9 * dv.val();
    }
    return converged && dl.val() > dv.val() * (1.0 + 1e-6);
}

auto WaterHGKgems::resid(real t, real *d) -> void
{
    int i=0, l, j, k, km;
    real  qr[11], qt[10], qzr[9], qzt[9];
    real  e, /* q10,*/ q20, v, dfdt,/* q2a, q5t,*/ d2f, dpt, tex, /* fct,*/
    /*dadt,*/ ddz, zz, del, dex,/* qp,*/ qm, att, tx, tau, ex1, ex2;
    real q10, fct, q5t, qp, dadt, q2a;
    qr[0]  = 0.0e0;
    qq.q5 = 0.0e0;
    qq.q6 = 0.0e0;
    qq.q0 = 0.0e0;
    res.ar = 0.0e0;
    dadt   = 0.0e0;
    res.cvr= 0.0e0;
    res.dpdtr = 0.0e0;
    e     = exp(-ac->aa * *d);
    q10   = (*d) * (*d) * e;
    q20   = 1.0e0 - e;
    qr[1] = q10;
    v     = ac->tz / t;
    qt[0] = t / ac->tz;

    while(++i < 10)
    {
        qr[i+1] = qr[i] * q20;
        qt[i]   = qt[i-1] * v;
    }
    for (i=0; i<=8; i++) /* EQUIVALENCE (qr(3), qzr(1)), (qt(2), qzt(1)) */
    {
        qzr[i] = qr[i+2];
        qzt[i]=qt[i+1];
    }                  /*  as resalt: qzr[0]=qr[2], ... qzr[8]=qr[10];
                                         qzt[0]=qt[1], ... qzt[8]=qt[9]; */
    i=-1;
    while(++i <  nc->nc)
    {
        k  = nc->ii[i] + 1;
        l  = nc->jj[i];
        zz = k;
        if (k == 1)
            qp = nc->g[i] * ac->aa * qr[1] * qzt[l-1];
        else
            qp = nc->g[i] * ac->aa * qzr[k-2] * qzt[l-1];

        qq.q0 += qp;
        qq.q5 += ac->aa * (2.0 / *d - ac->aa * (1.0 - e * (k-1) / q20)) * qp;
        {   // second derivative with respect to the density of qp: f = d^2 e (1-e)^(k-1)
            const real fp  = 2.0 / *d - ac->aa * (1.0 - e * (k-1) / q20);
            qq.q6 += qp * (fp * fp - 2.0 / (*d * *d) - (k-1) * ac->aa * ac->aa * e / (q20 * q20));
        }
        res.ar += nc->g[i] * qzr[k-1] * qzt[l-1] / q10 / zz / a1.rt;
        dfdt    = pow(q20,((double)k)) * (1-l) * qzt[l] / ac->tz / k;
        d2f     = l * dfdt;
        dpt     = dfdt*q10 * ac->aa * k / q20;
        dadt   += nc->g[i] * dfdt;
        res.dpdtr += nc->g[i] * dpt;
        res.cvr   += nc->g[i] * d2f / ac->gascon;
    }
    qp  = 0.0e0;
    q2a = 0.0e0;
    j=35;
    while (++j <= 39 )
    {
        if ( nc->g[j] )
        {
            k   = nc->ii[j];
            km  = nc->jj[j];
            ddz = ad->adz[j-36];
            del = *d / ddz - 1.0e0;
            if (fabs(del) < 1.0e-10)
                del = 1.0e-10;
            ex1   = -ad->aad[j-36] * pow(del,(double)k);

            if (ex1 < to->EXPTOL)
                dex = 0.0e0;
            else
                dex = exp(ex1)  * pow(del,(double)km);

            att   = ad->aat[j-36];
            tx    = ad->atz[j-36];
            tau   = t / tx - 1.0e0;
            ex2   = -att * tau * tau;

            if (ex2 <=  to->EXPTOL)
                tex = 0.0e0;
            else
                tex = exp(ex2);
            q10   = dex * tex;
            qm    = km / del - k * ad->aad[j-36] * pow(del,((double)k-1));
            fct   = qm * *d * *d * q10 / ddz;
            q5t   = fct * (2.0e0 / *d + qm / ddz) - (*d / ddz) * (*d / ddz) *
                    q10 * (km / del / del + k * (k-1) * ad->aad[j-36] *
                           pow(del,((double)k-2)));
            qq.q5    += q5t * nc->g[j];
            {   // derivatives of ln(dex tex) with respect to the density and the second derivative of fct
                const real L1 = qm / ddz;
                const real L2 = -(km / del / del + k * (k-1) * ad->aad[j-36] * pow(del,(double)k-2)) / ddz / ddz;
                const real L3 = (2.0 * km / del / del / del - k * (k-1) * (k-2) * ad->aad[j-36] *
                                 pow(del,(double)k-3)) / ddz / ddz / ddz;
                qq.q6 += nc->g[j] * q10 * (2.0 * L1 + 4.0 * *d * (L1 * L1 + L2) +
                                           *d * *d * (L1 * L1 * L1 + 3.0 * L1 * L2 + L3));
            }
            qp        += nc->g[j] * fct;
            dadt      -= 2.0e0 * nc->g[j] * att * tau * q10 / tx;
            res.dpdtr -= 2.0e0 * nc->g[j] * att * tau * fct / tx;
            q2a       += t * nc->g[j] * (4.0e0 * att * ex2 + 2.0e0 *
                                         att) * q10 / tx / tx;
            res.ar    += q10 * nc->g[j] / a1.rt;
        }
    }
    res.sr   = -dadt / ac->gascon;
    res.ur   = res.ar + res.sr;
    res.cvr += q2a / ac->gascon;
    qq.q0  += qp;
}

//--------------------------------------------------------------------//
// calc  Abase, Gbase, Sbase, Ubase, Hbase, Cvbase,
// see  Haar , Gallagher & Kell (1979), eq(1)
/*  /ellcon/  /basef/   /aconst/  */
auto WaterHGKgems::base(real *d, real t) -> void
{
    real x, z0, dz0, bb2tt;

    ac->yb = .25e0 * el->b1 * *d;
    x      = 1.0e0 - ac->yb;
    z0     = (1.0e0 + el->g1 * ac->yb + el->g2 * ac->yb * ac->yb) / (x*x*x);
    ac->zb = z0 + 4.0e0 * ac->yb * (el->b2 / el->b1 - el->gf);
    dz0    = (el->g1 + 2.0e0 * el->g2 * ac->yb) / (x * x * x) + 3.0e0 *
             (1.0e0 + el->g1 * ac->yb + el->g2 * ac->yb * ac->yb) / (x*x*x*x);
    ac->dzb= dz0 + 4.0e0 * (el->b2 / el->b1 - el->gf);
    ba.pb = ac->zb;
    ba.ab = -log(x) - (el->g2 - 1.0e0)/x + 28.16666667e0 / x / x +
            4.0e0* ac->yb *(el->b2 / el->b1 - el->gf) + 15.166666667e0 +
            log( *d * t * ac->gascon / .101325e0);
    ba.gb = ba.ab + ac->zb;
    ba.ub = -t * el->b1t * (ac->zb - 1.0e0 - *d * el->b2) / el->b1
            - *d * t * el->b2t;
    ba.sb = ba.ub - ba.ab;
    ba.hb = ac->zb + ba.ub;
    bb2tt  = t * t * el->b2tt;
    ba.cvb= 2.0e0 * ba.ub + (z0 - 1.0e0) * (((t * el->b1t / el->b1)
                                            * (t * el->b1t / el->b1)) - t * t * el->b1tt / el->b1)
            - *d * (bb2tt - el->gf * el->b1tt * t * t) -
            (t * el->b1t / el->b1) * (t * el->b1t / el->b1) * ac->yb * dz0;
    ba.dpdtb = ba.pb / t + *d * (ac->dzb * el->b1t / 4.0e0 + el->b2t -
                                 el->b2 / el->b1 * el->b1t);
}

//--------------------------------------------------------------------//
// calc t/d functions for HGK ( Helmholtz, Gibbs, et all)
auto WaterHGKgems::thmHGK(real *d, real t) -> void
{
    real  z;

    z = ac->zb + qq.q0 / a1.rt/ *d;
    fct.dpdd = a1.rt * (ac->zb + ac->yb * ac->dzb) + qq.q5;
    {   // P_rhorho at constant T: base part (Carnahan-Starling type z0(y)) + residual part
        const real xb  = 1.0 - ac->yb;
        const real N0  = 1.0 + el->g1 * ac->yb + el->g2 * ac->yb * ac->yb;
        const real d2z = 2.0 * el->g2 / (xb*xb*xb) + 6.0 * (el->g1 + 2.0 * el->g2 * ac->yb) / (xb*xb*xb*xb)
                         + 12.0 * N0 / (xb*xb*xb*xb*xb);
        const real Pdd = a1.rt * (el->b1 / 4.0) * (2.0 * ac->dzb + ac->yb * d2z) + qq.q6;
        fct.gam = *d * Pdd / fct.dpdd;
    }
    fct.ad   = ba.ab + res.ar + id.ai - ac->uref / t + ac->sref;
    fct.gd   = fct.ad + z;
    fct.ud   = ba.ub + res.ur + id.ui - ac->uref / t;
    fct.dpdt = a1.rt * *d * ba.dpdtb + res.dpdtr;
    fct.cvd  = ba.cvb + res.cvr + id.cvi;
    fct.cpd  = fct.cvd + t * fct.dpdt * fct.dpdt /
               (*d * *d * fct.dpdd * ac->gascon);
    fct.hd   = fct.ud + z;
    fct.sd   = ba.sb + res.sr + id.si - ac->sref;
    fct.dvdt = fct.dpdt / fct.dpdd / *d / *d;
    fct.cjtt = 1.0e0 / *d - t * fct.dvdt;
    fct.cjth = -fct.cjtt / fct.cpd / ac->gascon;
}

//--------------------------------------------------------------------//
auto WaterHGKgems::dimHGK(int isat,
                     int itripl, real t, real *p, real *d, int epseqn) -> void
{
    real pbars, dkgm3, betaPa, CpJKkg;

    wr.Aw   = fct.ad * a1.rt * un.fh;
    wr.Gw   = fct.gd * a1.rt * un.fh;
    wr.Sw   = fct.sd * ac->gascon * un.fh * un.ft;
    wr.Uw   = fct.ud * a1.rt * un.fh;
    wr.Hw   = fct.hd * a1.rt * un.fh;
    wr.Cvw  = fct.cvd * ac->gascon * un.fh * un.ft;
    wr.Cpw  = fct.cpd * ac->gascon * un.fh * un.ft;
    wr.Speedw = sqrt(fabs(fct.cpd * fct.dpdd * 1.0e3 /
                          fct.cvd));  // m/s
    wr.Betaw  = 1.0e0 / (*d * fct.dpdd * un.fp);
    wr.Alphaw = *d * fct.dvdt;
    wr.Gamw   = fct.gam;
    wr.dAldT  = dalHGK(d,t,wr.Alphaw);                 /*!!!*/
    pbars = *p * 1.0e1;
    dkgm3 = *d * 1.0e3;
    betaPa = wr.Betaw * un.fp / 1.0e6;
    CpJKkg = wr.Cpw / un.fh / un.ft * 1.0e3;
    // transport properties in SI (Pa s, W/(m K), N/m, m2/s)
    wr.Viscw  = waterViscosityGems(k_->transport, t, pbars, dkgm3, betaPa);
    wr.Tcondw = waterThermalConductivityGems(k_->transport, t, pbars, dkgm3, wr.Alphaw, betaPa);
    wr.Surtenw = (isat == 1) ? waterSurfaceTensionGems(k_->transport, t) : real(0.0);
    wr.Tdiffw = wr.Tcondw / (dkgm3 * CpJKkg);
    wr.Prndtlw = (wr.Tcondw != 0.0) ? real(wr.Viscw * CpJKkg / wr.Tcondw) : real(0.0);
    wr.Visckw = wr.Viscw / dkgm3;
    wr.Albew  = wr.Alphaw / betaPa;  // Pa/K

    if (itripl == 1)
        triple(t, &wr);
}

auto WaterHGKgems::PsHGK(real t) -> real
{
    real  pl, psHGK, v, w, b, q, z;
    int i=-1;
    const auto& a = k_->psat_a;
    if (t <= 314.0e0)
    {
        pl    = 6.3573118e0 - 8858.843e0 / t + 607.56335e0 * pow(t,-0.6e0);
        psHGK = 0.1e0 * exp(pl);
    }
    else
    {
        v = t / 647.25e0;
        w = fabs(1.0e0 - v);
        b = 0.0e0;
        while (++i <= 7)
        {
            z  = i + 1;
            b += a[i] * pow(w,((z + 1.0e0) / 2.0e0));
        }
        q = b / v;
        psHGK = 22.093e0 * exp(q);
    }
    return(psHGK);
}

//--------------------------------------------------------------------//
// (dliq & dvap),   (Gl-Gv)/RT  (delg)
auto  WaterHGKgems::corr(int itripl, real t, real *p, real *dL,
                    real *dV, real *delg, int epseqn) -> void
{
    real dguess, gl, gv;
    bb(t);
    dguess = *dL;
    if (*dL <= 0.0e0)
        dguess = 1.11e0 - 0.0004e0 * t;
    corr_converged_ = denHGK(dL,p,dguess,t, &fct.dpdd);
    ideal(t);
    thmHGK(dL,t);
    dimHGK(1,itripl,t,p,dL,epseqn);
    gl     = fct.gd;
    dguess = *dV;
    if (*dV <= 0.0e0)
        dguess = *p / a1.rt;
    corr_converged_ = denHGK(dV,p,dguess,t,&fct.dpdd) && corr_converged_;
    if (*dV < 5.0e-7)
        *dV = 5.0e-7;
    ideal(t);
    thmHGK(dV,t);
/*    FCTS
     *      ୮  (dimensionless); after
     *    pcorr's final call of corr (*delg < 10d-4)   */
    gv    = fct.gd;
    *delg = gl - gv;
}

//--------------------------------------------------------------------//
/* JN91 -Calculated (eps, dedP, dedT, d2edT2)(T,D) on statements
*         Johnson and Norton (1991);   fit regressed  from least
*         squares (HK74 and Pitz83 )
*           DATA type: T ............... K
*                    D ............... g/cm**3
*                    beta, dedP ...... bar**(-1)
*                    alpha, dedT ..... K**(-1)
*                    daldT, d2edT2 ... K**(-2)                         */
auto WaterHGKgems::JN91(real T, real D, real beta, real *alpha, real *daldT,
                   real *eps, real *dedP, real *dedT, real *d2edT2) -> void
{
    int j=-1, k=0;
    real c[5], dcdT[5], dc2dTT[5];
    real Tref = k_->jn_Tr, Tn;

    const auto& a = k_->jn_a;
    D = D / (k_->jn_Dr / 1000.0);  // D is in g/cm3

    Tn        = T / Tref;
    c[0]      = 1.0e0;
    dcdT[0]   = 0.0e0;
    dc2dTT[0] = 0.0e0;

    c[1]      = a[0] / Tn;
    dcdT[1]   = -a[0] * Tref / pow(T,2.);
    dc2dTT[1] = 2.0e0 * a[0] * Tref / pow(T,3.);

    c[2]      = a[1] / Tn + a[2] + a[3] * Tn;
    dcdT[2]   = -a[1] * Tref / pow(T,2.) + a[3] / Tref;
    dc2dTT[2] = 2.0e0 * a[1] * Tref / pow(T,3.);

    c[3]      = a[4] / Tn + a[5] * Tn + a[6] * pow(Tn,2.);
    dcdT[3]   = -a[4] * Tref / pow(T,2.) + a[5] / Tref + 2.0e0 *
                a[6] * T / pow(Tref,2.);
    dc2dTT[3] = 2.0e0 * a[4] * Tref / pow(T,3.) + 2.0e0 * a[6] / pow(Tref,2.);

    c[4]      = a[7] / pow(Tn,2.) + a[8] / Tn + a[9];
    dcdT[4]   =-2.0e0 * a[7] * pow(Tref,2.) / pow(T,3.) - a[8] * Tref /
               pow(T,2.);
    dc2dTT[4] = 6.0e0 * a[7] * pow(Tref,2.) / pow(T,4.) + 2.0e0 * a[8] *
                Tref / pow(T,3.);

    *eps = 0.0e0;

    while (++k <= 5)
    {
        *eps += c[k-1] * pow(D,(double)(k - 1));
    }
    *dedP = 0.0e0;
    while (++j <=  4)
    {
        *dedP += j * c[j] * pow(D,(double)j);
    }
    *dedP *= beta;
    *dedT = 0.0e0;
    j=-1;
    while (++j <= 4)
    {
        *dedT += pow(D,(double)j) * (dcdT[j] - j * *alpha * c[j]);
    }
    *d2edT2 = 0.0e0;
    j=-1;
    while (++j <= 4)
    {
        *d2edT2 += pow(D,(double)j) * (dc2dTT[j] - j * (*alpha *
                                       dcdT[j] + c[j] * *daldT) - j * *alpha *
                                       (dcdT[j] - j * *alpha * c[j]));
    }
}

//--------------------------------------------------------------------//
/* epsBrn - calc Brown functions Z, Q, Y, and X from their eps, dedP, dedT
   and d2edT2 counterparts.           */
auto WaterHGKgems::epsBrn(real *eps, real dedP, real dedT,real d2edT2,
                     real *Z, real *Q, real *Y, real *X) -> void
{
    *Z = -1.0e0 / *eps;
    *Q =  1.0e0 / pow(*eps,2.) * dedP;
    *Y =  1.0e0 / pow(*eps,2.) * dedT;
    *X =  1.0e0 / pow(*eps,2.) * d2edT2 - 2.0e0 * *eps * pow(*Y,2.);
}

auto WaterHGKgems::dalHGK(real *d, real t, real alpha) -> real
{
    int i=-1;
    real  k, l, km, lm, /* kp,*/ lp, x, xtzt, dydtp, dbdd,
    db2dd, db2ddt, db2dtp, db3ddt, db3dtt, drdd, dr2dd, dr2ddt,
    dr2dtp, /* dr3ddt, dr3dtt,*/ e1, e2, tzt, ai, bi, di, ti, tau, del,
    /* ex1, ex2, ex12, */ qm, xdell, xdelk, dalHGK/*, dex, tex*/,
    term1, term2, term3, term4;
    real /* db2dd, db2ddt, db2dtp, db3ddt, db3dtt,
                 drdd, dr2dd, dr2ddt,dr2dtp,*/ dr3ddt, dr3dtt,
    ex1, ex2, ex12, dex, tex, term5;
    /*   evaluate derivatives for the base function */
    ac->yb= .25e0 * el->b1 * *d;
    x     = 1.0e0 - ac->yb;
    dydtp = (*d / 4.0e0) * (el->b1t - el->b1 * alpha);
    dbdd  = ac->gascon * t * ((el->b1 / 4.0e0 / x) * (1.0e0 -
                              (el->g2 - 1.0e0) / x + (el->g1 + el->g2 + 1.0e0) /
                              x / x) + el->b2 - el->b1 * el->gf + 1.0e0 / *d);
    db2dd = ac->gascon * t * ((el->b1 * el->b1 / 16.0e0 / x / x) *
                              (1.0e0 - 2.0e0 * (el->g2 - 1.0e0) / x + 3.0e0 *
                               (el->g1 + el->g2 + 1.0e0) / x / x) - 1.0e0 / *d / *d);
    db2ddt = ac->gascon * t * ((el->b1t / 4.0e0 / x / x) *
                               (1.0e0 - (el->g2 - 1.0e0) * (1.0e0 + ac->yb) / x +
                                (el->g1 + el->g2 + 1.0e0) * (1.0e0 + 2.0e0 * ac->yb) /
                                x / x) + el->b2t - el->gf * el->b1t) + dbdd / t;
    db2dtp = dbdd / t + ac->gascon * t * ((el->b1 * dydtp / 4.0e0 /
                                           x / x / x) * (1.0e0 - el->g2 + 2.0e0 * (el->g1 +
                                                                                   el->g2 + 1.0e0) / x) + ((x * el->b1t + el->b1 *
                                                                                                            dydtp) / 4.0e0 / x / x) * (1.0e0 - (el->g2 - 1.0e0) /
                                                                                                                                       x + (el->g1 + el->g2 + 1.0e0) / x / x) + el->b2t -
                                          el->gf * el->b1t + alpha / *d );
    db3ddt = db2dd / t + ac->gascon * t * ((el->b1 * el->b1 *
                                            dydtp / 8.0e0 / x / x / x / x) * (1.0e0 - el->g2 +
                                                                              3.0e0 * (el->g1 + el->g2 + 1.0e0) / x) + (el->b1 *
                                                                                      (x * el->b1t + el->b1 * dydtp) / 8.0e0 / x / x / x) *
                                           (1.0e0 - 2.0e0 * (el->g2 - 1.0e0) / x + 3.0e0 *
                                            (el->g1 + el->g2 + 1.0e0) / x / x) - 2.0e0 * alpha /
                                           *d / *d );
    db3dtt = (db2ddt - dbdd / t) / t + ac->gascon * t * ((el->b1t *
             dydtp / 2.0e0 / x / x / x / x) * (1.0e0 - el->g2 +
                                               (el->g1 + el->g2 + 1.0e0) * (2.0e0 + ac->yb) / x) +
             ((x * el->b1tt + 2.0e0 * el->b1t * dydtp) / 4.0e0 /
              x / x / x) * (1.0e0 - (el->g2 - 1.0e0) * (1+ ac->yb) /
                            x + (el->g1 + el->g2 + 1.0e0) * (1.0e0 + 2.0e0 * ac->yb) /
                            x / x) + el->b2tt - el->gf * el->b1tt ) + (t * db2dtp -
                                    dbdd) / t / t;
    /***********************************************************/

    /*   evaluate derivatives for the residual function  */

    /*      drdd   = q/d/d                  */
    /*      dr2dd  = (q5 - 2.0e0/d*q0)/d/d   */
    /*      dr2ddt = dpdtr/d/d              */

    e1  = exp(-ac->aa * *d);
    e2  = 1.0e0 - e1;
    tzt = ac->tz / t;
    drdd   = 0.0e0;
    dr2dd  = 0.0e0;
    dr2ddt = 0.0e0;
    dr2dtp = 0.0e0;
    dr3ddt = 0.0e0;
    dr3dtt = 0.0e0;
    /*    evaluate terms 1-36      */
    while (++i < nc->nc)
    {
        k = nc->ii[i] + 1.0e0;
        l = nc->jj[i] - 1.0e0;
        km = k - 1.0e0;
        //   lm = l - 1.0e0;
        /*  kp = k + 1.0e0; */
        lp = l + 1.0e0;
        xtzt = pow(tzt,l);
        drdd   = drdd + nc->g[i] * xtzt * pow(e2,km) * e1;
        dr2dd  = dr2dd + nc->g[i] * e1 * xtzt * pow(e2,km) *
                 (km * e1 / e2 - 1.0e0);
        dr2ddt = dr2ddt - nc->g[i] * e1 * l * pow(e2,km) * pow(tzt,lp) /
                 ac->tz;
        dr2dtp = dr2dtp + nc->g[i] * e1 * pow(e2,km) * xtzt *
                 ( *d * alpha - l / t - km * e1 * *d * alpha / e2 );
        dr3ddt = dr3ddt + nc->g[i] * (km * *d * alpha * e1 * e1 * xtzt *
                                      pow(e2,(k-3.0e0)) + e1 * xtzt * pow(e2,km) * (km*e1/e2 -
                                              1.0e0) * (*d * alpha - l / t - km * *d * alpha * e1 / e2) );
        dr3dtt = dr3dtt + nc->g[i] * l * e1 * pow(e2,km) * pow(tzt,lp) /
                 ac->tz * (lp / t + *d * alpha * km * e1 / e2 - *d * alpha );
    }

    /*   evaluate terms 37-40   */

    i=35;
    while (++i <= 39)
    {
        k  = nc->ii[i];
        l  = nc->jj[i];
        km = k - 1.0e0;
        lm = l - 1.0e0;
        /*   kp = k + 1.0e0; */
        //  lp = l + 1.0e0;
        ai = ad->aad[i-36];
        bi = ad->aat[i-36];
        di = ad->adz[i-36];
        ti = ad->atz[i-36];
        tau = t / ti - 1.0e0;
        del = *d / di - 1.0e0;

        if (fabs(del) < 1.0e-10)
            del = 1.0e-10;
        ex1 = -ai * pow(del,k);
        if (ex1 < to->EXPTOL)
            dex = 0.0e0;
        else
            dex = exp(ex1);
        ex2  = -bi * tau * tau;
        if (ex2 <= to->EXPTOL)
            tex = 0.0e0;
        else
            tex = exp(ex2);
        ex12    = dex * tex;
        qm      = l / del - k * ai * pow(del,km);
        xdell   = pow(del,l);
        xdelk   = pow(del,k);
        drdd   += nc->g[i] * xdell * ex12 / di * qm;

        dr2dd  += nc->g[i] * xdell * ex12 / di / di * (qm * qm -
                  l / di / di - ai * k * km * pow(del,k-2.0e0));

        dr2ddt -= nc->g[i] * 2.0e0 * bi * tau * ex12 * xdell / ti / di * qm;

        dr2dtp += nc->g[i] / di * (*d * alpha * xdell * ex12 / di / del/del *
                                   (l + ai * k * km * xdelk) + qm * ( ex12 * (xdell* (k * ai *
                                                                      *d * alpha * pow(del,km) / di - 2.0e0 * bi * tau / ti) - l *
                                                                      *d * alpha * pow(del,lm)/di)));

        term1 = l * *d * alpha / di / del / del + ai * k * km * *d * alpha *
                pow(del,k-2.) / di;
        term2 = 2.* qm * term1 - 2.0e0 * l * *d * alpha / di / del / del /
                del + ai * k * km * (k-2.) * pow(del,k-3.) * *d * alpha / di;
        term3 = qm * qm - l / del / del - ai * k * km * pow(del,k-2.);
        term4 = ai * k * pow(del,k-1.) * *d * alpha / di - 2.0e0 * bi * tau / ti;
        term5 = ex12 * xdell * term4 - ex12 * l * pow(del,l-1.) * *d * alpha / di;

        dr3ddt += nc->g[i] / di / di * (xdell * ex12 * term2 + term3 * term5);

        dr3dtt -= 2.0e0 * nc->g[i] * bi / ti / di * (tau * xdell * ex12 * *d *
                  alpha / del / del / di * (l + ai * k * km * pow(del,k)) + qm *
                  (xdell * ex12 * (ai * k * *d * alpha * tau * pow(del,km) / di +
                                   (1.0e0 - 2.0e0 * bi * tau * tau) / ti - tau * l * *d * alpha /
                                   di / del )));
    }

    /*   compute (d(alpha)/dT)P   */

    dalHGK = ((db3dtt + dr3dtt) * (2.0e0 * (dbdd + drdd) + *d * (db2dd +
                                   dr2dd)) - (db2ddt + dr2ddt) * (2.0e0 * (db2dtp + dr2dtp) +
                                                                  *d * (db3ddt + dr3ddt) - *d * alpha * (db2dd + dr2dd))) /
             (2.0e0 * (dbdd + drdd) + *d * (db2dd + dr2dd)) / (2.0e0 *
                     (dbdd + drdd) + *d * (db2dd + dr2dd));

    return(dalHGK);
}

}

