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

#include "WaterHelmholtzStateHGK.hpp"
#include "WaterHelmholtzState.hpp"

namespace ThermoFun {
    namespace
    {
        void setzeros(ThermoFun::WaterHelmholtzState &s)
        {
            s.helmholtz = 0.0;
            s.helmholtzT = 0.0;
            s.helmholtzTT = 0.0;
            s.helmholtzTTT = 0.0;
            s.helmholtzD = 0.0;
            s.helmholtzTD = 0.0;
            s.helmholtzDD = 0.0;
            s.helmholtzTTD = 0.0;
            s.helmholtzTDD = 0.0;
            s.helmholtzDDD = 0.0;
        }

        auto calculateWaterHelmholtzStateHGK0(real t, real /*d*/, const HGKConstants& k) -> WaterHelmholtzState
        {
            WaterHelmholtzState s;
            setzeros(s);

            const auto ln_t = log(t);

            s.helmholtz = (k.A0[0] + k.A0[1] * t) * ln_t;
            s.helmholtzT = k.A0[0] / t + k.A0[1] * (ln_t + 1);
            s.helmholtzTT = -k.A0[0] / (t * t) + k.A0[1] / t;
            s.helmholtzTTT = 2 * k.A0[0] / (t * t * t) - k.A0[1] / (t * t);

            for (int i = 2; i <= 17; ++i)
            {
                const auto aux = k.A0[i] * pow(t, i - 4);

                s.helmholtz += aux;
                s.helmholtzT += aux * (i - 4) / t;
                s.helmholtzTT += aux * (i - 4) * (i - 5) / (t * t);
                s.helmholtzTTT += aux * (i - 4) * (i - 5) * (i - 6) / (t * t * t);
            }

            return s;
        }

        auto calculateWaterHelmholtzStateHGK1(real t, real d, const HGKConstants& k) -> WaterHelmholtzState
        {
            WaterHelmholtzState s;
			setzeros(s);

            for (int i = 0; i <= 4; ++i)
            {
                const auto aux = d * k.A1[i] * pow(t, 1 - i);

                s.helmholtz += aux;
                s.helmholtzT -= aux * (i - 1) / t;
                s.helmholtzTT += aux * (i - 1) * i / (t * t);
                s.helmholtzTTT -= aux * (i - 1) * i * (i + 1) / (t * t * t);
            }

            s.helmholtzD = s.helmholtz / d;
            s.helmholtzTD = s.helmholtzT / d;
            s.helmholtzTTD = s.helmholtzTT / d;

            return s;
        }

        auto calculateWaterHelmholtzStateHGK2(real t, real d, const HGKConstants& k) -> WaterHelmholtzState
        {
            WaterHelmholtzState s;
			setzeros(s);

            const auto t3 = pow(t, -3);
            const auto t5 = pow(t, -5);
            const auto ln_t = log(t);

            const auto y = d * (k.yc[0] + k.yc[1] * ln_t + k.yc[2] * t3 + k.yc[3] * t5);
            const auto y_r = y / d;
            const auto y_t = d * (k.yc[1] - 3.0 * k.yc[2] * t3 - 5.0 * k.yc[3] * t5) / t;
            const auto y_rr = 0.0;
            const auto y_tt = d * (-k.yc[1] + 12.0 * k.yc[2] * t3 + 30.0 * k.yc[3] * t5) / (t * t);
            const auto y_rt = y_t / d;
            const auto y_rrr = 0.0;
            const auto y_rrt = y_rt / d - y_t / (d * d);
            const auto y_rtt = y_tt / d;
            const auto y_ttt = d * (2 * k.yc[1] - 60 * k.yc[2] * t3 - 210 * k.yc[3] * t5) / (t * t * t);

            const auto x = 1.0 / (1.0 - y);
            const auto x2 = x * x;
            const auto x_r = y_r * x2;
            const auto x_t = y_t * x2;
            const auto x_rr = y_rr * x2 + 2.0 * y_r * x_r * x;
            const auto x_tt = y_tt * x2 + 2.0 * y_t * x_t * x;
            const auto x_rt = y_rt * x2 + 2.0 * y_r * x_t * x;
            const auto x_rrr = y_rrr * x2 + 4 * y_rr * x_r * x + 2 * y_r * (x_rr * x + x_r * x_r);
            const auto x_rrt = y_rrt * x2 + 2 * x * (y_rt * x_r + y_rr * x_t) + 2 * y_r * (x_rt * x + x_r * x_t);
            const auto x_rtt = y_rtt * x2 + 4 * y_rt * x_t * x + 2 * y_r * (x_tt * x + x_t * x_t);
            const auto x_ttt = y_ttt * x2 + 4 * y_tt * x_t * x + 2 * y_t * (x_tt * x + x_t * x_t);

            const auto u = log(d * x);
            const auto u_r = x_r / x + 1.0 / d;
            const auto u_t = x_t / x;
            const auto u_rr = x_rr / x - x_r * x_r / (x * x) - 1.0 / (d * d);
            const auto u_rt = x_rt / x - x_r * x_t / (x * x);
            const auto u_tt = x_tt / x - x_t * x_t / (x * x);
            const auto u_rrr = x_rrr / x - 3 * x_rr * x_r / (x * x) + 2 * x_r * x_r * x_r / (x * x * x) + 2 / (d * d * d);
            const auto u_rrt = x_rrt / x - (2 * x_rt * x_r + x_rr * x_t) / (x * x) + 2 * x_r * x_r * x_t / (x * x * x);
            const auto u_rtt = x_rtt / x - (2 * x_rt * x_t + x_tt * x_r) / (x * x) + 2 * x_t * x_t * x_r / (x * x * x);
            const auto u_ttt = x_ttt / x - 3 * x_tt * x_t / (x * x) + 2 * x_t * x_t * x_t / (x * x * x);

            const auto c1 = -130.0 / 3.0;
            const auto c2 = 169.0 / 6.0;
            const auto c3 = -14.0;

            s.helmholtz = k.A20 * t * (u + c1 * x + c2 * x * x + c3 * y);
            s.helmholtzD = k.A20 * t * (u_r + c1 * x_r + 2 * c2 * x * x_r + c3 * y_r);
            s.helmholtzT = k.A20 * t * (u_t + c1 * x_t + 2 * c2 * x * x_t + c3 * y_t) + s.helmholtz / t;
            s.helmholtzDD = k.A20 * t * (u_rr + c1 * x_rr + 2 * c2 * (x * x_rr + x_r * x_r) + c3 * y_rr);
            s.helmholtzTD = k.A20 * t * (u_rt + c1 * x_rt + 2 * c2 * (x * x_rt + x_r * x_t) + c3 * y_rt) + s.helmholtzD / t;
            s.helmholtzTT = k.A20 * t * (u_tt + c1 * x_tt + 2 * c2 * (x * x_tt + x_t * x_t) + c3 * y_tt) + 2 * (s.helmholtzT / t - s.helmholtz / (t * t));
            s.helmholtzDDD = k.A20 * t * (u_rrr + c1 * x_rrr + 2 * c2 * (3 * x_r * x_rr + x * x_rrr) + c3 * y_rrr);
            s.helmholtzTDD = k.A20 * t * (u_rrt + c1 * x_rrt + 2 * c2 * (x_t * x_rr + 2 * x_r * x_rt + x * x_rrt) + c3 * y_rrt) + s.helmholtzDD / t;
            s.helmholtzTTD = k.A20 * t * (u_rtt + c1 * x_rtt + 2 * c2 * (x_r * x_tt + 2 * x_t * x_rt + x * x_rtt) + c3 * y_rtt) + 2 * (s.helmholtzTD - s.helmholtzD / t) / t;
            s.helmholtzTTT = k.A20 * t * (u_ttt + c1 * x_ttt + 2 * c2 * (3 * x_t * x_tt + x * x_ttt) + c3 * y_ttt) + 3 * (s.helmholtzTT - 2 * s.helmholtzT / t + s.helmholtz / (t * t)) / t;

            return s;
        }

        auto calculateWaterHelmholtzStateHGK3(real t, real d, const HGKConstants& k) -> WaterHelmholtzState
        {
            WaterHelmholtzState s;
			setzeros(s);

            const auto z = 1 - exp(-k.z0 * d);
            const auto z_r = k.z0 * (1 - z);
            const auto z_rr = -k.z0 * z_r;
            const auto z_rrr = -k.z0 * z_rr;

            for (int i = 0; i <= 35; ++i)
            {
                const auto lambda = k.A3[i] * pow(t, -k.li[i]) * pow(z, k.ki[i]);
                const auto lambda_r = k.ki[i] * z_r * lambda / z;
                const auto lambda_t = -k.li[i] * lambda / t;
                const auto lambda_rr = lambda_r * (z_rr / z_r + lambda_r / lambda - z_r / z);
                const auto lambda_rt = lambda_r * lambda_t / lambda;
                const auto lambda_tt = lambda_t * (lambda_t / lambda - 1.0 / t);
                const auto lambda_rrr = lambda_rr * (z_rr / z_r + lambda_r / lambda - z_r / z) + lambda_r * (z_rrr / z_r - pow(z_rr / z_r, 2) + lambda_rr / lambda - pow(lambda_r / lambda, 2) - z_rr / z + pow(z_r / z, 2));
                auto l_pow = pow(lambda_t / lambda, 2);
                if (l_pow == 0)
                {
                    l_pow[1] = 0; // derivative of the pass
                }
                const auto lambda_rrt = -pow(lambda_r / lambda, 2) * lambda_t + (lambda_rr * lambda_t + lambda_rt * lambda_r) / lambda;
                const auto lambda_rtt = -l_pow * lambda_r + (lambda_tt * lambda_r + lambda_rt * lambda_t) / lambda;
                const auto lambda_ttt = lambda_tt * (lambda_t / lambda - 1.0 / t) + lambda_t * (lambda_tt / lambda - l_pow + 1.0 / (t * t));

                s.helmholtz += lambda;
                s.helmholtzD += lambda_r;
                s.helmholtzT += lambda_t;
                s.helmholtzDD += lambda_rr;
                s.helmholtzTD += lambda_rt;
                s.helmholtzTT += lambda_tt;
                s.helmholtzDDD += lambda_rrr;
                s.helmholtzTDD += lambda_rrt;
                s.helmholtzTTD += lambda_rtt;
                s.helmholtzTTT += lambda_ttt;
            }

            return s;
        }

        auto calculateWaterHelmholtzStateHGK4(real t, real d, const HGKConstants& k) -> WaterHelmholtzState
        {
            WaterHelmholtzState s;
			setzeros(s);

            for (int i = 0; i <= 3; ++i)
            {
                const auto delta = (d - k.ri[i]) / k.ri[i];
                const auto tau = (t - k.ti[i]) / k.ti[i];
                const auto delta_r = 1.0 / k.ri[i];
                const auto tau_t = 1.0 / k.ti[i];

                const auto delta_m = pow(delta, k.mi[i]);
                const auto delta_n = pow(delta, k.ni[i]);

                const auto psi = (k.ni[i] - k.alpha[i] * k.mi[i] * delta_m) * delta_r / delta;
                const auto psi_r = -(k.ni[i] + k.alpha[i] * k.mi[i] * (k.mi[i] - 1) * delta_m) * pow(delta_r / delta, 2);
                const auto psi_rr = (2 * k.ni[i] - k.alpha[i] * k.mi[i] * (k.mi[i] - 1) * (k.mi[i] - 2) * delta_m) * pow(delta_r / delta, 3);

                const auto theta = k.A4[i] * delta_n * exp(-k.alpha[i] * delta_m - k.beta[i] * tau * tau);
                const auto theta_r = psi * theta;
                const auto theta_t = -2 * k.beta[i] * tau * tau_t * theta;
                const auto theta_rr = psi_r * theta + psi * theta_r;
                const auto theta_tt = 2 * k.beta[i] * (2 * k.beta[i] * tau * tau - 1) * tau_t * tau_t * theta;
                const auto theta_rt = -2 * k.beta[i] * tau * tau_t * theta_r;
                const auto theta_rrr = psi_rr * theta + 2 * psi_r * theta_r + psi * theta_rr;
                const auto theta_rrt = psi_r * theta_t + psi * theta_rt;
                const auto theta_rtt = psi * theta_tt;
                const auto theta_ttt = -2 * k.beta[i] * (2 * tau_t * tau_t * theta_t + tau * tau_t * theta_tt);

                s.helmholtz += theta;
                s.helmholtzD += theta_r;
                s.helmholtzT += theta_t;
                s.helmholtzDD += theta_rr;
                s.helmholtzTD += theta_rt;
                s.helmholtzTT += theta_tt;
                s.helmholtzDDD += theta_rrr;
                s.helmholtzTDD += theta_rrt;
                s.helmholtzTTD += theta_rtt;
                s.helmholtzTTT += theta_ttt;
            }

            return s;
        }

    } // namespace

auto waterHelmholtzStateHGK(real T, real D, const HGKConstants& k) -> WaterHelmholtzState
{
	// The dimensionless temperature and density
	const auto t = T/k.referenceTemperature;
	const auto r = D/k.referenceDensity;

	// Compute the contributions from each auxiliary Helmholtz state
	WaterHelmholtzState aux0, aux1, aux2, aux3, aux4, res;
	aux0 = calculateWaterHelmholtzStateHGK0(t, r, k);
	aux1 = calculateWaterHelmholtzStateHGK1(t, r, k);
	aux2 = calculateWaterHelmholtzStateHGK2(t, r, k);
	aux3 = calculateWaterHelmholtzStateHGK3(t, r, k);
	aux4 = calculateWaterHelmholtzStateHGK4(t, r, k);

	// Assemble the contributions from each auxiliary Helmholtz state
	res.helmholtz    = aux0.helmholtz    + aux1.helmholtz    + aux2.helmholtz    + aux3.helmholtz    + aux4.helmholtz;
	res.helmholtzD   = aux0.helmholtzD   + aux1.helmholtzD   + aux2.helmholtzD   + aux3.helmholtzD   + aux4.helmholtzD;
	res.helmholtzT   = aux0.helmholtzT   + aux1.helmholtzT   + aux2.helmholtzT   + aux3.helmholtzT   + aux4.helmholtzT;
	res.helmholtzDD  = aux0.helmholtzDD  + aux1.helmholtzDD  + aux2.helmholtzDD  + aux3.helmholtzDD  + aux4.helmholtzDD;
	res.helmholtzTD  = aux0.helmholtzTD  + aux1.helmholtzTD  + aux2.helmholtzTD  + aux3.helmholtzTD  + aux4.helmholtzTD;
	res.helmholtzTT  = aux0.helmholtzTT  + aux1.helmholtzTT  + aux2.helmholtzTT  + aux3.helmholtzTT  + aux4.helmholtzTT;
	res.helmholtzDDD = aux0.helmholtzDDD + aux1.helmholtzDDD + aux2.helmholtzDDD + aux3.helmholtzDDD + aux4.helmholtzDDD;
	res.helmholtzTDD = aux0.helmholtzTDD + aux1.helmholtzTDD + aux2.helmholtzTDD + aux3.helmholtzTDD + aux4.helmholtzTDD;
	res.helmholtzTTD = aux0.helmholtzTTD + aux1.helmholtzTTD + aux2.helmholtzTTD + aux3.helmholtzTTD + aux4.helmholtzTTD;
	res.helmholtzTTT = aux0.helmholtzTTT + aux1.helmholtzTTT + aux2.helmholtzTTT + aux3.helmholtzTTT + aux4.helmholtzTTT;

	// Convert the Helmholtz free energy of water and its derivatives to dimensioned form
	res.helmholtz    *= k.referenceHelmholtz;
	res.helmholtzD   *= k.referenceHelmholtz/k.referenceDensity;
	res.helmholtzT   *= k.referenceHelmholtz/k.referenceTemperature;
	res.helmholtzDD  *= k.referenceHelmholtz/k.referenceDensity/k.referenceDensity;
	res.helmholtzTD  *= k.referenceHelmholtz/k.referenceDensity/k.referenceTemperature;
	res.helmholtzTT  *= k.referenceHelmholtz/k.referenceTemperature/k.referenceTemperature;
	res.helmholtzDDD *= k.referenceHelmholtz/k.referenceDensity/k.referenceDensity/k.referenceDensity;
	res.helmholtzTDD *= k.referenceHelmholtz/k.referenceDensity/k.referenceDensity/k.referenceTemperature;
	res.helmholtzTTD *= k.referenceHelmholtz/k.referenceDensity/k.referenceTemperature/k.referenceTemperature;
	res.helmholtzTTT *= k.referenceHelmholtz/k.referenceTemperature/k.referenceTemperature/k.referenceTemperature;

	return res;
}

} // namespace Reaktoro
