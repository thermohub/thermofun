#ifndef THERMOFUN_CGFPURE_HPP
#define THERMOFUN_CGFPURE_HPP

// The free energy of a pure fluid of the Churakov and Gottschalk (2003) EOS and its derivatives, analytical (see CGFanalytic.hpp).
// Replaces the finite differences of the original implementation (the compressibility factor, the internal energy of the reference
// fluid and the residual entropy were calculated with a relative step of 1e-5).

#include "Substances/Gases/CGFanalytic.hpp"

namespace ThermoFun {
namespace cgf {

/// The WCA free energy and its derivatives at (beta, rho) given as numbers of the pass. The diameter of the hard spheres is solved for the
/// value; its derivative along the pass follows from the analytical derivatives d_beta, d_rho (implicit differentiation).
inline auto wcaAt(const real& beta, const real& ro) -> WcaDerivatives
{
    const double d0 = hardSphereDiameter(beta.val(), ro.val());
    WcaDerivatives o0;
    wcaDerivatives(real(beta.val()), real(ro.val()), real(d0), o0);
    real d(d0);
    d[1] = o0.d_b.val()*beta[1] + o0.d_r.val()*ro[1];
    WcaDerivatives o;
    wcaDerivatives(beta, ro, d, o);
    return o;
}

/// The free energy of a pure fluid F(T, rho), dF/drho at constant T, and dF/dT at constant rho (with the parameters of the EOS as functions of T
/// given by the 12 coefficients of the substance: sigma = c0 + c4 exp(c5 T), eps = c1 + c6 exp(c7 T), m = c2 + c8/(T + c9), a = c3 + c10/(T + c11))
inline auto pureFluid(const real& T, const real& rho, const real* c, PureFluidDerivatives& o) -> void
{
    real b0, r0;
    pureFluidArguments(T, rho, c, b0, r0);
    const WcaDerivatives w = wcaAt(b0, r0);
    pureFluidDerivatives(T, rho, c, w, o);
}

/// The same for parameters of the EOS that do not depend on T (sigma, eps, m, a), then dF/dT is at constant parameters
inline auto pureFluid(const real& T, const real& rho, const real& sigma, const real& eps, const real& m, const real& a, PureFluidDerivatives& o) -> void
{
    const real c[12] = {sigma, eps, m, a, real(0.0), real(0.0), real(0.0), real(0.0), real(0.0), real(0.0), real(0.0), real(0.0)};
    pureFluid(T, rho, c, o);
}

} // namespace cgf
} // namespace ThermoFun

#endif // THERMOFUN_CGFPURE_HPP
