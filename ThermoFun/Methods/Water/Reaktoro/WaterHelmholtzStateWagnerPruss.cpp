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

#include "WaterHelmholtzStateWagnerPruss.hpp"

// Reaktoro includes (modified DM 11.05.2016)
#include <Common/Real.hpp>
#include <Methods/Water/Reaktoro/WaterThermoStateUtils.hpp>
#include <Methods/Water/Reaktoro/WaterHelmholtzState.hpp>

namespace ThermoFun {
auto waterHelmholtzStateWagnerPruss(real T, real D, const WagnerPrussConstants& k) -> WaterHelmholtzState
{
    const real tau   = k.saturation.Tcr/T;
    const real delta = D/k.saturation.Dcr;

	auto phio     =  log(delta) + k.no[1] + k.no[2]*tau + k.no[3]*log(tau);
	auto phio_d   =  1.0/delta;
	auto phio_t   =  k.no[2] + k.no[3]/tau;
	auto phio_dd  = -1.0/pow(delta, 2);
	auto phio_tt  = -k.no[3]/pow(tau, 2);
	auto phio_dt  =  0.0;
	auto phio_ddd =  2.0/pow(delta, 3);
	auto phio_ttt =  2.0*k.no[3]/pow(tau, 3);
	auto phio_dtt =  0.0;
	auto phio_ddt =  0.0;

	for(int i = 4; i <= 8; ++i)
	{
		const int j = i - 4;

		const auto ee = exp(k.gammao[j] * tau);

		phio     += k.no[i] * log(1.0 - 1.0/ee);
		phio_t   += k.no[i] * (k.gammao[j]/(ee - 1));
		phio_tt  -= k.no[i] * ee * pow((k.gammao[j]/(ee - 1)), 2);
		phio_ttt += k.no[i] * ee * (1 + ee) * pow((k.gammao[j]/(ee - 1)), 3);
	}

    real phir;
    real phir_d;
    real phir_t;
    real phir_dd;
    real phir_tt;
    real phir_dt;
    real phir_ddd;
    real phir_ttt;
    real phir_dtt;
    real phir_ddt;

    phir = 0.0;
    phir_d = 0.0;
    phir_t = 0.0;
    phir_dd = 0.0;
    phir_tt = 0.0;
    phir_dt = 0.0;
    phir_ddd = 0.0;
    phir_ttt = 0.0;
    phir_dtt = 0.0;
    phir_ddt = 0.0;

	for(int i = 1; i <= 7; ++i)
	{
		const auto A     = k.n[i]*pow(delta, k.d[i])*pow(tau, k.t[i]);
		const auto A_d   = k.d[i]/delta * A;
		const auto A_t   = k.t[i]/tau * A;
		const auto A_dd  = (k.d[i] - 1)/delta * A_d;
		const auto A_tt  = (k.t[i] - 1)/tau * A_t;
		const auto A_dt  = k.t[i]*k.d[i]/(tau*delta) * A;
		const auto A_ddd = (k.d[i] - 2)/delta * A_dd;
		const auto A_ttt = (k.t[i] - 2)/tau * A_tt;
		const auto A_dtt = k.d[i]/delta * A_tt;
		const auto A_ddt = k.t[i]/tau * A_dd;

		phir     += A;
		phir_d   += A_d;
		phir_t   += A_t;
		phir_dd  += A_dd;
		phir_tt  += A_tt;
		phir_dt  += A_dt;
		phir_ddd += A_ddd;
		phir_ttt += A_ttt;
		phir_dtt += A_dtt;
		phir_ddt += A_ddt;
	}

	for(int i = 8; i <= 51; ++i)
	{
		const auto dci = pow(delta, k.c[i]);

		const auto B     =  k.n[i]*pow(delta, k.d[i])*pow(tau, k.t[i])*exp(-dci);
		const auto B_d   = (k.d[i] - k.c[i]*dci)/delta * B;
		const auto B_t   =  k.t[i]/tau * B;
		const auto B_dd  = (k.d[i] - k.c[i]*dci - 1)/delta * B_d - dci*pow(k.c[i]/delta, 2) * B;
		const auto B_tt  = (k.t[i] - 1)/tau * B_t;
		const auto B_dt  =  k.t[i]/tau * B_d;
		const auto B_ddd = (k.d[i] - k.c[i]*dci - 1)/delta * B_dd - ((k.d[i] - k.c[i]*dci - 1) + 2*k.c[i]*k.c[i]*dci)/pow(delta, 2) * B_d - k.c[i]*k.c[i]*dci*(k.c[i] - 2)/pow(delta, 3) * B;
		const auto B_ttt = (k.t[i] - 2)/tau * B_tt;
		const auto B_dtt = (k.t[i] - 1)/tau * B_dt;
		const auto B_ddt = (k.d[i] - k.c[i]*dci - 1)/delta * B_dt - k.c[i]*k.c[i]*dci/pow(delta, 2) * B_t;

		phir     += B;
		phir_d   += B_d;
		phir_t   += B_t;
		phir_dd  += B_dd;
		phir_tt  += B_tt;
		phir_dt  += B_dt;
		phir_ddd += B_ddd;
		phir_ttt += B_ttt;
		phir_dtt += B_dtt;
		phir_ddt += B_ddt;
	}

	for(int i = 52; i <= 54; ++i)
	{
		const int j = i - 52;

		const auto aux1d = (k.d[i]/delta - 2*k.alpha[j]*(delta - k.epsilon[j]));
		const auto aux1t = (k.t[i]/tau - 2*k.beta[j]*(tau - k.gamma[j]));

		const auto aux2d = (k.d[i]/pow(delta, 2) + 2*k.alpha[j]);
		const auto aux2t = (k.t[i]/pow(tau, 2) + 2*k.beta[j]);

		const auto C     = k.n[i]*pow(delta, k.d[i])*pow(tau, k.t[i])*exp(-k.alpha[j]*pow(delta - k.epsilon[j], 2) - k.beta[j]*pow(tau - k.gamma[j], 2));
		const auto C_d   = aux1d * C;
		const auto C_t   = aux1t * C;
		const auto C_dd  = aux1d * C_d - aux2d * C;
		const auto C_tt  = aux1t * C_t - aux2t * C;
		const auto C_dt  = aux1d * aux1t * C;
		const auto C_ddd = aux1d * C_dd - 2*aux2d * C_d + 2*k.d[i]/pow(delta, 3) * C;
		const auto C_ttt = aux1t * C_tt - 2*aux2t * C_t + 2*k.t[i]/pow(tau, 3) * C;
		const auto C_dtt = aux1t * C_dt - aux2t * C_d;
		const auto C_ddt = aux1d * C_dt - aux2d * C_t;

		phir     += C;
		phir_d   += C_d;
		phir_t   += C_t;
		phir_dd  += C_dd;
		phir_tt  += C_tt;
		phir_dt  += C_dt;
		phir_ddd += C_ddd;
		phir_ttt += C_ttt;
		phir_dtt += C_dtt;
		phir_ddt += C_ddt;
	}

	for(int i = 55; i <= 56; ++i)
	{
		const int j = i - 55;

		const auto dd = pow(delta - 1, 2);
		const auto tt = pow(tau - 1, 2);

		const auto theta     = (1 - tau) + k.A[j]*pow(dd, 0.5/k.E[j]);
		const auto theta_d   = (theta + tau - 1)/(delta - 1)/k.E[j];
		const auto theta_dd  = (1.0/k.E[j] - 1) * theta_d/(delta - 1);
		const auto theta_ddd = (1.0/k.E[j] - 1) * (theta_dd/(delta - 1) - theta_d/dd);

		const auto psi     = exp(-k.C[j]*dd - k.F[j]*tt);
		const auto psi_d   = -2*k.C[j]*(delta - 1) * psi;
		const auto psi_t   = -2*k.F[j]*(tau - 1) * psi;
		const auto psi_dd  = -2*k.C[j]*(psi + (delta - 1) * psi_d);
		const auto psi_tt  = -2*k.F[j]*(psi + (tau - 1) * psi_t);
		const auto psi_dt  =  4*k.C[j]*k.F[j]*(delta - 1)*(tau - 1) * psi;
		const auto psi_ddd = -2*k.C[j]*(2*psi_d + (delta - 1) * psi_dd);
		const auto psi_ttt = -2*k.F[j]*(2*psi_t + (tau - 1) * psi_tt);
		const auto psi_dtt = -2*k.F[j]*(psi_d + (tau - 1) * psi_dt);
		const auto psi_ddt = -2*k.C[j]*(psi_t + (delta - 1) * psi_dt);

		const auto Delta     = theta*theta + k.B[j]*pow(dd, k.a[j]);
		const auto Delta_d   = 2*(theta*theta_d + k.a[j]*(Delta - theta*theta)/(delta - 1));
		const auto Delta_t   = -2*theta;
		const auto Delta_dd  = 2*(theta_d*theta_d + theta*theta_dd + k.a[j] * ((Delta_d - 2*theta*theta_d)/(delta - 1) - (Delta - theta*theta)/pow(delta - 1, 2)));
		const auto Delta_tt  = 2;
		const auto Delta_dt  = -2*theta_d;
		const auto Delta_ddd = 2*(3*theta_d*theta_dd + theta*theta_ddd + k.a[j] * ((Delta_dd - 2*theta_d*theta_d - 2*theta*theta_dd)/(delta - 1) - 2*(Delta_d - 2*theta*theta_d)/pow(delta - 1, 2) + 2*(Delta - theta*theta)/pow(delta - 1, 3)));
		const auto Delta_ttt = 0;
		const auto Delta_dtt = 0;
		const auto Delta_ddt = -2*theta_dd;

		const auto DeltaPow     =  pow(Delta, k.b[j]);
		const auto DeltaPow_d   =  k.b[j]*Delta_d/Delta * DeltaPow;
		const auto DeltaPow_t   =  k.b[j]*Delta_t/Delta * DeltaPow;
		const auto DeltaPow_dd  = (k.b[j]*Delta_dd/Delta + k.b[j]*(k.b[j] - 1)*pow(Delta_d/Delta, 2)) * DeltaPow;
		const auto DeltaPow_tt  = (k.b[j]*Delta_tt/Delta + k.b[j]*(k.b[j] - 1)*pow(Delta_t/Delta, 2)) * DeltaPow;
		const auto DeltaPow_dt  = (k.b[j]*Delta_dt/Delta + k.b[j]*(k.b[j] - 1)*Delta_d*Delta_t/Delta/Delta) * DeltaPow;
		const auto DeltaPow_ddd = (k.b[j]*Delta_ddd/Delta + 3*k.b[j]*(k.b[j] - 1)*Delta_d*Delta_dd/Delta/Delta + k.b[j]*(k.b[j] - 1)*(k.b[j] - 2)*pow(Delta_d/Delta, 3)) * DeltaPow;
		const auto DeltaPow_ttt = (k.b[j]*Delta_ttt/Delta + 3*k.b[j]*(k.b[j] - 1)*Delta_t*Delta_tt/Delta/Delta + k.b[j]*(k.b[j] - 1)*(k.b[j] - 2)*pow(Delta_t/Delta, 3)) * DeltaPow;
		const auto DeltaPow_dtt = (k.b[j]*Delta_dtt/Delta + k.b[j]*(k.b[j] - 1)*(Delta_d*Delta_tt + 2*Delta_t*Delta_dt)/Delta/Delta + k.b[j]*(k.b[j] - 1)*(k.b[j] - 2)*Delta_t*Delta_t*Delta_d/pow(Delta, 3)) * DeltaPow;
		const auto DeltaPow_ddt = (k.b[j]*Delta_ddt/Delta + k.b[j]*(k.b[j] - 1)*(Delta_t*Delta_dd + 2*Delta_d*Delta_dt)/Delta/Delta + k.b[j]*(k.b[j] - 1)*(k.b[j] - 2)*Delta_d*Delta_d*Delta_t/pow(Delta, 3)) * DeltaPow;

		const auto D     = k.n[i]*DeltaPow*delta*psi;
		const auto D_d   = k.n[i]*(DeltaPow*(psi + delta*psi_d) + DeltaPow_d*delta*psi);
		const auto D_t   = k.n[i]*delta*(DeltaPow_t*psi + DeltaPow*psi_t);
		const auto D_dd  = k.n[i]*(DeltaPow*(2*psi_d + delta*psi_dd) + 2*DeltaPow_d*(psi + delta*psi_d) + DeltaPow_dd*delta*psi);
		const auto D_tt  = k.n[i]*delta*(DeltaPow_tt*psi + 2*DeltaPow_t*psi_t + DeltaPow*psi_tt);
		const auto D_dt  = k.n[i]*(DeltaPow*(psi_t + delta*psi_dt) + delta*DeltaPow_d*psi_t + DeltaPow_t*(psi + delta*psi_d) + DeltaPow_dt*delta*psi);
		const auto D_ddd = k.n[i]*(DeltaPow_ddd*delta*psi + 3*DeltaPow_dd*(psi + delta*psi_d) + 3*DeltaPow_d*(2*psi_d + delta*psi_dd) + DeltaPow*(3*psi_dd + delta*psi_ddd));
		const auto D_ttt = k.n[i]*delta*(DeltaPow_ttt*psi + 3*DeltaPow_tt*psi_t + 3*DeltaPow_t*psi_tt + DeltaPow*psi_ttt);
		const auto D_dtt = k.n[i]*(DeltaPow_tt*psi + 2*DeltaPow_t*psi_t + DeltaPow*psi_tt) + k.n[i]*delta*(DeltaPow_dtt*psi + DeltaPow_tt*psi_d + 2*DeltaPow_dt*psi_t + 2*DeltaPow_t*psi_dt + DeltaPow_d*psi_tt + DeltaPow*psi_dtt);
		const auto D_ddt = k.n[i]*(DeltaPow_ddt*delta*psi + 2*DeltaPow_dt*(psi + delta*psi_d) + DeltaPow_dd*delta*psi_t + DeltaPow_t*(2*psi_d + delta*psi_dd) + 2*DeltaPow_d*(psi_t + delta*psi_dt) + DeltaPow*(2*psi_dt + delta*psi_ddt));

		phir     += D;
		phir_d   += D_d;
		phir_t   += D_t;
		phir_dd  += D_dd;
		phir_tt  += D_tt;
		phir_dt  += D_dt;
		phir_ddd += D_ddd;
		phir_ttt += D_ttt;
		phir_dtt += D_dtt;
		phir_ddt += D_ddt;
	}

	const auto phi     = phio     + phir    ;
	const auto phi_d   = phio_d   + phir_d  ;
	const auto phi_t   = phio_t   + phir_t  ;
	const auto phi_dd  = phio_dd  + phir_dd ;
	const auto phi_tt  = phio_tt  + phir_tt ;
	const auto phi_dt  = phio_dt  + phir_dt ;
	const auto phi_ddd = phio_ddd + phir_ddd;
	const auto phi_ttt = phio_ttt + phir_ttt;
	const auto phi_dtt = phio_dtt + phir_dtt;
	const auto phi_ddt = phio_ddt + phir_ddt;

	const auto Tcr = k.saturation.Tcr;
	const auto Dcr = k.saturation.Dcr;

	const auto tT   = -Tcr/(T*T);
	const auto tTT  =  2*Tcr/(T*T*T);
	const auto tTTT = -6*Tcr/(T*T*T*T);
	const auto dD   =  1/Dcr;

	const auto phiT   = phi_t*tT;
	const auto phiD   = phi_d*dD;
	const auto phiTT  = phi_tt*tT*tT + phi_t*tTT;
	const auto phiTD  = phi_dt*tT*dD;
	const auto phiDD  = phi_dd*dD*dD;
	const auto phiTTT = phi_ttt*tT*tT*tau*T + 3*phi_tt*tT*tTT + phi_t*tTTT;
	const auto phiTTD = phi_dtt*tT*tT*dD + phi_dt*tTT*dD;
	const auto phiTDD = phi_ddt*tT*dD*dD;
	const auto phiDDD = phi_ddd*dD*dD*dD;

	// The specific gas constant in units of J/(kg*K)
	const auto R = 461.51805;

	WaterHelmholtzState hs;

	hs.helmholtz    = R*T*phi;
	hs.helmholtzT   = R*T*phiT + R*phi;
	hs.helmholtzD   = R*T*phiD;
	hs.helmholtzTT  = R*T*phiTT + 2*R*phiT;
	hs.helmholtzTD  = R*T*phiTD + R*phiD;
	hs.helmholtzDD  = R*T*phiDD;
	hs.helmholtzTTT = R*T*phiTTT + 3*R*phiTT;
	hs.helmholtzTTD = R*T*phiTTD + 2*R*phiTD;
	hs.helmholtzTDD = R*T*phiTDD + R*phiDD;
	hs.helmholtzDDD = R*T*phiDDD;

	return hs;
}

} // namespace Reaktoro
