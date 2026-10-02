#!/usr/bin/env python3
"""Symbolic derivation of the analytical derivatives of the free energy of the Churakov and Gottschalk (2003) EOS of a pure fluid.

Generates ThermoFun/Substances/Gases/CGFanalytic.hpp (do not edit that file: change this script and run it again):
    python3 ThermoFun/Substances/Gases/tools/generate_cgf_derivatives.py       (needs sympy)

1. The free energy of the Weeks-Chandler-Andersen reference fluid F(beta, rho) = F0(nu) + 48 nuw (I1_12 u^12 - I1_6 u^6 + I2) beta + RPA(beta, nuw),
   u = 1/dW, nu = pi/6 d^3 rho, nuw = nu (1 - nu/16), dW = d (1 - nu/16)^(1/3), where the hard-sphere diameter d(beta, rho) is the root of the
   polynomial G(d; beta, rho) = 0 (the equation solved with Newton's iterations in dHS). The partial derivatives of F up to the second order are
   derived with the implicit differentiation of G: d_x = -G_x/G_d, d_xy = -(G_xy + G_xd d_y + G_yd d_x + G_dd d_x d_y)/G_d, and the chain rule.
2. The free energy of a pure fluid F_total(T, rho) = A0 + A1 + AP (WCA, induced and dipole terms) as a function of the WCA free energy and its
   derivatives at (beta0, rho0) = (eps/T, s3 NA rho), and of the parameters sigma(T), eps(T), m(T), a(T): its derivatives with respect to rho and to T
   (including the dependence of the parameters on T) by the chain rule.
"""
import os
import sympy as sp
from sympy.printing.c import C99CodePrinter

# ----------------------------------------------------------------------------------------------------------------------
# constants of the model (TCGFcalc::set_internal)
K = dict(PI=sp.pi, PISIX=sp.Float('0.523598775598298927'), TWOPI=sp.Float('6.283185307179586230'), rm=sp.Float('1.12246204830937302'),
         P1=sp.Float('1.186892378996'), PP2=sp.Float('-0.4721963005527'), P3=sp.Float('3.259515855283'), P4=sp.Float('3.055229342609'),
         P5=sp.Float('1.095409321023'), P6=sp.Float('1.282306659774E-2'), P7=sp.Float('9.55712461425E-2'), P8=sp.Float('13.67807693107'),
         P9=sp.Float('35.75464856619'), P10=sp.Float('16.04724381643'),
         AA1=sp.Float('-0.120078459237'), AA2=sp.Float('-.808712488307'), AA3=sp.Float('.321543801337'),
         A4=sp.Float('1.16965477132'), A5=sp.Float('-.410564939543'), A6=sp.Float('-.516834310691'),
         BB1=sp.Float('-2.18839961483'), BB2=sp.Float('1.59897428009'), BB3=sp.Float('-.392578806128'),
         B4=sp.Float('-.189396607904'), B5=sp.Float('-.576898496254'), B6=sp.Float('-0.0185167641359'),
         A00=sp.Float('.9985937977069455'), A01=sp.Float('.5079834224407451'), A10=sp.Float('1.021887697885469'),
         A11=sp.Float('-5.136619463333883'), A12=sp.Float('-5.196188074016755'), A21=sp.Float('-6.049240839050804'),
         A22=sp.Float('18.67848155616692'), A23=sp.Float('20.10652684217768'), A31=sp.Float('9.896491419756988'),
         A32=sp.Float('14.6738380473899'), A33=sp.Float('-77.44825116542995'), A34=sp.Float('-4.82871082941229'))
globals().update(K)
NA = sp.Float('0.6023')

b, r, d = sp.symbols('b r d', positive=True)   # beta, rho, hard sphere diameter


class Printer(C99CodePrinter):
    """C++ code with products for the small integer powers and pow/sqrt/exp/log of the autodiff numbers"""
    def _print_Pow(self, expr):
        base, e = expr.as_base_exp()
        if e == sp.Rational(1, 2):
            return 'sqrt(%s)' % self._print(base)
        if e == -sp.Rational(1, 2):
            return '(1.0/sqrt(%s))' % self._print(base)
        if e.is_Integer:
            n = int(e)
            bs = self._print(base)
            if not (base.is_Symbol or base.is_Number):
                bs = '(' + bs + ')'
            if 1 <= n <= 4:
                return '(' + '*'.join([bs] * n) + ')'
            if -4 <= n <= -1:
                return '(1.0/(' + '*'.join([bs] * (-n)) + '))'
            return 'pow(%s, %d)' % (bs, n)
        return 'pow(%s, %s)' % (self._print(base), self._print(sp.Float(e) if e.is_Rational else e))

    def _print_Float(self, expr):
        return repr(float(expr))

    def _print_Rational(self, expr):
        return '(%d.0/%d.0)' % (expr.p, expr.q)

    def _print_Integer(self, expr):
        return '%d.0' % int(expr)

    def _print_Symbol(self, expr):
        return str(expr)


def code(exprs, names, prefix):
    """common subexpressions and the lines of code"""
    repl, red = sp.cse(exprs, symbols=sp.numbered_symbols(prefix), optimizations='basic')
    pr = Printer()
    lines = ['const auto %s = %s;' % (s, pr.doprint(e)) for s, e in repl]
    lines += ['%s = %s;' % (n, pr.doprint(e)) for n, e in zip(names, red)]
    return lines


# ----------------------------------------------------------------------------------------------------------------------
# 1. the WCA free energy and its derivatives
dB = (P1*b**sp.Rational(1, 12) + PP2*b**sp.Rational(7, 12) + (P3 + (P4 + P5*b)*b)*b)/(1 + b)**3
sb = sp.sqrt(b)
delta = (P6 + P7*sb)/(1 + (P8 + (P9 + P10*sb)*sb)*sb)
dbdl = dB*delta
ri6ro = PISIX*r
c0_, c3_, c4_, c6_, c7_, c9_, c12_ = dB + dbdl, (-sp.Rational(3, 2)*dB - sp.Float('3.75')*dbdl)*ri6ro, sp.Rational(3, 2)*ri6ro, \
    (2*dB + dbdl)*sp.Rational(1, 4)*ri6ro**2, -sp.Rational(1, 2)*ri6ro**2, -sp.Float('2.89325')*ri6ro**3*dbdl, -sp.Float('0.755')*ri6ro**4*dbdl
G = c0_ - d + c3_*d**3 + c4_*d**4 + c6_*d**6 + c7_*d**7 + c9_*d**9 + c12_*d**12

nu = PISIX*d**3*r
t1 = 1 - nu/16
nuw = nu*t1
dW = d*t1**sp.Rational(1, 3)
u = 1/dW
F0 = (4 - 3*nu)*nu/(1 - nu)**2
fa0 = (A00 + A01*nuw)/(1 - nuw)**2
fa1 = (A10 + (A11 + A12*nuw)*nuw)/(1 - nuw)**3
fa2 = ((A21 + (A22 + A23*nuw)*nuw)*nuw)/(1 - nuw)**4
fa3 = ((A31 + (A32 + (A33 + A34*nuw)*nuw)*nuw)*nuw)/(1 - nuw)**5
I1_6 = (1 + (A4 + (A5 + A6*nuw)*nuw)*nuw)/((1 + (AA1 + (AA2 + AA3*nuw)*nuw)*nuw)*3)
I1_12 = (1 + (B4 + (B5 + B6*nuw)*nuw)*nuw)/((1 + (BB1 + (BB2 + BB3*nuw)*nuw)*nuw)*9)
rmdw = rm*u
tt1 = (fa0/4 + fa1/12 + fa2/24 + fa3/24)*u**6
tt2 = (fa0/10 + fa1/90 + fa2/720 + fa3/5040)*(-u**12)
tt3 = (fa0 - fa1/3 + fa2/12 - fa3/60)/8
tt4 = (fa0 - fa1 + fa2/2 - fa3/6)*rmdw**2*(-9)/40
tt5 = (fa1 - fa2 + fa3/2)*rmdw**3*(-2)/9
tt6 = (fa2 - fa3)*rmdw**4*(-9)/64
tt7 = fa3*(-3)/35*rmdw**5
I2 = tt1 + tt2 + tt3 + tt4 + tt5 + tt6 + tt7
F1 = 48*nuw*(I1_12*u**12 - I1_6*u**6 + I2)*b
fi1 = (sp.Float('1.20110') + (sp.Float('0.064890') + (-sp.Float('76.860') + (sp.Float('562.686') + (-sp.Float('2280.090') + (sp.Float('6266.840') + (-sp.Float('11753.40') + (sp.Float('14053.8')
      + (-sp.Float('9491.490') + sp.Float('2731.030')*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw
fi2 = (sp.Float('0.588890') + (-sp.Float('7.455360') + (sp.Float('40.57590') + (-sp.Float('104.8970') + (sp.Float('60.25470') + (sp.Float('390.6310') + (-sp.Float('1193.080')
      + (sp.Float('1576.350') + (-sp.Float('1045.910') + sp.Float('283.7580')*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw)*nuw*nuw
FA = (-12*fi1 + 192*fi2*b)*b*b/sp.pi
F = F0 + F1 + FA

var = (b, r)
Gd, Gdd = sp.diff(G, d), sp.diff(G, d, 2)
dx = {x: -sp.diff(G, x)/Gd for x in var}
dxy = {}
for i, x in enumerate(var):
    for y in var[i:]:
        dxy[(x, y)] = -(sp.diff(G, x, y) + sp.diff(G, x, d)*dx[y] + sp.diff(G, y, d)*dx[x] + Gdd*dx[x]*dx[y])/Gd

Fd = sp.diff(F, d)
Fx = {x: sp.diff(F, x) + Fd*dx[x] for x in var}
Fxy = {}
for i, x in enumerate(var):
    for y in var[i:]:
        Fxy[(x, y)] = (sp.diff(F, x, y) + sp.diff(F, x, d)*dx[y] + sp.diff(F, y, d)*dx[x] + sp.diff(F, d, 2)*dx[x]*dx[y] + Fd*dxy[(x, y)])

wca_names = ['o.F', 'o.Fb', 'o.Fr', 'o.Fbb', 'o.Fbr', 'o.Frr', 'o.d_b', 'o.d_r']
wca_exprs = [F, Fx[b], Fx[r], Fxy[(b, b)], Fxy[(b, r)], Fxy[(r, r)], dx[b], dx[r]]
wca_lines = code(wca_exprs, wca_names, 'w')

# ----------------------------------------------------------------------------------------------------------------------
# 2. the pure fluid
T, rho = sp.symbols('T rho', positive=True)
c = sp.symbols('c0:12')
FW, FWr, FWb, FWrr, FWrb, FWbb = sp.symbols('FW FWr FWb FWrr FWrb FWbb')
sigma = c[0] + c[4]*sp.exp(T*c[5])
eps = c[1] + c[6]*sp.exp(T*c[7])
m = c[2] + c[8]/(T + c[9])
a_ = c[3] + c[10]/(T + c[11])
s3 = sigma**3
m2r = m**2/sp.Float('1.38048E-4')
b0 = eps/T
r0 = s3*NA*rho
rotmp = NA*rho
J = (r0*FWr - 4*b0*FWb)/(16*sp.pi*r0*b0)
A0 = FW
A2 = -(m2r*m2r*J/s3)*K['TWOPI']*rotmp/(3*T**2)
logT = sp.log(T/eps)
kd = [sp.Float('-1.050534'), sp.Float('1.747476'), sp.Float('1.749366'), sp.Float('-1.999227'), sp.Float('-0.661046'), sp.Float('-3.028720')]
Kf = sp.exp((kd[0]*r0**2*logT + kd[1]*r0**2 + kd[2]*r0*logT + kd[3]*r0 + kd[4]*logT + kd[5])/3)
A3 = m2r**3*Kf**3/s3*32*sp.sqrt(14*sp.pi/5)*rotmp**2*sp.pi**3/(135*T*T**2)
AP = A2/(1 - A3/A2)
A1 = -((a_*m2r + a_*m2r)*J/s3)*K['TWOPI']*rotmp/T
Theta = A0 + A1 + AP

# total derivatives: the WCA quantities FW.. are functions of (b0, r0)
W_der = {FW: (FWb, FWr), FWr: (FWrb, FWrr), FWb: (FWbb, FWrb)}   # (d/db0, d/dr0)


def D(expr, x):
    res = sp.diff(expr, x)
    for q, (qb, qr) in W_der.items():
        res += sp.diff(expr, q)*(qb*sp.diff(b0, x) + qr*sp.diff(r0, x))
    return res


Th_r, Th_T = D(Theta, rho), D(Theta, T)
# the polar terms: A2 = A3 = 0 for m = 0, then AP = 0 (the expression is 0/0): the terms are generated for A2 != 0 only
AP_r, AP_T = D(AP, rho), D(AP, T)
nonpolar = [A0 + A1, D(A0 + A1, rho), D(A0 + A1, T)]
polar = [AP, AP_r, AP_T]
np_lines = code(nonpolar, ['F', 'Frho', 'FT'], 'n')
pol_lines = code(polar, ['dF', 'dFrho', 'dFT'], 'p')
bw_lines = code([b0, r0], ['b0', 'r0'], 'q')

# ----------------------------------------------------------------------------------------------------------------------
header = '''// Generated by ThermoFun/Substances/Gases/tools/generate_cgf_derivatives.py: do not edit.
//
// Analytical derivatives of the free energy of the Churakov and Gottschalk (2003) EOS of a pure fluid, derived symbolically (sympy):
//  - wcaDerivatives: the free energy of the Weeks-Chandler-Andersen reference fluid F(beta, rho) and its partial derivatives up to the
//    second order, with the implicit differentiation of the hard sphere diameter d(beta, rho) (the root of a polynomial);
//  - pureFluidDerivatives: the free energy of the pure fluid (WCA, induced and dipole terms) and its derivatives with respect to the
//    density and to the temperature (including the temperature dependence of the parameters of the EOS).
// The formulas are evaluated with `real` numbers: the derivatives along the pass (the T or P derivatives of the properties) of
// these analytical expressions are the derivatives of the properties.

#ifndef THERMOFUN_CGFANALYTIC_HPP
#define THERMOFUN_CGFANALYTIC_HPP

#include <cmath>

#include "Common/Real.hpp"

namespace ThermoFun {
namespace cgf {

/// the WCA free energy and its partial derivatives with respect to beta = 1/T and rho (reduced units), and those of the diameter
struct WcaDerivatives
{
    real F, Fb, Fr, Fbb, Fbr, Frr, d_b, d_r;
};

/// the diameter d(beta, rho) of the hard spheres: the root of the polynomial of the Newton's iterations of the model (values only)
inline auto hardSphereDiameter(double beta, double ro) -> double
{
    const double DV112 = 1./12., DV712 = 7./12.;
    const double T12 = std::sqrt(beta);
    const double T112 = std::exp(DV112*std::log(beta));
    const double T712 = std::exp(DV712*std::log(beta));
    double B13 = (1+beta);
    B13 = B13*B13*B13;
    const double dB = (%(P1)s*T112+(%(PP2)s)*T712+(%(P3)s+(%(P4)s+%(P5)s*beta)*beta)*beta)/B13;
    const double delta = (%(P6)s+%(P7)s*T12)/(1.+(%(P8)s+(%(P9)s+%(P10)s*T12)*T12)*T12);
    const double dbdl = dB*delta;
    const double ri6ro = %(PISIX)s*ro;
    const double ri6ro2 = ri6ro*ri6ro;
    const double a0 = dB+dbdl, a1 = -1.;
    const double a3 = (-1.5*dB -3.75*dbdl)*ri6ro;
    const double a4 = (1.5*ri6ro);
    const double a6 = (2.*dB + dbdl)*0.25*ri6ro2;
    const double a7 = -0.5*ri6ro2;
    const double a9 = -2.89325*ri6ro2*ri6ro*dbdl;
    const double a12 = -0.755*ri6ro2*ri6ro2*dbdl;
    const double p0 = -1., p2 = a3*3., p3 = a4*4., p5 = a6*6., p6 = a7*7., p8 = a9*9., p11 = a12*12.;
    double dd = dB;
    for (int i = 0; i < 100; ++i)
    {
        const double d2 = dd*dd, d3 = dd*dd*dd;
        const double F0 = a0+(a1+(a3+(a4+(a6+(a7+(a9+a12*d3)*d2)*dd)*d2)*dd)*d2)*dd;
        const double F1 = p0+(p2+(p3+(p5+(p6+(p8+p11*d3)*d2)*dd)*d2)*dd)*d2;
        const double dnew = dd-F0/F1;
        const bool done = std::fabs(dnew-dd) < 1.E-14;
        dd = dnew;
        if (done) break;
    }
    return dd;
}

/// The WCA free energy and its derivatives at (beta, rho); the diameter is a number of the pass with its (analytical) derivatives
inline auto wcaDerivatives(const real& b, const real& r, const real& d, WcaDerivatives& o) -> void
{
%(wca)s
}

/// The free energy of a pure fluid of the EOS and its partial derivatives with respect to rho and T at constant rho or T
/// (the parameters of the EOS depend on T). The WCA quantities are those at (b0, r0), see pureFluidArguments.
struct PureFluidDerivatives
{
    real F, Frho, FT;
};

/// The arguments (b0, r0) of the WCA free energy of a pure fluid from the 12 coefficients of the substance
inline auto pureFluidArguments(const real& T, const real& rho, const real* c, real& b0, real& r0) -> void
{
    const real& c0 = c[0]; const real& c1 = c[1]; const real& c2 = c[2]; const real& c3 = c[3]; const real& c4 = c[4]; const real& c5 = c[5];
    const real& c6 = c[6]; const real& c7 = c[7]; const real& c8 = c[8]; const real& c9 = c[9]; const real& c10 = c[10]; const real& c11 = c[11];
%(bw)s
}

/// @param T temperature (K), rho density (units of the model), c the 12 coefficients of the substance
/// @param w the WCA free energy and its derivatives at (b0, r0)
inline auto pureFluidDerivatives(const real& T, const real& rho, const real* c, const WcaDerivatives& w, PureFluidDerivatives& o) -> void
{
    const real& FW = w.F; const real& FWr = w.Fr; const real& FWb = w.Fb;
    const real& FWrr = w.Frr; const real& FWrb = w.Fbr; const real& FWbb = w.Fbb;
    const real& c0 = c[0]; const real& c1 = c[1]; const real& c2 = c[2]; const real& c3 = c[3]; const real& c4 = c[4]; const real& c5 = c[5];
    const real& c6 = c[6]; const real& c7 = c[7]; const real& c8 = c[8]; const real& c9 = c[9]; const real& c10 = c[10]; const real& c11 = c[11];
    real F, Frho, FT;
    {
%(np)s
    }
    // the dipole terms (zero if the dipole moment is zero)
    const real mm = c[2] + c[8]/(T + c[9]);
    if (mm.val() != 0.0)
    {
        real dF, dFrho, dFT;
%(pol)s
        F += dF; Frho += dFrho; FT += dFT;
    }
    o.F = F; o.Frho = Frho; o.FT = FT;
}

} // namespace cgf
} // namespace ThermoFun

#endif // THERMOFUN_CGFANALYTIC_HPP
''' % dict({k: repr(float(v)) if k not in ('PI',) else 'M_PI' for k, v in K.items()},
           wca='\n'.join('    ' + l for l in wca_lines),
           bw='\n'.join('    ' + l for l in bw_lines),
           np='\n'.join('        ' + l for l in np_lines),
           pol='\n'.join('        ' + l for l in pol_lines))
path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'CGFanalytic.hpp')
open(path, 'w').write(header)
print('written', os.path.normpath(path), len(header), 'bytes;', len(wca_lines), 'wca lines,', len(np_lines), len(pol_lines))
