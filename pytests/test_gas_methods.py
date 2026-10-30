# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The fluid methods alone, with no database and no engine, against the old classes.

import json
import math
import pathlib

import pytest
import thermofun as tf

HERE = pathlib.Path(__file__).resolve().parent
SYNTHETIC = HERE / "data" / "legacy-synthetic"
CO = [132.91000366211, 34.959999084473e5, 0.048300001770258]  # Tc K, Pc Pa, omega

CASES = [
    ("s13-fluid_peng_robinson78", "CO-pr78", "PengRobinson78", "GasPR78", dict(Tc=CO[0], Pc=CO[1], omega=CO[2], N=0.042789999395609)),
    ("s11-fluid_soave_redlich_kwong", "CO-srk", "SoaveRedlichKwong72", "GasSRK", dict(Tc=CO[0], Pc=CO[1], omega=CO[2], N=0.042789999395609)),
    ("s12-fluid_sterner_pitzer", "CO-stp", "SternerPitzer94", "GasSTP", dict(Tc=CO[0], Pc=CO[1])),
]


def load(name, symbol):
    doc = json.loads((SYNTHETIC / (name + ".json")).read_text())
    record = next(r for r in doc["substances"] if r["symbol"] == symbol)
    return tf.Database(str(SYNTHETIC / (name + ".json"))).getSubstance(symbol), record


def reference_of(record):
    value = lambda k: record[k]["values"][0]
    return tf.methods.Reference(T=record["Tst"], P=record["Pst"], G=value("sm_gibbs_energy"), H=value("sm_enthalpy"), S=value("sm_entropy_abs"),
                                Cp=value("sm_heat_capacity_p"), V=value("sm_volume") * 1e-5)


def method_of(case, record):
    return tf.methods.substanceMethod(case[2], parameters=case[4], symbol=case[1])


@pytest.mark.parametrize("case", CASES, ids=lambda c: c[2])
@pytest.mark.parametrize("T,P", [(400.0, 1e6), (800.0, 1e8), (1200.0, 5e8)])
def test_a_fluid_alone_gives_the_result_of_the_old_class(case, T, P):
    substance, record = load(case[0], case[1])
    # with the lower T of the old class: the reference T, as neither has a heat capacity here
    old = getattr(tf, case[3])(substance).thermoProperties(T, P, tf.ThermoPropertiesSubstance(), False)
    new = method_of(case, record).evaluate(T, P, reference_of(record))
    assert new.H.val == pytest.approx(old.enthalpy.val, rel=1e-3, abs=1e-3)
    assert new.S.val == pytest.approx(old.entropy.val, rel=1e-3, abs=1e-3)
    assert new.V.val * 1e5 == pytest.approx(old.volume.val, rel=1e-3)


@pytest.mark.parametrize("case", CASES, ids=lambda c: c[2])
def test_the_volume_follows_the_ideal_gas_at_low_pressure(case):
    _, record = load(case[0], case[1])
    T, P = 1500.0, 1e5
    r = method_of(case, record).evaluate(T, P, reference_of(record))
    assert r.V.val == pytest.approx(8.31451 * T / P, rel=2e-2)


CO2 = dict(Tc=304.2, Pc=73.8e5, omega=0.225)
DERIVATIVES = [("PengRobinson78", dict(CO2, N=0.04)), ("SoaveRedlichKwong72", dict(CO2, N=0.04)), ("PRSV86", dict(CO2, k1=0.04)),
               ("CorkHP91", dict(CO2, k1=0.0)), ("SternerPitzer94", dict(Tc=CO2["Tc"], Pc=CO2["Pc"])),
               ("ChurakovGottschalk03", dict(sigma0=3.623, eps0=101.2, mu0=0.11))]


@pytest.mark.parametrize("method_id,parameters", DERIVATIVES, ids=lambda x: x if isinstance(x, str) else "")
def test_the_derivative_by_each_parameter_agrees_with_finite_differences(method_id, parameters):
    # a cast to double in the old code hid the derivatives by the parameters
    method = tf.methods.substanceMethod(method_id, parameters=parameters, symbol="CO2")
    T, P = 800.0, 1e8
    reference = tf.methods.Reference(T=298.15, P=1e5, G=-394e3, H=-393e3, S=213.0, Cp=37.0, V=2.4e-2)
    values = method.parameterValues()
    for i, value in enumerate(values):
        if value == 0.0:
            continue
        step = abs(value) * 1e-5
        high = method.withParameter(i, value + step).evaluate(T, P, reference).V.val
        low = method.withParameter(i, value - step).evaluate(T, P, reference).V.val
        assert method.parameterDerivative(i, T, P, reference)["V"] == pytest.approx((high - low) / (2 * step), rel=1e-3, abs=1e-12), i


@pytest.mark.parametrize("name,symbol", [("s13-fluid_peng_robinson78", "CO-pr78")])
def test_the_engine_has_the_same_result_on_both_paths_of_the_options(name, symbol):
    engine = tf.ThermoEngine(str(SYNTHETIC / (name + ".json")))
    base = engine.thermoPropertiesSubstance(800.0, 1e8, symbol).gibbs_energy.val
    preferences = engine.preferences
    preferences.apply_pressure_correction_to_gas_props = True
    engine.setPreferences(preferences)
    corrected = engine.thermoPropertiesSubstance(800.0, 1e8, symbol)
    assert corrected.gibbs_energy.val > base
    assert math.isfinite(corrected.enthalpy.val)
