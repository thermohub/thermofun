// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_CGFREFERENCE_HPP
#define THERMOFUN_CGFREFERENCE_HPP

// Churakov and Gottschalk (2003) EOS of a pure fluid with exact derivatives. The original implementation (TCGFcalc in
// s_solmod2_.cpp) calculates the compressibility Z = 1 + rho dF/drho, the internal energy U = dF/dbeta and the entropy
// of the residual properties with finite differences (relative step 1e-5). These are derivatives of closed form
// functions: here the free energy F(T, rho) is a template on the number type, and the derivatives are calculated with
// nested autodiff dual numbers. The pass derivative (d/dT or d/dP of the properties) is the base level of the numbers.

#include <autodiff/forward/dual.hpp>

#include "Common/Real.hpp"

namespace ThermoFun {
namespace cgfref {

using Dual0 = autodiff::dual;                         // the derivative of the pass (T or P)
template<class N> using Up = autodiff::Dual<N, N>;   // one more (inner) derivative variable

// constants of the model (as in TCGFcalc::set_internal)
constexpr double PI_1 = 3.141592653589793120, TWOPI = 6.283185307179586230, PISIX = 0.523598775598298927,
                 TWOPOW1SIX = 1.12246204830937302, NA = 0.6023,
                 P1 = 1.186892378996, PP2 = -0.4721963005527, P3 = 3.259515855283, P4 = 3.055229342609,
                 P5 = 1.095409321023, P6 = 1.282306659774E-2, P7 = 9.55712461425E-2, P8 = 13.67807693107,
                 P9 = 35.75464856619, P10 = 16.04724381643,
                 AA1 = -0.120078459237, AA2 = -.808712488307, AA3 = .321543801337,
                 A4 = 1.16965477132, A5 = -.410564939543, A6 = -.516834310691,
                 BB1 = -2.18839961483, BB2 = 1.59897428009, BB3 = -.392578806128,
                 B4 = -.189396607904, B5 = -.576898496254, B6 = -0.0185167641359,
                 A00 = .9985937977069455, A01 = .5079834224407451, A10 = 1.021887697885469,
                 A11 = -5.136619463333883, A12 = -5.196188074016755, A21 = -6.049240839050804,
                 A22 = 18.67848155616692, A23 = 20.10652684217768, A31 = 9.896491419756988,
                 A32 = 14.6738380473899, A33 = -77.44825116542995, A34 = -4.82871082941229;

// ------------------------------------------------------------------------------------------------------------------
// number type helpers

inline auto value(double x) -> double { return x; }
template<class N> auto value(const N& x) -> double { return autodiff::val(x); }

/// A real (the value and the derivative of the pass) as the base number type
inline auto fromReal(const real& r) -> Dual0 { Dual0 d(r[0]); d.grad = r[1]; return d; }

/// A base number as a real
inline auto toReal(const Dual0& d) -> real { real r(d.val); r[1] = d.grad; return r; }

/// The number of a base number type in a nested number type
template<class N> struct Lift { static auto from(const Dual0& d) -> N { return d; } };
template<class M> struct Lift<Up<M>> { static auto from(const Dual0& d) -> Up<M> { Up<M> r; r.val = Lift<M>::from(d); r.grad = M(0.0); return r; } };

/// A number of the next level with the value x (and zero derivative with respect to the inner variable). The constructor of
/// the nested dual number from a dual number would take x as a function of the new variable.
template<class N> auto up(const N& x) -> Up<N> { Up<N> r; r.val = x; r.grad = N(0.0); return r; }

/// The derivative of f with respect to an inner variable: f takes a number of the next level, and x is the point
template<class N, class F> auto derivativeOf(const F& f, const N& x) -> N
{
    Up<N> ux; ux.val = x; ux.grad = N(1.0);
    Up<N> r = f(ux);
    return r.grad;
}

// ------------------------------------------------------------------------------------------------------------------
// free energy of the Weeks-Chandler-Andersen reference fluid

template<class N> auto RPA(const N& beta, const N& nuw) -> N
{
    N fi1 = (1.20110+(0.064890+(-76.860+(562.686+(-2280.090+(6266.840+(-11753.40+(14053.8
            +(-9491.490 +2731.030*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw;
    N fi2 = (0.588890+(-7.455360+(40.57590+(-104.8970+(60.25470+(390.6310+(-1193.080
            +(1576.350+(-1045.910+283.7580*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw*nuw;
    return (-12.*fi1 + 192.*fi2*beta)*beta*beta/PI_1;
}

/// the effective hard sphere diameter: Newton's iterations, with extra steps for the derivatives of the number
template<class N> auto dHS(const N& beta, const N& ro) -> N
{
    const double DV112 = 1./12., DV712 = 7./12.;
    const N T12 = sqrt(beta);
    const N T112 = exp(DV112*log(beta));
    const N T712 = exp(DV712*log(beta));
    N B13 = (1+beta);
    B13 = B13*B13*B13;

    const N dB = (P1*T112+PP2*T712+(P3+(P4+P5*beta)*beta)*beta)/B13;
    const N delta = (P6+P7*T12)/(1.+(P8+(P9+P10*T12)*T12)*T12);

    const N dbdl = dB*delta;
    const N ri6ro = PISIX*ro;
    const N ri6ro2 = ri6ro*ri6ro;

    const N a0 = dB+dbdl;
    const double a1 = -1.;
    const N a3 = (-1.5*dB -3.75*dbdl)*ri6ro;
    const N a4 = (1.5*ri6ro);
    const N a6 = (2.*dB + dbdl)*0.25*ri6ro2;
    const N a7 = -0.5*ri6ro2;
    const N a9 = -2.89325*ri6ro2*ri6ro*dbdl;
    const N a12 = -0.755*ri6ro2*ri6ro2*dbdl;

    const double p0 = -1.;
    const N p2 = a3*3.;
    const N p3 = a4*4.;
    const N p5 = a6*6.;
    const N p6 = a7*7.;
    const N p8 = a9*9.;
    const N p11 = a12*12.;

    auto step = [&](const N& d) -> N
    {
        const N d2 = d*d;
        const N d3 = d*d*d;
        const N F0 = a0+(a1+(a3+(a4+(a6+(a7+(a9+a12*d3)*d2)*d)*d2)*d)*d2)*d;
        const N F1 = p0+(p2+(p3+(p5+(p6+(p8+p11*d3)*d2)*d)*d2)*d)*d2;
        return d-F0/F1;
    };

    N d = dB;
    for (int i = 0; i < 21; ++i)
    {
        const N dnew = step(d);
        if (std::fabs(value(dnew) - value(d)) < 1.E-7)
        {
            N result = dnew;
            for (int k = 0; k < 3; ++k) // the derivatives of the number converge one order per step
                result = step(result);
            return result;
        }
        d = dnew;
    }
    return dB;
}

template<class N> auto fI1_6(const N& nuw) -> N
{
    return (1.+(A4+(A5+A6*nuw)*nuw)*nuw)/((1.+(AA1+(AA2+AA3*nuw)*nuw)*nuw)*3.);
}

template<class N> auto fI1_12(const N& nuw) -> N
{
    return (1.+(B4+(B5+B6*nuw)*nuw)*nuw)/((1.+(BB1+(BB2+BB3*nuw)*nuw)*nuw)*9.);
}

/// The reduced Helmholtz free energy of the WCA reference fluid, T and ro in reduced units
template<class N> auto FWCA(const N& T, const N& ro) -> N
{
    const double rm = TWOPOW1SIX;
    const N beta = 1./T;
    const N d = dHS(beta, ro);
    N tmp2 = PISIX*d*d*d;
    const N nu = tmp2*ro;
    N tmp1 = (1. - nu/16.);
    const N nuw = nu*tmp1;
    const N dW = d*exp(1./3.*log(tmp1));

    const N nu1w1 = (1.-nuw);
    const N nu1w2 = nu1w1*nu1w1;
    const N nu1w3 = nu1w2*nu1w1;
    const N nu1w4 = nu1w2*nu1w2;
    const N nu1w5 = nu1w2*nu1w3;

    tmp1 = (1-nu);
    tmp1 = tmp1*tmp1;
    const N F0 = ((4.-3.*nu)*nu)/tmp1;

    const N a0 = (A00 + A01*nuw)/nu1w2;
    const N a1 = (A10+(A11+A12*nuw)*nuw)/nu1w3;
    const N a2 = ((A21+(A22+A23*nuw)*nuw)*nuw)/nu1w4;
    const N a3 = ((A31+(A32+(A33+A34*nuw)*nuw)*nuw)*nuw)/nu1w5;

    const N I1_6 = fI1_6(nuw);
    const N I1_12 = fI1_12(nuw);

    const N rmdw1 = rm/dW;
    const N rmdw2 = rmdw1*rmdw1;
    const N rmdw3 = rmdw1*rmdw2;
    const N rmdw4 = rmdw2*rmdw2;
    const N rmdw5 = rmdw3*rmdw2;

    N dW6 = dW*dW*dW;
    dW6 = 1./(dW6*dW6);
    const N dW12 = dW6*dW6;

    const N t1 = (a0/4.+ a1/12. + a2/24. + a3/24.)*dW6;
    const N t2 = (a0/10.+ a1/90. + a2/720. + a3/5040.)*(-dW12);
    const N t3 = (a0 - a1/3. + a2/12 - a3/60)/8.;
    const N t4 = (a0 - a1 + a2/2. - a3/6.)*rmdw2*(-9.)/40.;
    const N t5 = (a1 - a2 + a3/2)*rmdw3*(-2.)/9.;
    const N t6 = (a2 - a3)*rmdw4*(-9.)/64.;
    const N t7 = a3*(-3.)/35.*rmdw5;

    const N I2 = t1+t2+t3+t4+t5+t6+t7;

    const N F1 = 48.*nuw*(I1_12*dW12-I1_6*dW6 + I2)*beta;
    const N FA = RPA(beta, nuw);

    return F0+F1+FA;
}

/// The compressibility factor of the WCA fluid minus 1 plus 1: Z = 1 + ro dF/dro (exact, was a finite difference)
template<class N> auto ZWCA(const N& T, const N& ro) -> N
{
    auto f = [&](const Up<N>& r) -> Up<N> { return FWCA<Up<N>>(up<N>(T), r); };
    return 1. + ro*derivativeOf<N>(f, ro);
}

/// The configurational internal energy of the WCA fluid U = dF/dbeta (exact, was a finite difference)
template<class N> auto UWCA(const N& T, const N& ro) -> N
{
    auto f = [&](const Up<N>& t) -> Up<N> { return FWCA<Up<N>>(t, up<N>(ro)); };
    return -T*T*derivativeOf<N>(f, T); // dF/dbeta = -T^2 dF/dT
}

template<class N> auto J6LJ(const N& T, const N& ro) -> N
{
    const N beta = 1./T;
    const N Z = ZWCA<N>(T, ro);
    const N kappa = -16.*PI_1*ro*beta;
    const N U = UWCA<N>(T, ro);
    return (4.*beta*U-Z+1.)/kappa;
}

template<class N> auto K23_13(const N& T, const N& ro) -> N
{
    static const double dtmp[] = { -1.050534, 1.747476,  1.749366,  -1.999227, -0.661046, -3.028720};
    const N logT = log(T);
    const N a = ro*ro*logT, b = ro*ro, c = ro*logT, d = ro, e = logT;
    N K = dtmp[0]*a + dtmp[1]*b + dtmp[2]*c + dtmp[3]*d + dtmp[4]*e + dtmp[5];
    return exp(K/3.);
}

/// The parameters of a pure fluid: the cube of the diameter, the depth of the potential, the reduced dipole moment squared
/// and the polarizability
template<class N> struct Pure
{
    N sig3, eps, m2r, a;
};

/// The reduced free energy of a pure fluid (the mixture of one component), T in K and ro in the units of the model
template<class N> auto FTOTAL(const N& T, const N& ro, const Pure<N>& p) -> N
{
    const N rotmp = NA*ro;
    const N T2R = T*T;
    const N s3 = p.sig3;
    const N epspar = p.sig3*p.eps;      // MIXES3(0,0)

    const N A0 = FWCA<N>(T/p.eps, s3*rotmp);   // emix = eps, s3mix = sig3

    // dipole part
    N Jdp = J6LJ<N>(T/p.eps, s3*rotmp);
    N A2 = p.m2r*p.m2r*Jdp/s3;
    A2 = -A2*TWOPI*rotmp/(3.*T2R);

    N AP = 0.;
    if (value(A2) != 0.)
    {
        const N IKt = K23_13<N>(T*s3/epspar, s3*rotmp);
        const N IK = IKt*IKt*IKt;
        N A3 = p.m2r*p.m2r*p.m2r*IK*pow(s3*s3*s3, -1./3.);
        A3 = A3*32.*sqrt(14.*PI_1/5.)*rotmp*rotmp*PI_1*PI_1*PI_1/(135.*T*T2R);
        AP = A2/(1. - A3/A2);
    }

    // induced interaction
    const N Jind = J6LJ<N>(T*s3/epspar, s3*rotmp);
    N A1 = (p.a*p.m2r + p.a*p.m2r)*Jind/s3;
    A1 = -A1*TWOPI*rotmp/T;

    return A0 + A1 + AP;
}

/// The compressibility factor Z = 1 + ro dF/dro of a pure fluid (exact, was a finite difference)
template<class N> auto ZTOTAL(const N& T, const N& ro, const Pure<N>& p) -> N
{
    using M = Up<N>;
    auto f = [&](const M& r) -> M
    {
        Pure<M> q{up<N>(p.sig3), up<N>(p.eps), up<N>(p.m2r), up<N>(p.a)};
        return FTOTAL<M>(up<N>(T), r, q);
    };
    return 1. + ro*derivativeOf<N>(f, ro);
}

/// The parameters of the pure fluid at T from the 12 coefficients of the substance (as in TCGFcalc::CGFugacityPT)
template<class N> auto parametersAt(const N& T, const N* c) -> Pure<N>
{
    const N sigma = c[0] + c[4]*exp(T*c[5]);
    const N eps   = c[1] + c[6]*exp(T*c[7]);
    const N m     = c[2] + c[8]/(T+c[9]);
    const N a     = c[3] + c[10]/(T+c[11]);
    Pure<N> p;
    p.sig3 = sigma*sigma*sigma;
    p.eps = eps;
    p.m2r = m*m/(1.38048E-4);
    p.a = a;
    return p;
}

} // namespace cgfref
} // namespace ThermoFun

#endif // THERMOFUN_CGFREFERENCE_HPP
