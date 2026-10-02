#ifndef THERMOFUN_JETS_HPP
#define THERMOFUN_JETS_HPP

#include <autodiff/forward/dual.hpp>

#include "Common/Real.hpp"
#include "Common/ThermoProperty.hpp"

namespace ThermoFun {

/// Exact partial derivatives of a function of the temperature and the pressure up to the third order, without finite
/// differences. The function is evaluated with autodiff higher-order dual numbers (`dual3rd`) in the two variables and the
/// derivatives are extracted. They are the values and the derivatives (`ddt`, `ddp`) of the first and second order
/// derivatives of the properties of the solvent and of the dielectric constant.
namespace jets {

using Dual = autodiff::dual3rd;

/// The value and the partial derivatives of a function f(T, P): f, f_T, f_P, f_TT, f_TP, f_PP, f_TTT, f_TTP, f_TPP, f_PPP
struct Jet
{
    double f = 0, fT = 0, fP = 0, fTT = 0, fTP = 0, fPP = 0, fTTT = 0, fTTP = 0, fTPP = 0, fPPP = 0;
};

/// The jet of a function `Dual f(Dual T, Dual P)` at (T, P) (the function must return a `Dual`, not an expression)
template<class F>
auto jet(const F& f, double T, double P) -> Jet
{
    using namespace autodiff;
    Dual x = T, y = P;
    Jet j;
    j.f   = val(f(x, y));
    j.fT  = derivative<1>(f, wrt(x), at(x, y));
    j.fP  = derivative<1>(f, wrt(y), at(x, y));
    j.fTT = derivative<2>(f, wrt(x, x), at(x, y));
    j.fTP = derivative<2>(f, wrt(x, y), at(x, y));
    j.fPP = derivative<2>(f, wrt(y, y), at(x, y));
    j.fTTT = derivative<3>(f, wrt(x, x, x), at(x, y));
    j.fTTP = derivative<3>(f, wrt(x, x, y), at(x, y));
    j.fTPP = derivative<3>(f, wrt(x, y, y), at(x, y));
    j.fPPP = derivative<3>(f, wrt(y, y, y), at(x, y));
    return j;
}

/// The third-order Taylor polynomial in (dT, dP) of a quantity given with its derivatives (the dual numbers dT and dP
/// are the increments of T and P from the point of the derivatives)
inline auto taylor(const Jet& c, const Dual& dT, const Dual& dP) -> Dual
{
    return c.f + c.fT*dT + c.fP*dP
         + 0.5*(c.fTT*dT*dT + 2.0*c.fTP*dT*dP + c.fPP*dP*dP)
         + (c.fTTT*dT*dT*dT + 3.0*c.fTTP*dT*dT*dP + 3.0*c.fTPP*dT*dP*dP + c.fPPP*dP*dP*dP)/6.0;
}

/// The jet of the density of the solvent from its properties: the density, its first derivatives, and the derivatives of
/// these (the second derivatives and, from the `ddt` and `ddp` of the second derivatives, the third derivatives)
template<class Props>
auto densityJet(const Props& ps) -> Jet
{
    Jet c;
    c.f   = ps.density.val();
    c.fT  = ps.densityT.val();
    c.fP  = ps.densityP.val();
    c.fTT = ps.densityTT.val();
    c.fTP = ps.densityT.ddp();      // the derivative of densityT with respect to P
    c.fPP = ps.densityP.ddp();      // the derivative of densityP with respect to P
    c.fTTT = ps.densityTT.ddt();
    c.fTTP = ps.densityTT.ddp();
    c.fTPP = ps.densityPP.ddt();
    c.fPPP = ps.densityPP.ddp();
    return c;
}

/// A value of the pass with its derivative: the value v, and the derivatives dT and dP of v with respect to T and P
inline auto along(const Reaktoro_::Pass& pass, double v, double dT, double dP) -> real
{
    real r(v);
    r[1] = (pass.wrt == Reaktoro_::Wrt::T) ? dT : (pass.wrt == Reaktoro_::Wrt::P) ? dP : 0.0;
    return r;
}

} // namespace jets
} // namespace ThermoFun

#endif // THERMOFUN_JETS_HPP
