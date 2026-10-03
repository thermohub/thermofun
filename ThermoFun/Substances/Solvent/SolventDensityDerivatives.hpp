#ifndef THERMOFUN_SOLVENTDENSITYDERIVATIVES_HPP
#define THERMOFUN_SOLVENTDENSITYDERIVATIVES_HPP

#include "Common/ThermoProperty.hpp"
#include "ThermoProperties.h"

namespace ThermoFun {

/// The density of the solvent and its partial derivatives with respect to T (K) and P (Pa), up to the second order, as autodiff
/// numbers of the pass: their derivatives are the derivatives of one order higher (the third-order derivatives). The second
/// derivatives are taken from the derivatives of the first derivatives, which every solvent model provides (`densityT.ddp`
/// is rho_TP, `densityP.ddp` is rho_PP), the third-order ones from the second derivatives (`densityTT.ddt` is rho_TTT, ...).
struct DensityDerivatives
{
    real rho, T, P, TT, TP, PP;
};

inline auto densityDerivatives(const Reaktoro_::Pass& pass, const PropertiesSolvent& ps) -> DensityDerivatives
{
    const double rho_TTP = ps.densityTT.ddp();                                         // = densityTP.ddt
    const double rho_TPP = ps.densityPP.ddt() != 0.0 ? ps.densityPP.ddt() : ps.densityTP.ddp();
    DensityDerivatives d;
    d.rho = pass(ps.density);
    d.T   = pass(ps.densityT);
    d.P   = pass(ps.densityP);
    d.TT  = pass(ps.densityTT);
    d.TP  = Reaktoro_::along(pass, ps.densityT.ddp(), rho_TTP, rho_TPP);
    d.PP  = Reaktoro_::along(pass, ps.densityP.ddp(), rho_TPP, ps.densityPP.ddp());
    return d;
}

} // namespace ThermoFun

#endif // THERMOFUN_SOLVENTDENSITYDERIVATIVES_HPP
