# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The mineral and table methods alone, with no database and no engine, against the old classes and the published
# equations. A correction applied alone, from an empty state, gives its own increment.

import json
import pathlib

import pytest
import thermofun as tf

HERE = pathlib.Path(__file__).resolve().parent
SYNTHETIC = HERE / "data" / "legacy-synthetic"
AQ17 = HERE.parent / "Resources" / "databases" / "mines16-thermofun.json"
POINTS = [(298.15, 1e5), (500.0, 1e5), (800.0, 1e8), (1000.0, 5e8)]


def method_entry(record, name):
    return next(m for m in record["TPMethods"] if name in m["method"].values())


def reference_of(record):
    value = lambda k: record[k]["values"][0]
    return tf.methods.Reference(T=record["Tst"], P=record["Pst"], G=value("sm_gibbs_energy"), H=value("sm_enthalpy"), S=value("sm_entropy_abs"),
                                Cp=value("sm_heat_capacity_p"), V=value("sm_volume") * 1e-5)


def fixture(name):
    return json.loads((SYNTHETIC / (name + ".json")).read_text())


def check_against_old(method, old, record, T, P):
    # Birch-Murnaghan uses finite differences inside: one ulp of a parameter through SI changes G by about 2e-9
    now = method.evaluate(T, P, reference_of(record))
    before = old.thermoProperties(T, P, tf.ThermoPropertiesSubstance())
    assert now.G.val == pytest.approx(before.gibbs_energy.val, rel=1e-8, abs=1e-6)
    assert now.H.val == pytest.approx(before.enthalpy.val, rel=1e-8, abs=1e-6)
    assert now.S.val == pytest.approx(before.entropy.val, rel=1e-8, abs=1e-8)
    assert now.V.val * 1e5 == pytest.approx(before.volume.val, rel=1e-8, abs=1e-10)
    assert now.G.ddt == pytest.approx(before.gibbs_energy.ddt, rel=1e-6, abs=1e-8)


def parameters_si(entry, key, names_units):
    values = entry[key]["values"]
    return {n: values[i] * f for i, n, f in names_units}


VOLUME = [(0, "a1", 1.0), (1, "a2", 1.0), (2, "a3", 1.0), (3, "b1", 1e-5), (4, "b2", 1e-10)]
BIRCH = [(0, "a1", 1.0), (1, "a2", 1.0), (2, "a3", 1.0), (5, "kt00", 1e6), (6, "dkdt", 1e6), (7, "kp", 1.0), (8, "kpp", 1e-6)]
CASES = [
    ("s36-mv_equation_berman88", "mv_equation_berman88", "VolumeBerman88", "m_volume_fpt_coeffs", VOLUME, "MinBerman88"),
    ("s35-mv_equation_dorogokupets88", "mv_equation_dorogokupets88", "VolumeDorogokupets88", "m_volume_fpt_coeffs", VOLUME, "MinBerman88"),
    ("s37-mv_eos_birch_murnaghan_gott97", "mv_eos_birch_murnaghan_gott97", "BirchMurnaghan47", "eos_birch_murnaghan_coeffs", BIRCH, "MinBMGottschalk"),
]


def case_method(case):
    name, legacy, method_id, key, names, old = case
    doc = fixture(name)
    record = next(r for r in doc["substances"] if any(legacy in m["method"].values() for m in r["TPMethods"]))
    return tf.methods.substanceMethod(method_id, parameters=parameters_si(method_entry(record, legacy), key, names), symbol=record["symbol"]), record, old


@pytest.mark.parametrize("case", CASES, ids=lambda c: c[2])
@pytest.mark.parametrize("T,P", POINTS)
def test_a_correction_alone_gives_the_result_of_the_old_class(case, T, P):
    method, record, old = case_method(case)
    substance = tf.Database(json.dumps({"elements": [], "substances": [record], "reactions": []})).getSubstance(record["symbol"])
    check_against_old(method, getattr(tf, old)(substance), record, T, P)


@pytest.mark.parametrize("case", CASES, ids=lambda c: c[2])
def test_the_derivative_by_each_parameter_agrees_with_finite_differences(case):
    method, record, _ = case_method(case)
    T, P, reference = 800.0, 1e8, reference_of(record)
    values = method.parameterValues()
    for i, value in enumerate(values):
        if value == 0.0:
            continue
        step = abs(value) * 1e-5
        high = method.withParameter(i, value + step).evaluate(T, P, reference).G.val
        low = method.withParameter(i, value - step).evaluate(T, P, reference).G.val
        assert method.parameterDerivative(i, T, P, reference)["G"] == pytest.approx((high - low) / (2 * step), rel=1e-4, abs=1e-3), i


# The Landau term of Holland and Powell (2011) against an independent form of the equations (that of BurnMan, with the
# correction of the "landaunote" of Holland). Quartz of the data set ds62.
TR, PR = 298.15, 1e5
TC0, SMAX, VMAX = 847.0, 4.95, 0.1188e-5
LANDAU11 = dict(Tc0=TC0, Smax=SMAX, Vmax=VMAX)
EMPTY = dict(T=TR, P=PR)


def landau11(T, P):
    q0 = ((TC0 - TR) / TC0) ** 0.25 if TR < TC0 else 0.0
    Tc = TC0 + VMAX * (P - PR) / SMAX
    q = ((Tc - T) / TC0) ** 0.25 if T < Tc else 0.0
    G = (TC0 * SMAX * (q0**2 - q0**6 / 3.0) - SMAX * (Tc * q**2 - TC0 * q**6 / 3.0)
         - T * SMAX * (q0**2 - q**2) + (P - PR) * VMAX * q0**2)
    S = SMAX * (q0**2 - q**2)
    V = VMAX * (q0**2 - q**2)
    Cp = T * SMAX / (2.0 * TC0 * q**2) if q > 0 else 0.0
    return G, S, V, Cp


def landau11_method():
    return tf.methods.substanceMethod("LandauHP11", parameters=LANDAU11, symbol="Q")


@pytest.mark.parametrize("T,P", [(298.15, 1e5), (500.0, 1e5), (800.0, 1e5), (846.0, 1e5), (900.0, 1e5), (700.0, 5e8), (1100.0, 1e9), (400.0, 2e9)])
def test_the_landau_term_of_2011_agrees_with_the_independent_equations(T, P):
    r = landau11_method().evaluate(T, P, tf.methods.Reference(**EMPTY))
    G, S, V, Cp = landau11(T, P)
    assert r.G.val == pytest.approx(G, rel=1e-9, abs=1e-6)
    assert r.S.val == pytest.approx(S, rel=1e-9, abs=1e-9)
    assert r.V.val == pytest.approx(V, rel=1e-9, abs=1e-14)
    assert r.Cp.val == pytest.approx(Cp, rel=1e-9, abs=1e-9)


def test_the_landau_term_of_2011_is_zero_at_the_reference_state():
    r = landau11_method().evaluate(TR, PR, tf.methods.Reference(**EMPTY))
    assert r.G.val == pytest.approx(0.0, abs=1e-9)
    assert r.S.val == pytest.approx(0.0, abs=1e-12)


def test_the_landau_term_of_2011_is_consistent():
    T, P = 700.0, 3e8
    r = landau11_method().evaluate(T, P, tf.methods.Reference(**EMPTY))
    assert r.G.ddt == pytest.approx(-r.S.val, rel=1e-9)
    assert r.S.ddt == pytest.approx(r.Cp.val / T, rel=1e-8)
    assert r.H.val == pytest.approx(r.G.val + T * r.S.val, rel=1e-12)
    assert r.G.ddp == pytest.approx(r.V.val, rel=1e-8)


UNIQUAC = dict(a=-200.0, b=0.5, c=30000.0, alpha=1e-5 * 1e-5, beta=-1e-9 * 1e-10, Theta=200.0)


def test_extended_uniquac_is_consistent():
    method = tf.methods.substanceMethod("CpExtendedUNIQUAC", parameters=UNIQUAC, symbol="X")
    reference = tf.methods.Reference(T=298.15, P=1e5, G=-1e5, H=-1.2e5, S=50.0, Cp=100.0, V=1e-5)
    T, P = 400.0, 1e7
    r = method.evaluate(T, P, reference)
    assert r.G.ddt == pytest.approx(-r.S.val, rel=1e-6)
    assert r.H.ddt == pytest.approx(r.Cp.val, rel=1e-6)
