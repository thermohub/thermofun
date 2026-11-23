# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The dielectric constant of Bradley and Pitzer (1979), against the tables of Tanger and Helgeson (1988), which used it at low T and high P.

import pytest
import thermofun as tf

METHOD = tf.methods.dielectricMethod("BradleyPitzer79")
WATER = tf.PropertiesSolvent()


def at(T, P):
    return METHOD.properties(T, P, WATER, 0)


def test_the_value_at_25_degrees():
    assert at(298.15, 1e5).epsilon.val == pytest.approx(78.38, abs=0.01)


@pytest.mark.parametrize("T_C,P_kb,table", [(0, 5, 4.65), (25, 1.5, 5.40), (25, 3, 5.14), (25, 5, 4.91), (50, 5, 5.26)])
def test_y_of_table_h4(T_C, P_kb, table):
    assert -at(T_C + 273.15, P_kb * 1e8).bornY.val * 1e5 == pytest.approx(table, abs=0.01)


@pytest.mark.parametrize("T_C,P_kb,table", [(0, 5, 0.81), (25, 3, 1.62), (25, 5, 1.25)])
def test_x_of_table_h6(T_C, P_kb, table):
    assert -at(T_C + 273.15, P_kb * 1e8).bornX.val * 1e7 == pytest.approx(table, abs=0.01)


def test_the_formula_of_the_paper():
    T, Pbar = 473.15, 1500.0
    U = (3.4279e2, -5.0866e-3, 9.4690e-7, -2.0525, 3.1159e3, -1.8289e2, -8.0325e3, 4.2142e6, 2.1417)
    import math
    B = U[6] + U[7] / T + U[8] * T
    D = U[0] * math.exp(U[1] * T + U[2] * T * T) + (U[3] + U[4] / (U[5] + T)) * math.log((B + Pbar) / (B + 1000.0))
    assert at(T, Pbar * 1e5).epsilon.val == pytest.approx(D, rel=1e-13)


@pytest.mark.parametrize("T,P", [(298.15, 1e5), (473.15, 1e8), (343.15, 4e8)])
def test_all_derivatives_agree_with_finite_differences(T, P):
    x = at(T, P)
    hT, hP = 1e-2, 1e3
    assert x.epsilonT.val == pytest.approx((at(T + hT, P).epsilon.val - at(T - hT, P).epsilon.val) / (2 * hT), rel=1e-6)
    assert x.epsilonP.val == pytest.approx((at(T, P + hP).epsilon.val - at(T, P - hP).epsilon.val) / (2 * hP) * 1e5, rel=1e-6)
    assert x.epsilonTT.val == pytest.approx((at(T + hT, P).epsilonT.val - at(T - hT, P).epsilonT.val) / (2 * hT), rel=1e-6)
    assert x.epsilonTP.val == pytest.approx((at(T, P + hP).epsilonT.val - at(T, P - hP).epsilonT.val) / (2 * hP) * 1e5, rel=1e-6)
    assert x.epsilonPP.val == pytest.approx((at(T, P + hP).epsilonP.val - at(T, P - hP).epsilonP.val) / (2 * hP) * 1e5, rel=1e-5)
    assert x.epsilon.ddt == pytest.approx(x.epsilonT.val, rel=1e-10)
    assert x.bornQ.val == pytest.approx(x.epsilonP.val / x.epsilon.val ** 2 / 1e5, rel=1e-12)
