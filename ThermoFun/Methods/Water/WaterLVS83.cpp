// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "WaterLVS83.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace ThermoFun {

auto makeWaterLVS83ModelParameters(const std::map<std::string, model::Quantity>& values) -> std::shared_ptr<const WaterLVS83ModelParameters>
{
    auto out = std::make_shared<WaterLVS83ModelParameters>();
    auto number = [&](const std::string& name) { return values.at(name).value; };
    for (int i = 0; i < 20; ++i)
    {
        out->co.a[i] = number("co_a_" + std::to_string(i + 1));
        out->co.q[i] = number("co_q_" + std::to_string(i + 1));
    }
    out->cr.Tc = number("cr_Tc");
    out->cr.rhoC = number("cr_rhoC");
    out->cr.Pc = number("cr_Pc");
    out->cr.Pcon = number("cr_Pcon");
    out->cr.Ucon = number("cr_Ucon");
    out->cr.Scon = number("cr_Scon");
    out->cr.dPcon = number("cr_dPcon");
    out->Tmin1 = number("Tmin1");
    out->Tmin2 = number("Tmin2");
    out->Tmax = number("Tmax");
    out->Dmin = number("Dmin");
    out->Dmax = number("Dmax");
    out->Pbase1 = number("Pbase1");
    out->Pbase2 = number("Pbase2");
    out->PTmins = number("PTmins");
    out->PTmaxs = number("PTmaxs");
    // the code reads the region from co.x as SUPCRT92 does
    out->co.x[0] = out->Tmin1;
    out->co.x[1] = out->Tmin2;
    out->co.x[2] = out->Tmax;
    out->co.x[3] = out->Dmin;
    out->co.x[4] = out->Dmax;
    out->co.x[5] = out->Pbase1;
    out->co.x[6] = out->Pbase2;
    out->co.x[9] = out->PTmins;
    out->co.x[10] = out->PTmaxs;
    return out;
}

WaterLVS83::WaterLVS83(std::shared_ptr<const WaterLVS83ModelParameters> constants) : keep_(std::move(constants)), k_(keep_.get()), co(&keep_->co), cr(&keep_->cr) {}

auto WaterLVS83::coversSaturation(double T) const -> bool
{
    return T >= k_->Tmin1 && T < k_->cr.Tc;
}

// crtreg of SUPCRT92 for T and P given
auto WaterLVS83::inRegion(double T, double P) -> bool
{
    if (T < k_->Tmin1 || T > k_->Tmax)
        return false;
    const double Pmin = k_->Pbase1 + k_->PTmins * (T - k_->Tmin1);
    const double Pmax = k_->Pbase2 + k_->PTmaxs * (T - k_->Tmin2);
    if (P >= Pmin && P <= Pmax)
        return true;
    // below Tmin2 the upper isochore lies under the saturation curve: the vapour side up to Psat
    return P >= Pmin && T <= k_->Tmin2 && P <= saturationPressure(real(T)).val();
}

auto WaterLVS83::saturationPressure(real T) -> real
{
    return Pfind(1, T, 0.0);
}

// The densities of the two phases at saturation (the isat = 1 branch of denLVS), after Pfind(1, T, .)
auto WaterLVS83::saturatedDensities(real T) -> void
{
    const real pw11 = co->q[8];
    const real xk0 = co->a[6];
    const real xk1 = co->a[11];
    const real Tw = -cr->Tc / T;
    const real dTw = 1.0 + Tw;
    ss(par.r1, par.th1, dv.s, dv.sd);
    const real rho1 = 1.0 + pw11 * dTw + co->a[0] * (dv.s[0] + dv.s[1]);
    const real rho2 = xk0 * pow(par.r1, co->a[5]) + xk1 * pow(par.r1, co->q[15]);
    sa.Dvap = cr->rhoC * (rho1 - rho2);
    sa.Dliq = cr->rhoC * (rho1 + rho2);
}

auto WaterLVS83::saturation(real T) -> std::array<WaterLVS83Phase, 2>
{
    if (!coversSaturation(T.val()))
        throw std::runtime_error("WaterLVS83: no saturation at " + std::to_string(T.val()) + " K (from " + std::to_string(k_->Tmin1) + " K to the critical temperature)");
    const real P = Pfind(1, T, 0.0);
    saturatedDensities(T);
    const auto vapour = phase(T, P, -1.0);
    const auto liquid = phase(T, P, 1.0);
    return {liquid, vapour};
}

auto WaterLVS83::pressure(real T, real D) -> real
{
    return Pfind(0, T, D);
}

// The stable phase off the saturation curve. Below Tc it lies outside the two-phase dome, on the liquid side if P > Psat:
// the root of P(T, D) = P is bracketed there (the code of the equation gives no metastable states inside the dome) and
// found to 1e-13 in D; one Newton step with T and P carried then gives the derivatives of D (implicit function).
auto WaterLVS83::fluid(real T, real P) -> WaterLVS83Phase
{
    const double Tv = T.val(), Pv = P.val();
    auto f = [&](double D) { return pressure(real(Tv), real(D)).val() - Pv; };
    double a = 0.0, b = 0.0;
    if (Tv < cr->Tc)
    {
        const double Ps = Pfind(1, real(Tv), 0.0).val();
        saturatedDensities(real(Tv));
        const double dl = sa.Dliq.val(), dvap = sa.Dvap.val();
        if (Pv > Ps)
        {
            a = dl * (1.0 + 1e-10);
            b = a;
            while (f(b) < 0.0 && b < 2000.0)
                b *= 1.05;
        }
        else
        {
            b = dvap * (1.0 - 1e-10);
            a = b;
            while (f(a) > 0.0 && a > 1e-6)
                a *= 0.95;
        }
    }
    else
    {
        a = b = cr->rhoC.val();
        while (f(a) > 0.0 && a > 1e-6)
            a *= 0.9;
        while (f(b) < 0.0 && b < 2000.0)
            b *= 1.1;
    }
    double fa = f(a), fb = f(b);
    double D = (fa >= 0.0) ? a : (fb <= 0.0) ? b : 0.5 * (a + b);
    // bisection with secant steps (Illinois)
    for (int i = 0; i < 200 && fa < 0.0 && fb > 0.0; ++i)
    {
        D = b - fb * (b - a) / (fb - fa);
        if (!(D > a && D < b))
            D = 0.5 * (a + b);
        const double fD = f(D);
        if (fD == 0.0 || (b - a) < 1e-13 * D)
            break;
        if ((fD < 0.0) == (fa < 0.0))
        {
            a = D;
            fa = fD;
            fb *= 0.5;
        }
        else
        {
            b = D;
            fb = fD;
            fa *= 0.5;
        }
    }
    // one Newton step with T and P: exact derivatives of D
    real Dr = D;
    const real Pd = pressure(T, Dr);
    Dr = Dr - (Pd - P) / d2.dPdD;
    pressure(T, Dr);  // the parametric variables at the final density
    sa.DH2O = Dr;
    return phase(T, P, par.th1);
}

// The properties of one phase: theta = -1 or 1 at saturation (after Pfind(1, ...)), else after Pfind(0, T, D) with sa.DH2O = D
auto WaterLVS83::phase(real T, real P, real theta) -> WaterLVS83Phase
{
    const int isat = (fabs(theta) == 1.0) ? 1 : 0;
    thmLVS(isat, T, par.r1, theta);
    WaterLVS83Phase out;
    out.T = T;
    out.P = P;
    out.D = sa.DH2O;
    out.A = th.AE;
    out.G = th.GE;
    out.S = th.Entrop;
    out.U = th.U;
    out.H = th.H;
    out.Cv = th.Cv;
    out.Cp = th.Cp;
    out.alpha = th.alphw;
    out.beta = th.betaw;
    out.gamma = th.gamw;
    out.speed = th.Speed;
    a2.th = theta;
    out.dAldT = dalLVS(sa.DH2O, T, P, th.alphw);
    return out;
}

//--------------------------------------------------------------------//
// The equation of Levelt Sengers et al. (1983) as in SUPCRT92: moved from the GEMS water code (WaterHGK-JNgems.cpp).
// conver converges to 1e-13 in the reduced density (before 1e-5 and 1e-6).

auto  WaterLVS83::Pfind(int isat, real T, real DD) -> real
{
    real err=0.;
    real  /* sd[2],*/ Pw0, Pw1, Pw2, Pw3,  am1, am2, am3,
    p00, p20, p40, p01, p21, p41, aa, xk0, xk1, pw11, alpha,
    alhi, besq, Tee, rho, rhodi=0., rho1=0., tt1, tt2, Pwmu,
                                         dPw0, dPw1, dPw, Pfind, Uw, dPdTcd, dPwdTw, Cvcoex=0.;
    /*  real amc */

    Pw1  = co->a[4];
    Pw2  = co->a[3];
    Pw3  = co->a[1];
    /* amc  = co->a[12]; */
    am1  = co->a[13];
    am2  = co->a[14];
    am3  = co->a[15];
    p00  = co->q[10];
    p20  = co->q[11];
    p40  = co->q[12];
    p01  = co->q[17];
    p21  = co->q[18];
    p41  = co->q[19];
    aa   = co->a[9];
    xk0  = co->a[6];
    xk1  = co->a[11];
    pw11 = co->q[8];
    alpha= co->q[9];
    alhi = co->q[14];
    besq = co->a[8];

    dv.xk[0] = xk0;
    dv.xk[1] = xk1;

    // the critical point itself (a temperature a few 1e-8 K below it is a subcritical state with two distinct densities)
    if (fabs(T - cr->Tc) < 1.0e-12)
        T = cr->Tc;
    Tee     = (T - cr->Tc) / cr->Tc;
    dv.Tw  = - cr->Tc / T;
    dv.dTw = 1.0 + dv.Tw;
    if ( !isat )
    {                           /* when isat == 0 */
        rho = DD / cr->rhoC;
        conver(&rho, Tee, rho1, &rhodi, err);
    }
    else
    {                           /* when isat == 1 */
        par.th1 = -1.0;
        a2.th  = par.th1;
        par.r1  = dv.dTw / (1.0 - besq);
        a2.r   = par.r1;

        ss(par.r1, par.th1, dv.s, dv.sd);
        rho = par.th1 * (xk0 * pow(par.r1, co->a[5]) +
                         xk1 * pow(par.r1, co->q[15])) + co->a[0] * (dv.s[0] + dv.s[1]);
        rho += 1.0 + pw11 * dv.dTw;
        dv.amu = 0.0;
        DD = rho * cr->rhoC;
    }
    tt1 = par.th1 * par.th1;
    tt2 = tt1*tt1;
    Pw0  = 1.0 + dv.dTw * (Pw1 + dv.dTw * (Pw2 + dv.dTw * Pw3));
    if ( !isat )
        Pwmu = dv.amu * rhodi;              /* when isat == 0 */
    else
        Pwmu = 0.0;                          /* when isat == 1 */
    dv.p0th = p00 + p20 * tt1 + p40 * tt2;
    dv.p1th = p01 + p21 * tt1 + p41 * tt2;
    dPw0 = xk0 * dv.p0th * pow(par.r1,(2.0 - alpha));
    dPw1 = xk1 * dv.p1th * pow(par.r1,(2.0 - alhi));
    dPw  = aa * (dPw0 + dPw1);
    dv.Pw   = Pw0 + Pwmu + dPw;
    Pfind = dv.Pw * cr->Pcon * T;
    if (fabs(par.th1) < 1.0)
        sa.iphase = 1;
    else
    {
        sa.iphase = 2;
        dv.dP0dT = Pw1 + dv.dTw * (2.0 * Pw2 + 3.0 * Pw3 * dv.dTw);
        dv.dM0dT = am1 + dv.dTw * (2.0 * am2 + 3.0 * am3 * dv.dTw);
        Uw = dv.dP0dT - rho * dv.dM0dT + pw11 * dv.amu + dv.s[0] + dv.s[1];
        dPdTcd = Uw + rho * dv.dM0dT;
        dPwdTw = dv.Pw - dv.Tw * dPdTcd;
        d2.dPdT   = cr->Pcon * dPwdTw;
    }

    aux(par.r1, par.th1, &dv.d2PdT2, &dv.d2PdMT, &dv.d2PdM2,
        aa, dv.xk, dv.sd, Cvcoex);
    if (sa.iphase == 1)
        d2.dPdD = cr->dPcon * DD * T / dv.d2PdM2;
    return(Pfind);
}

//--------------------------------------------------------------------//
/*   /coefs/ a[19], q[19], x[10]   */
auto  WaterLVS83::aux(real r1, real th1, real *d2PdT2, real *d2PdMT,
                   real *d2PdM2, real aa, real *xk, real *sd, real Cvcoex) -> void
/* xk[2], sd[2] */
{
    int i= -1;
    real  sS[2], w[2], y[2], z[2], coex[2], cc, beta, besq, delta, alpha,
    s00, s20, s01, s21, deli, ww, yy, zz, gamma, tt1, ter, g, alhi,
    beti, gami, a1, a2, a4, f1;

    cc   = co->a[0];
    beta = co->a[5];
    besq = co->a[8];
    delta= co->a[10];
    alpha= co->q[9];
    s00  = co->a[16];
    s20  = co->a[17];
    s01  = co->a[18];
    s21  = co->a[19];

    deli  = 0.0;
    sS[0]   = s00 + s20 * th1 * th1;
    sS[1]   = s01 + s21 * th1 * th1;
    sd[0]  = 2.0 * th1 * s20;
    sd[1]  = 2.0 * th1 * s21;
    ww     = 0.0;
    yy     = 0.0;
    zz     = 0.0;
    gamma  = beta * (delta - 1.0);
    tt1    = th1 * th1;
    ter    = 2.0 * beta * delta - 1.0;
    g      = (1.0 + (besq * ter - 3.0) * tt1 - besq * (ter - 2.0) * tt1 * tt1);
    Cvcoex = 0.0;

    while ( ++i < 2 )
    {
        alhi    = alpha - deli;
        beti    = beta + deli;
        gami    = gamma - deli;
        if ( r1 )
        {
            w[i]  = (1.0 - alhi) * (1.0 - 3.0 * tt1) * sS[i] -
                    beta * delta * (1.0-tt1) * th1 * sd[i];
            w[i]  = (w[i] * pow(r1, -alhi)) / g;
            w[i]  = w[i] * xk[i];
            ww    = ww + w[i];
            y[i]  = beti * (1.0 - 3.0 * tt1) * th1 -
                    beta * delta * (1.0 - tt1) * th1;
            y[i]  = (y[i] * pow(r1, beti - 1.0)) * xk[i] / g;
            yy    = yy + y[i];
            z[i]  = 1.0-besq*(1.0-(2.0*beti))*tt1;
            z[i]  = (z[i]*pow(r1,-gami)) * xk[i] / g;
            zz    = zz + z[i];
            a1 = (beta * (delta - 3.0) - 3.0 * deli - besq * alhi * gami) /
                 (2.0 * besq * besq * (2.0 - alhi) * (1.0 - alhi) * alhi);
            a2 = 1 + ((beta * (delta - 3.0) - 3.0 * deli - besq * alhi
                       * ter) / (2.0 * besq * (1.0 - alhi) * alhi));
            a2 = -a2;
            a4 = 1.0 + ((ter - 2.0) / (2.0 * alhi));
            f1 = a1 + a2 + a4;
            coex[i] = ((2.0 - alhi) * (1.0 - alhi) * pow(r1, -alhi) * f1 * xk[i]);
            Cvcoex  += coex[i];
        }
        deli    = 0.5;
    }
    *d2PdT2 = aa * ww;
    *d2PdMT = yy + aa * cc * ww;
    *d2PdM2 = zz / aa + 2.0 * cc * yy + cc * cc * aa * ww;
}

//--------------------------------------------------------------------//
/* dFdMLVS - third derivative d3P/dM3 = d(d2P/dM2)/dM of the Levelt Sengers et al. (1983) critical-region equation
 *           at constant T, with the closed forms of aux() (F = d2P/dM2 = zz/a + 2c yy + c^2 a ww, M is the field of
 *           the revised and extended linear model).
 *
 * The parametric variables are X = r (1 - b^2 th^2) (temperature-like) and H = r^(beta delta) th (1 - th^2), with the
 * Jacobian J = r^(beta delta) g(th). The thermodynamic field is M = a H and the temperature variable is mixed with
 * the field, X = dT + c M, so that d/dM (at constant T) = (1/a) d/dH + c d/dX. With
 *    d r/dH = 2 b^2 r th/J,   d th/dH = (1 - b^2 th^2)/J,
 *    d r/dX = (1 - 3 th^2)/g, d th/dX = -beta delta th (1 - th^2)/(r g)
 * and the partial derivatives of F with respect to r (pure powers of r) and th (quotients of polynomials of th)
 *    dF/dM = (1/a)(F_r r_H + F_th th_H) + c (F_r r_X + F_th th_X).                                           */
auto WaterLVS83::dFdMLVS(real r1, real th1) -> real
{
    const real cc    = co->a[0];
    const real beta  = co->a[5];
    const real besq  = co->a[8];
    const real delta = co->a[10];
    const real alpha = co->q[9];
    const real aa    = co->a[9];
    const real s0[2] = { co->a[16], co->a[18] };
    const real s2[2] = { co->a[17], co->a[19] };
    const real xk[2] = { co->a[6],  co->a[11] };
    const real bd    = beta * delta;
    const real gamma = beta * (delta - 1.0);
    const real ter   = 2.0 * bd - 1.0;
    const real tt1   = th1 * th1;

    const real g   = 1.0 + (besq * ter - 3.0) * tt1 - besq * (ter - 2.0) * tt1 * tt1;
    const real gth = 2.0 * (besq * ter - 3.0) * th1 - 4.0 * besq * (ter - 2.0) * th1 * tt1;

    real Fr = 0.0, Fth = 0.0;
    for (int i = 0; i < 2; ++i)
    {
        const real deli = (i == 0) ? 0.0 : 0.5;
        const real alhi = alpha - deli;
        const real beti = beta + deli;
        const real gami = gamma - deli;

        const real s  = s0[i] + s2[i] * tt1;
        const real sp = 2.0 * th1 * s2[i];

        // numerators of w_i, y_i, z_i of aux() and their derivatives with respect to theta
        const real Nw  = (1.0 - alhi) * (1.0 - 3.0 * tt1) * s - bd * (1.0 - tt1) * th1 * sp;
        const real Nwt = (1.0 - alhi) * (-6.0 * th1 * s + (1.0 - 3.0 * tt1) * sp)
                         - bd * ((1.0 - 3.0 * tt1) * sp + (1.0 - tt1) * th1 * 2.0 * s2[i]);
        const real Ny  = (beti - bd) * th1 + (bd - 3.0 * beti) * th1 * tt1;
        const real Nyt = (beti - bd) + 3.0 * (bd - 3.0 * beti) * tt1;
        const real Nz  = 1.0 - besq * (1.0 - 2.0 * beti) * tt1;
        const real Nzt = -2.0 * besq * (1.0 - 2.0 * beti) * th1;

        const real w = Nw * pow(r1, -alhi)       * xk[i] / g;
        const real y = Ny * pow(r1, beti - 1.0)  * xk[i] / g;
        const real z = Nz * pow(r1, -gami)       * xk[i] / g;
        const real wt = (Nwt - Nw * gth / g) * pow(r1, -alhi)      * xk[i] / g;
        const real yt = (Nyt - Ny * gth / g) * pow(r1, beti - 1.0) * xk[i] / g;
        const real zt = (Nzt - Nz * gth / g) * pow(r1, -gami)      * xk[i] / g;

        Fr  += (-gami * z) / (aa * r1) + 2.0 * cc * (beti - 1.0) * y / r1 - cc * cc * aa * alhi * w / r1;
        Fth += zt / aa + 2.0 * cc * yt + cc * cc * aa * wt;
    }

    const real rbd = pow(r1, bd);
    const real rH  = 2.0 * besq * r1 * th1 / (rbd * g);
    const real thH = (1.0 - besq * tt1) / (rbd * g);
    const real rX  = (1.0 - 3.0 * tt1) / g;
    const real thX = -bd * th1 * (1.0 - tt1) / (r1 * g);

    return (Fr * rH + Fth * thH) / aa + cc * (Fr * rX + Fth * thX);
}

//--------------------------------------------------------------------//
auto WaterLVS83::thmLVS(int isat, real T, real r1, real th1)  -> void
{
    /* real  sd[2];*/
    real pw1, pw11, pw2, pw3, amc, am1, am2, am3, aa, Hw, dPwdTw,
    dPdTal,/* xk0, xk1,*/ d2P0dT, d2M0dT, dPdT2, rho, Uw, Cvcoex=0.,
            CviTw2, Cvw, Cpw, Sw, Scond;
    /* real alhi, besq, alpha; */

    pw2  = co->a[3];
    pw3  = co->a[1];
    /*  besq = co->a[8]; */
    amc  = co->a[12];
    am1  = co->a[13];
    am2  = co->a[14];
    aa   = co->a[9];
    dv.xk[0]  = co->a[6];
    am3  = co->a[15];
    dv.xk[1]  = co->a[11];
    pw11 = co->q[8];
    /*  alpha= co->q[9]; */
    /* alhi = co->q[14]; */
    pw1  = co->a[4];

    d2P0dT = 2.0 * pw2 + 6.0 * pw3 * dv.dTw;
    d2M0dT = 2.0 * am2 + 6.0 * am3 * dv.dTw;
    dv.dP0dT  = pw1 + dv.dTw * (2.0 * pw2 + 3.0 * pw3 * dv.dTw);
    dv.dM0dT  = am1 + dv.dTw * (2.0 * am2 + 3.0 * am3 * dv.dTw);
    if (isat == 0)
    {
        rho = sa.DH2O / cr->rhoC;
        Uw  = dv.dP0dT - rho * dv.dM0dT + pw11 * dv.amu +
              dv.s[0] + dv.s[1];
    }
    else
    {
        rho  = th1 * ( dv.xk[0] * pow(r1,co->a[5]) + dv.xk[1] *
                       pow(r1,co->q[15])) + co->a[0] * (dv.s[0] + dv.s[1]);
        rho += 1.0 + pw11 * dv.dTw;
        Uw   = dv.dP0dT - rho * dv.dM0dT + pw11 * dv.amu +
               dv.s[0] + dv.s[1];
        sa.DH2O = rho * cr->rhoC;
        dPdT2    = dv.Pw - dv.Tw * (Uw + rho * dv.dM0dT);
        th.heat = 1.0e3 * T * (cr->Pcon * dPdT2) * (1.0 / sa.Dvap -
                  1.0 / sa.Dliq);

        ss(r1, th1, dv.s, dv.sd);
        aux(r1,th1, &dv.d2PdT2, &dv.d2PdMT, &dv.d2PdM2, aa,
            dv.xk, dv.sd, Cvcoex);
        if (r1 != 0.0)
            d2.dPdD = cr->dPcon * sa.DH2O * T / dv.d2PdM2;
    }
    if (r1 != 0.0)
    {
        a1.dPdTcd = dv.dP0dT + pw11 * (dv.amu -rho / dv.d2PdM2) +
                    dv.s[0] + dv.s[1] - dv.d2PdMT * rho / dv.d2PdM2;
        dPwdTw = dv.Pw - dv.Tw * a1.dPdTcd;
        dPdTal = cr->Pcon * dPwdTw;
        CviTw2 = d2P0dT - rho * d2M0dT + dv.d2PdT2 -
                 (pw11 + dv.d2PdMT) * (pw11 + dv.d2PdMT) / dv.d2PdM2;
        Cvw    = CviTw2 * dv.Tw * dv.Tw;
        Cpw    = Cvw + dv.d2PdM2 * dPwdTw * dPwdTw / (rho * rho);
        th.betaw  = 1.0 / (sa.DH2O * d2.dPdD);
        th.alphw  = th.betaw * dPdTal;
        // Gamma = rho P_rhorho / P_rho = 1 - rho (dF/dM)/F^2 with F = d2P/dM2 (= d rho/dM) and rho reduced
        th.gamw   = 1.0 - rho * dFdMLVS(r1, th1) / (dv.d2PdM2 * dv.d2PdM2);
        th.Speed   = 1.0e3 * sqrt(Cpw / Cvw * d2.dPdD);
    }
    else
    {
        Cvw   = 1.0;
        Cpw   = 1.0;
        th.betaw = 1.0;
        th.alphw = 1.0;
        th.Speed = 0.0;
        th.gamw  = 0.0;
    }
    Hw = dv.Pw - dv.Tw * Uw;
    Sw = Hw - rho * (dv.amu + amc + dv.dTw *
                     (am1 + dv.dTw * (am2 + dv.dTw * am3)));
    Scond  = cr->Scon / sa.DH2O;
    th.U      = Uw * cr->Ucon / sa.DH2O;
    th.H      = Hw * Scond * T;
    th.Entrop = Sw * Scond;
    th.AE     = th.U - T * th.Entrop;
    th.GE     = th.H - T * th.Entrop;
    th.Cv     = Cvw * Scond;
    th.Cp     = Cpw * Scond;
}

/* *************************************************************** */
/*  dalLVS - Calc and return  (d(alpha)/dt)p(D,T,alpha) for equation
*            of state Levelt Sengers et al. (1983).
*            Comments:  D (kg/m**3),  T (degK),  P (MPa),
*                         alpha (degK**-1).                        */
auto WaterLVS83::dalLVS(real D, real T, real P, real alpha) -> real
{
    int  i=-1;
    real s[2], dsdT[2], sp[2], dspdT[2], k[2], calpha[2], cbeta[2],
    cgamma[2], u[2], v[2], w[2], dudT[2], dvdT[2], dwdT[2];
    real dalLVS, a, c, delta, bsq, P11, Delta1, P2, P3, s00, s01, s20,
    s21, b1, b2, ar1, a01, ar2, a02, delT, q, amult, d0dT, drdT,
    dqdT, dP0dTT, ddelMT, dPdTT, dPdMMT, dPdMTT, dPPTT, pterm;
    /* real P1; */

    a     = co->a[9];
    c     = co->a[0];
    delta = co->a[10];
    bsq   = co->a[8];
    P11   = co->q[8];
    Delta1= co->q[13];
    /* P1    = co->a[4]; */
    P2    = co->a[3];
    P3    = co->a[1];
    s00   = co->a[16];
    s01   = co->a[18];
    s20   = co->a[17];
    s21   = co->a[19];

    if (a2.r == 0.0)
    {
        dalLVS = 1.0e6;
        return(dalLVS);
    }

    k[0]      = co->a[6];
    k[1]      = co->a[11];
    calpha[0] = co->q[9];
    calpha[1] = co->q[14];
    cbeta[0]  = co->a[5];
    cbeta[1]  = co->q[15];
    cgamma[0] = cbeta[0] * (delta - 1.0);
    cgamma[1] = cgamma[0] - Delta1;
    delT      = (T - cr->Tc) / T;
    s[0]      = s00 + s20 * pow(a2.th,2.);
    s[1]      = s01 + s21 * pow(a2.th,2.);
    sp[0]     = 2.0 * s20 * a2.th;
    sp[1]     = 2.0 * s21 * a2.th;
    /* ********************************************************************* */
    /* Compute drdT and d0dT from solution of the linear system              */
    /*                      ax = b                                           */
    /* d(dPdM)/dT = -D/Dc*alpha - P11*Tc/pow(T,2) = ar1*drdT + a01*d0dT = b1 */
    /* d(delT)/dT =           Tc/pow(T,2)         = ar2*drdT + a02*d0dT = b2 */
    /* ********************************************************************* */

    b1 = -D / cr->rhoC * alpha - P11 * cr->Tc / T / T;
    b2 =  cr->Tc / pow(T,2.);

    ar1 = 0.0;
    a01 = 0.0;

    while(++i < 2 )
    {
        ar1 = ar1 + k[i] * (cbeta[i] * a2.th * pow(a2.r,(cbeta[i]-1.0)) +
                            a * c * (1.0 - calpha[i]) * pow(a2.r,-calpha[i]) * s[i]);
        a01 = a01 + k[i] * (pow(a2.r,cbeta[i]) + a * c * sp[i] *
                            pow(a2.r,(1.0-calpha[i])));
    }
    ar2 = 1.0 - bsq * pow(a2.th,2.) - a * c * cbeta[0] * delta *
          (1.0 - pow(a2.th,2.)) * a2.th * pow(a2.r,(cbeta[0] * delta - 1.0));
    a02 = 3.0 * a * c * pow(a2.th,2.) * pow(a2.r,cbeta[0] * delta) -
          2.0 * bsq * a2.r * a2.th - a * c * pow(a2.r,cbeta[0] * delta);
    /********************************************************************/
    /* solve the linear system with simplistic GE w/ partial pivoting  */
    /********************************************************************/
    if (fabs(ar1) > fabs(ar2))
    {
        amult = -ar2 / ar1;
        d0dT  = (b2 + amult * b1) / (a02 + amult * a01);
        drdT  = (b1 - a01 * d0dT) / ar1;
    }
    else
    {
        amult = -ar1 / ar2;
        d0dT  = (b1 + amult * b2) / (a01 + amult * a02);
        drdT  = (b2 - a02 * d0dT) / ar2;
    }
    /********************************************************************/
    /*   Compute theta polynomials and their tempertaure derivatives  */
    dsdT[0]   = 2.0 * s20 * a2.th * d0dT;
    dsdT[1]   = 2.0 * s21 * a2.th * d0dT;
    dspdT[0]  = 2.0 * s20 * d0dT;
    dspdT[1]  = 2.0 * s21 * d0dT;
    q    = 1.0 + (bsq * (2.0 * cbeta[0] * delta - 1.0) - 3.0) *
           pow(a2.th,2.) - bsq * (2.0 * cbeta[0] * delta - 3.0)
           * pow(a2.th,4.);
    dqdT = 2.0 * (bsq * (2.0 * cbeta[0] * delta - 1.0) - 3.0) *
           a2.th * d0dT - 4.0 * bsq * (2.0 * cbeta[0] * delta -
                                       3.0) * pow(a2.th,3.) * d0dT;
    i=-1;
    while(++i < 2)
    {
        u[i]    = (1.0 - bsq * (1.0 - 2.0 * cbeta[i]) * pow(a2.th,2.)) / q;
        dudT[i] = (-2.0 * bsq * (1.0 - 2.0 * cbeta[i]) * a2.th * d0dT -
                   u[i] * dqdT) / q;
        v[i]    = ((cbeta[i] - cbeta[0] * delta) * a2.th +
                   (cbeta[0] * delta - 3.0 * cbeta[i]) * pow(a2.th,3.)) / q;
        dvdT[i] = ((cbeta[i] - cbeta[0] * delta) * d0dT +
                   3.0 * (cbeta[0] * delta - 3.0 * cbeta[i]) *
                   pow(a2.th,2.) * d0dT - v[i] * dqdT) / q;
        w[i]    = ((1.0 - calpha[i]) * (1.0 - 3.0 * pow(a2.th,2.)) *
                   s[i] - cbeta[0] * delta * (a2.th - pow(a2.th,3.)) *
                   sp[i]) / q;
        dwdT[i] = ((1.0 - calpha[i]) * ((1.0 - 3.0 * pow(a2.th,2.)) *
                                        dsdT[i] - 6.0 * a2.th * s[i] * d0dT) - cbeta[0] *
                   delta * ((a2.th - pow(a2.th,3.)) * dspdT[i] + sp[i] *
                            (d0dT - 3.0 * pow(a2.th,2.) * d0dT)) - w[i] * dqdT) / q;
    }
    /***************************************************************/
    /*   Compute dP0dTT, ddelMT, dPdTT, dPdMMT, dPdMTT, dPPTT      */

    dP0dTT = cr->Tc / pow(T,2.) * (2.0* P2 + 6.0 * P3 * delT);
    ddelMT = a * pow(a2.r,cbeta[0]*delta) * (cbeta[0]* delta *
             a2.th / a2.r * (1.0 - pow(a2.th,2.)) * drdT +
             (1.0 - 3.0 * pow(a2.th,2.)) * d0dT);
    dPdTT  = 0.0;
    dPdMMT = 0.0;
    dPdMTT = 0.0;
    i=-1;
    while (++i < 2)
    {
        dPdTT  = dPdTT + a * k[i] * (pow(a2.r,(1.0 - calpha[i])) *
                                     dsdT[i] + s[i] * (1.0 - calpha[i]) *
                                     pow(a2.r,-calpha[i]) * drdT);
        dPdMMT = dPdMMT + k[i] * ((pow(a2.r,(-cgamma[i])) * dudT[i] -
                                   u[i] * cgamma[i] * pow(a2.r,(-1.0 - cgamma[i])) * drdT) /
                                  a + 2.0 * c * (pow(a2.r,(cbeta[i] - 1.0)) * dvdT[i] +
                                                 v[i] * (cbeta[i] - 1.0) * pow(a2.r,(cbeta[i] - 2.0)) * drdT)
                                  + a * pow(c,2.) * (pow(a2.r,(-calpha[i])) * dwdT[i] -
                                                     calpha[i] * w[i] * pow(a2.r,(-1.0 - calpha[i])) * drdT));
        dPdMTT = dPdMTT + k[i] * (pow(a2.r,(cbeta[i] - 1.0)) * dvdT[i] +
                                  v[i] * (cbeta[i] - 1.0) * pow(a2.r,(cbeta[i] - 2.0)) *
                                  drdT + a * c * (pow(a2.r,(-calpha[i])) * dwdT[i] -
                                                  calpha[i] * pow(a2.r,(-1.0 - calpha[i])) * drdT * w[i]));
    }
    dPPTT = dP0dTT + dPdTT + P11 * ddelMT - D / cr->rhoC * dPdMTT /
            dv.d2PdM2 + (P11 + dv.d2PdMT) * (D / cr->rhoC * alpha /
                                             dv.d2PdM2 + D / cr->rhoC * dPdMMT / pow(dv.d2PdM2,2.));
    pterm = P / cr->Pc + a1.dPdTcd;

    /*          compute (d(alpha)/dT)P              */
    dalLVS  = cr->Tc * pow(cr->rhoC,2.) / pow(D,2.) / pow(T,2.) *
              (-2.0 / T * dv.d2PdM2 * pterm + 2.0 * alpha *
               dv.d2PdM2 * pterm + pterm * dPdMMT + dv.d2PdM2 * dPPTT);
    return(dalLVS);
}

//--------------------------------------------------------------------//
// translate parametrs into users units and load they into tprops.
auto  WaterLVS83::conver(real *rho,
                      real Tee, real rho1s, real *rhodi, real error1) -> void
{
    int isig=0;
    real error2;
    real /*sd[2]*/ beta, delta, xk1, cc, besq, p11, aa, xk0,  betai,
    Tstar, dtstin, rhoweg, rhodit, drho, rho1co, twofaz, hold,
    y1, den1, den2, den12, tt, rho1s2, slope;

    /* real alhi, alpha, deli, p1w, p2w, p4w, s00, s20; */
    beta  =co->a[5];
    delta =co->a[10];
    xk1   =co->a[11];
    cc    =co->a[0];
    /* alhi  =co->q[14];
       alpha =co->q[9];      */
    besq  =co->a[8];
    p11   =co->q[8];
    /* deli  =co->q[13];     */
    /* p1w =co->q[17];
       p2w   =co->q[18];
       p4w   =co->q[19];     */
    aa    =co->a[9];
    xk0   =co->a[6];
    /* s00 =co->a[16];       */
    /* s20 =co->a[17];       */
    betai =co->q[15];

    Tstar  = Tee + 1.0;
    dtstin = 1.0 - (1.0 / Tstar);
    par.r1     = dtstin;
    if (dtstin <= 0.0)
    {
        par.r1  = dtstin / (1.0 - besq);
        par.th1 = 1.0;
    }
    else
        par.th1 = 0.0;

    ss(par.r1, par.th1, dv.s, dv.sd);
    *rhodi   = 1.0 + p11 * dtstin;
    rhodit = *rhodi  + cc * dv.s[0] + cc * dv.s[1];
    drho   = *rho - rhodit;
    dv.amu    = 0.0;
    if (dtstin <= 0.0)
    {
        rho1co = xk0 * pow(par.r1,beta) + xk1 * pow(par.r1,betai);
        twofaz = rho1co;
        if (fabs(drho) <= twofaz)
        {
            //rho1s   = (rho1co * drho / fabs(drho)) + cc * dv.s[0];
            par.th1    = drho * fabs(drho);
            // error1 = 1.0;
            a2.r = par.r1;
            a2.th = par.th1;
            return;
        }
    }
    if ( !drho )
    {
        par.th1   = 0.0;
        par.r1    = dtstin;
        //rho1s  = cc * dv.s[0];
    }

    /*   rule for first pass   */
    y1   = dtstin;
    den1 = *rho - rhodit;

    rtheta(&par.r1, &par.th1, den1, y1);
    tt       = par.th1 * par.th1;
    dv.amu  = aa * pow(par.r1,(beta * delta)) * par.th1 * (1.0 - tt);
    y1       = dtstin + cc * dv.amu;

    ss(par.r1, par.th1, dv.s, dv.sd);
    rhoweg = xk1 * pow(par.r1,betai) * par.th1 + cc * dv.s[1];
    rho1s   = den1 + cc * dv.s[0] + rhoweg;
    error1 = *rho - *rhodi  - rho1s ;
    a2.r  = par.r1;
    a2.th = par.th1;

    if (fabs(error1) < 1.0e-13)
        return;

    /*   rule for second pass  */

    den12 = *rho - *rhodi  - cc * dv.s[1] + rhoweg;
    if (den12 == den1)
        den12 = den1 - 1.0e-6;

    rtheta(&par.r1,&par.th1,den12,y1);
    tt      = par.th1 * par.th1;
    dv.amu = aa * pow(par.r1,(beta * delta)) * par.th1 * (1.0 - tt);
    y1      = dtstin + cc * dv.amu;

    ss(par.r1, par.th1, dv.s, dv.sd);
    rhoweg = xk1 * pow(par.r1,betai) * par.th1 + cc * dv.s[1];
    rho1s2 = den12 + cc * dv.s[0] + rhoweg;
    error2 = *rho - *rhodi  - rho1s2;
    if (fabs(error2) <= 1.0e-13)
    {
        a2.r  = par.r1;
        a2.th = par.th1;
        //error1 = error2;
        // rho1s   = rho1s2;
        return;
    }

    /*    rule for nth pass   */

    den2   = den12;
    while( ++isig <= 50 )
    {
        slope  = (error2 - error1) / (den2 - den1);
        hold   = den2;
        den2   = den1 - (error1 / slope);

        rtheta(&par.r1,&par.th1,den2,y1);
        tt      = par.th1 * par.th1;
        dv.amu = aa * pow(par.r1,(beta * delta)) * par.th1 * (1.0 - tt);
        y1      = dtstin + cc * dv.amu;

        ss(par.r1, par.th1, dv.s, dv.sd);
        rhoweg = xk1 * pow(par.r1,betai) * par.th1 + cc * dv.s[1];
        rho1s   = den2 + cc * dv.s[0] + rhoweg;
        error1 = error2;
        error2 = *rho - *rhodi  - rho1s ;
        a2.r  = par.r1;
        a2.th = par.th1;
        if (fabs(error2) < 1.0e-13)
            return;
        den1 = hold;
    }
}

//--------------------------------------------------------------------//
//  ss - calc terms of the summation zat defined dPotl/dT and 1-th
// proizvodnuyu theta (s)  square polynomial.
auto WaterLVS83::ss(real r, real th, real *s, real *sd) -> void/*  s[2], sd[2] */
{
    real  sx[2];
    real alpha, beta, alhi, beti, s00, s20, s01, s21, tt;
    /* real besq, delta, deli, gami, p00, p01;*/

    alpha= co->q[9];
    beta = co->a[5];
    /* besq = co->a[8];  */
    /* delta= co->a[10];
    deli = co->q[13];    */
    alhi = co->q[14];
    beti = co->q[15];
    /* gami = co->q[16];
    p00  = co->q[10];    */
    /* p01  = co->q[17]; */
    s00  = co->a[16];
    s20  = co->a[17];
    s01  = co->a[18];
    s21  = co->a[19];

    tt    = th * th;
    sx[0] = s00 + s20 * tt;
    sd[0] = 2.0 * s20 * th;
    sx[1] = s01 + s21 * tt;
    sd[1] = 2.0 * s21 * th;
    s[0]  = sx[0] * co->a[9] * co->a[6] * pow(r,(1.0 - alpha));
    s[1]  = sx[1] * co->a[9] * co->a[11] * pow(r,(1.0 - alhi));

    a1.dPdM  = pow(r,beta) * co->a[6] * th + co->a[0] * pow(r,(1.0-alpha)) *
               co->a[9] * co->a[6] * sx[0] + pow(r,beti) * co->a[11] * th +
               co->a[0] * pow(r,(1.0-alhi)) * co->a[9] * co->a[11] * sx[1];
}

//--------------------------------------------------------------------//
// rtheta - Fits data for 1.0 < theta < 1.000001. Calc:
//                        rho = em*theta*(r**beta)
//                        Tee = r*(1.0-besq*theta*theta)
//  Routine given by Moldover (1978): Jour. Res. NBS, v. 84, n. 4, p. 329 - 334.
auto WaterLVS83::rtheta(real *r, real *theta, real rho, real Tee) -> void
{
    int n=0;
    real  beta, em, besq, absrho, bee, z, z2, z3, c, dz, tr, tmr;

    beta = co->a[5];
    em   = co->a[6];
    besq = co->a[8];
    if (em <= 0.0 || besq <= 1.0)
        goto MET600;

    absrho = fabs(rho);
    if (absrho < 1.0e-12)
        goto MET600;

    bee = sqrt(besq);
    if (fabs(Tee) < 1.0e-12)
        goto  MET495;

    if (Tee < 0.0)
        z = 1.0-(1.0-bee) * Tee / (1.0 - besq) * pow(em / absrho,1.0 / beta);
    else
        z = pow(1.0 + Tee * pow(em / bee / absrho,1.0 / beta), - beta);

    if (z > 1.00234 * bee)
        goto MET496;

    tr = fabs(Tee);
    c  = -rho * bee / em / pow(tr,beta);
    z  = z * rho / fabs(rho);
    while ( ++n <= 16)
    {
        z2 = z * z;
        z3 = 1.0 - z2;
        tmr= fabs(z3);
        dz = z3 * (z + c * pow(tmr,beta)) / (z3 + 2.0 * beta * z2);
        z  = z - dz;
        if (fabs(dz/z) < 1.0e-12)
            goto MET498;
    }

MET601:
    if (fabs(*theta) > 1.0001)
        *theta /= fabs(*theta);
    return;

MET498:
    *theta = z / bee;
    *r     = Tee /(1.0 - z * z);
    *r     = fabs(*r);
    return;

MET495:
    *theta = rho / fabs(rho) / bee;
    *r     = pow(rho / (em * (*theta)),1.0 / beta);
    return;

MET496:
    *theta = rho / fabs(rho);
    *r     = Tee / (1.0 - besq);
    *r     = fabs(*r);
    return;

MET600:
    if (fabs(Tee) < 1.0e-12)
        goto MET601;

    if (Tee < 0.0)
        goto MET496;
    *theta = 1.0e-12;
    *r     = Tee;
}

} // namespace ThermoFun
