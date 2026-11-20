// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "WaterElectroHelgesonKirkham74.h"
#include <ThermoFun/Common/Constants.h>
#include <ThermoFun/Common/ThermoProperties.h>
#include "Methods/Water/SolventDensityDerivatives.hpp"

namespace ThermoFun {

/// The dielectric constant of water of Helgeson and Kirkham (1974), as in Appendix E of Tanger and Helgeson (1988):
/// eps = sum e_ij T^i rho^j for i = 0..4, j = 0..4-i, T in K, rho in g/cm3. The derivatives are analytical, with those of the density
/// from the solvent model. The constants are e_ij by rows.
auto electroPropertiesWaterHelgesonKirkham74(const Reaktoro_::Pass& pass, const PropertiesSolvent& solvent, const std::vector<double>& constants) -> ElectroPropertiesSolventAD
{
    const auto r = densityDerivatives(pass, solvent);
    const real T = pass.T;
    const real rho = r.rho/1000.0;
    const real rhoT = r.T/1000.0, rhoP = r.P/1000.0, rhoTT = r.TT/1000.0, rhoTP = r.TP/1000.0, rhoPP = r.PP/1000.0;

    real eps = 0.0, epsT = 0.0, epsP = 0.0, epsTT = 0.0, epsTP = 0.0, epsPP = 0.0;
    std::size_t place = 0;
    for (int i = 0; i <= 4; ++i)
        for (int j = 0; j <= 4 - i; ++j)
        {
            const double e = constants[place++];
            const real Ti = pow(T, double(i)), Tl = i > 0 ? pow(T, double(i - 1)) : real(0.0), Tm = i > 1 ? pow(T, double(i - 2)) : real(0.0);
            const real Dj = pow(rho, double(j)), Dl = j > 0 ? pow(rho, double(j - 1)) : real(0.0), Dm = j > 1 ? pow(rho, double(j - 2)) : real(0.0);
            eps   += e*Ti*Dj;
            epsT  += e*(i*Tl*Dj + j*Ti*Dl*rhoT);
            epsP  += e*j*Ti*Dl*rhoP;
            epsTT += e*(i*(i - 1)*Tm*Dj + 2.0*i*j*Tl*Dl*rhoT + j*(j - 1)*Ti*Dm*rhoT*rhoT + j*Ti*Dl*rhoTT);
            epsTP += e*(i*j*Tl*Dl*rhoP + j*(j - 1)*Ti*Dm*rhoT*rhoP + j*Ti*Dl*rhoTP);
            epsPP += e*(j*(j - 1)*Ti*Dm*rhoP*rhoP + j*Ti*Dl*rhoPP);
        }

    const real epsilon2 = eps*eps;
    ElectroPropertiesSolventAD wep;
    // the pressure derivatives per bar (the derivatives with respect to P in Pa multiplied by bar_to_Pa)
    wep.epsilon   = eps;
    wep.epsilonT  = epsT;
    wep.epsilonP  = epsP*bar_to_Pa;
    wep.epsilonTT = epsTT;
    wep.epsilonTP = epsTP*bar_to_Pa;
    wep.epsilonPP = epsPP*bar_to_Pa*bar_to_Pa;
    wep.bornZ = -1.0/eps;
    wep.bornY = epsT/epsilon2;
    wep.bornQ = epsP/epsilon2;
    wep.bornX = epsTT/epsilon2 - 2.0*wep.bornY*wep.bornY*eps;
    return wep;
}

}
