# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The method objects run alone from Python, with no database and no engine."""
import math

import pytest
import thermofun as tf

needs_autodiff = pytest.mark.skipif(not getattr(tf, "with_autodiff", True), reason="built with -DTFUN_USE_AUTODIFF=OFF: the derivatives are 0")

REF = tf.methods.Reference(T=298.15, P=1e5, G=-1129178.0, H=-1207600.0, S=91.7, Cp=83.47)


def test_constant_properties():
    r = tf.methods.ConstantProperties().evaluate(500.0, 1e7, REF)
    assert r.G.val == -1129178.0 and r.Cp.val == 83.47 and r.V.val == 0.0


@needs_autodiff
def test_constant_cp():
    m = tf.methods.ConstantCp()
    assert m.id == "ConstantCp"
    r = m.evaluate(500.0, 1e5, REF)
    assert r.S.val == pytest.approx(91.7 + 83.47 * math.log(500.0 / 298.15), rel=1e-14)
    assert r.H.ddt == pytest.approx(83.47, rel=1e-12)


@needs_autodiff
def test_cp_polynomial_and_parameters():
    a = [99.7, 0.027, -2148800.0, 0, 0, 0, 0, 0, 0, 0]
    m = tf.methods.CpPolynomial([(273.15, 2000.0)], a)
    assert m.parameterValues() == a
    r = m.evaluate(600.0, 1e5, REF)
    assert r.H.ddt == pytest.approx(r.Cp.val, rel=1e-10)
    assert r.Cp.val == pytest.approx(99.7 + 0.027 * 600 - 2148800.0 / 600**2, rel=1e-14)
    assert not r.outside_range and m.evaluate(3000.0, 1e5, REF).outside_range

    # a parameter derivative against finite differences; the original does not change
    d = m.parameterDerivative(1, 600.0, 1e5, REF)
    up = m.withParameter(1, a[1] + 1e-3).evaluate(600.0, 1e5, REF)
    assert d["G"] == pytest.approx((up.G.val - r.G.val) / 1e-3, rel=1e-6)
    assert m.parameterValues() == a


def test_cp_polynomial_with_a_transition():
    a = [100.0] + [0.0] * 9
    m = tf.methods.CpPolynomial([(273.15, 800.0), (800.0, 2000.0)], a + a, [tf.methods.PhaseTransition(800.0, dS=2.5, dH=2000.0, dV=1e-6)])
    assert m.evaluate(900.0, 1e5, REF).V.val == pytest.approx(1e-6)
    assert m.evaluate(900.0, 1e5, REF).H.val - m.evaluate(700.0, 1e5, REF).H.val == pytest.approx(100.0 * 200 + 2000.0)
