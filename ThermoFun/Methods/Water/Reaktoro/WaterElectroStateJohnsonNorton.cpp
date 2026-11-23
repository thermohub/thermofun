// SPDX-License-Identifier: LGPL-2.1-or-later
// Reaktoro is a unified framework for modeling chemically reactive systems.
//
// Copyright (C) 2014-2015 Allan Leal
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "WaterElectroStateJohnsonNorton.hpp"

// Reaktoro includes
#include <Methods/Water/Reaktoro/WaterElectroState.hpp>
#include <Methods/Water/Reaktoro/WaterThermoState.hpp>

namespace ThermoFun {
namespace {

//------------------------------------------------------------------------------------------
// Reference:
//------------------------------------------------------------------------------------------
// Johnson, J. W. and Norton, D., 1991, Critical phenomena in hydrothermal system: State,
// thermodynamic, electrostatic, and transport properties of H2O in the critical region,
// Amer. Jour. Sci., v. 291, pp. 541-648.
//------------------------------------------------------------------------------------------

/// k_i(t), and its first and second derivative. a: a1 to a10, in the places 1 to 10.
struct Term
{
    real k, k_t, k_tt;
};

auto term(int i, const real& t, const double* a) -> Term
{
    switch (i)
    {
        case 0: return {real(1.0), real(0.0), real(0.0)};
        case 1: return {a[1]/t, -a[1]/(t*t), 2*a[1]/(t*t*t)};
        case 2: return {a[2]/t + a[3] + a[4]*t, -a[2]/(t*t) + a[4], 2*a[2]/(t*t*t)};
        case 3: return {a[5]/t + a[6]*t + a[7]*t*t, -a[5]/(t*t) + a[6] + 2*a[7]*t, 2*a[5]/(t*t*t) + 2*a[7]};
        default: return {a[8]/t/t + a[9]/t + a[10], -2*a[8]/(t*t*t) - a[9]/(t*t), 6*a[8]/(t*t*t*t) + 2*a[9]/(t*t*t)};
    }
}

} // namespace

auto waterElectroStateJohnsonNorton(real T, /*real P,*/ const WaterThermoState& wt, const std::vector<double>& constants, int state) -> WaterElectroState
{
    const double a[] = {0.0, constants[0], constants[1], constants[2], constants[3], constants[4], constants[5], constants[6], constants[7], constants[8], constants[9]};
	WaterElectroState we;

    we.epsilon   = 0.0;
    we.epsilonT  = 0.0;
    we.epsilonP  = 0.0;
    we.epsilonTT = 0.0;
    we.epsilonTP = 0.0;
    we.epsilonPP = 0.0;

	const auto alpha  = -wt.densityT/wt.density;
    const auto beta   =  wt.densityP/wt.density;
    const auto alphaT = -wt.densityTT/wt.density + alpha*alpha;
    const auto betaT  =  wt.densityTP/wt.density + alpha*beta;
    const auto betaP  =  wt.densityPP/wt.density - beta*beta;

    const auto Tr = constants[10];
    const auto Dr = constants[11];

	const auto t = T/Tr;
	const auto r = wt.density/Dr;

	for(int i = 0; i <= 4; ++i)
	{
		const auto ri    = pow(r, i);
		const auto ti    = term(i, t, a);
		const auto ki    = ti.k;
		const auto ki_t  = ti.k_t/Tr;
		const auto ki_tt = ti.k_tt/Tr/Tr;

		we.epsilon   += ki*ri;
		we.epsilonT  += ri*(ki_t - i*alpha*ki);
		we.epsilonP  += ri*ki*i*beta;
		we.epsilonTT += ri*(ki_tt - i*(alpha*ki_t + ki*alphaT) - i*alpha*(ki_t - i*alpha*ki));
		we.epsilonTP += ri*ki*i*beta*(ki_t/ki - i*alpha + betaT/beta);
		we.epsilonPP += ri*ki*i*beta*(i*beta + betaP/beta);
	}

	const auto epsilon2 = we.epsilon * we.epsilon;

	we.bornZ = -1.0/we.epsilon;
	we.bornY = we.epsilonT/epsilon2;
	we.bornQ = we.epsilonP/epsilon2;
	we.bornU = we.epsilonTP/epsilon2 - 2.0*we.bornY*we.bornQ*we.epsilon;
	we.bornN = we.epsilonPP/epsilon2 - 2.0*we.bornQ*we.bornQ*we.epsilon;
	we.bornX = we.epsilonTT/epsilon2 - 2.0*we.bornY*we.bornY*we.epsilon;

	return we;
}

} // namespace Reaktoro
