# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The method objects run alone from Python, with no database and no engine."""
import glob
import json
import math
import os

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


def test_engine_runs_records_on_method_objects():
    # the substances with only methods that have code run on the new path, the others on the old one
    here = os.path.dirname(os.path.abspath(__file__))
    with_code = {"cp_ft_equation", "standard_entropy_cp_integration", "mv_constant", "mv_pvnrt", "solute_hkf88_gems", "solute_hkf88_reaktoro", "solute_aknifiev_diamond03", "landau_holland_powell98", "mv_eos_murnaghan_hp98", "mv_equation_berman88", "mv_equation_dorogokupets88", "mv_eos_birch_murnaghan_gott97"}
    new_path = 0
    for path in glob.glob(os.path.join(here, "..", "Resources", "databases", "*.json")):
        engine = tf.ThermoEngine(path)
        with open(path) as f:
            data = json.load(f)
        for record in data["substances"]:
            names = {list(m["method"].values())[0].strip() for m in record.get("TPMethods") or []}
            base = {"cp_ft_equation", "standard_entropy_cp_integration", "solute_hkf88_gems", "solute_hkf88_reaktoro"} & names
            try:
                steps = engine.explain(298.15, 1e5, record["symbol"])
            except RuntimeError:  # the record has invalid parameters: the old engine raises the same error
                with pytest.raises(RuntimeError):
                    engine.thermoPropertiesSubstance(298.15, 1e5, record["symbol"])
                continue
            if names <= with_code and len(base) == 1:
                assert steps, record["symbol"]
                new_path += 1
            elif not names <= with_code:
                assert steps == [], record["symbol"]
    assert new_path > 2000


def test_constant_volume_adds_the_error_of_the_volume():
    here = os.path.dirname(os.path.abspath(__file__))
    path = os.path.join(here, "mines16-thermofun.json")
    with open(path) as f:
        record = next(r for r in json.load(f)["substances"] if r["symbol"] == "Fluorite")
    err_g = record["sm_gibbs_energy"]["errors"][0]
    err_v = record["sm_volume"]["errors"][0]  # J/bar
    engine = tf.ThermoEngine(path)
    assert engine.explain(298.15, 1e5, "Fluorite")
    tps = engine.thermoPropertiesSubstance(298.15, 1e5, "Fluorite")
    assert tps.gibbs_energy.err == pytest.approx(math.sqrt(err_g**2 + err_v**2), rel=1e-14)
    assert tps.gibbs_energy.err != err_g


def test_explain_gives_the_state_after_each_method():
    here = os.path.dirname(os.path.abspath(__file__))
    engine = tf.ThermoEngine(os.path.join(here, "test-thermoengine-thermofun.json"))
    steps = engine.explain(400.0, 1e7, "Quartz")
    if steps:
        assert steps[0]["method"] == "CpPolynomial"
        assert steps[-1]["G"] == pytest.approx(engine.thermoPropertiesSubstance(400.0, 1e7, "Quartz").gibbs_energy.val, rel=1e-12)
