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


# Sterner and Pitzer have their own equations for H2O and CO2. J/bar: 1 cm3/mol is 0.1 J/bar.
STERNER_PITZER = [
    ("H2O", 647.3, 220.9, 1000.0, 1e5, 8.31451 * 1000.0 / 1e5 * 1e5, 1e-2),  # supercritical, close to an ideal gas
    ("H2O", 647.3, 220.9, 500.0, 1e5, 8.31451 * 500.0 / 1e5 * 1e5, 2e-2),    # steam, below the saturation pressure
    ("H2O", 647.3, 220.9, 500.0, 1e8, 2.0, 5e-2),                            # liquid: 0.9 g/cm3
    ("CO2", 304.2, 73.8, 1000.0, 1e5, 8.31451 * 1000.0 / 1e5 * 1e5, 1e-2),
    ("CO2", 304.2, 73.8, 280.0, 1e6, 8.31451 * 280.0 / 1e6 * 1e5 * 0.94, 3e-2),  # vapour below Psat, 4.2 MPa
    ("CO2", 304.2, 73.8, 280.0, 5e6, 4.9, 5e-2),                              # liquid: 0.9 g/cm3
]


@pytest.mark.parametrize("formula,Tc,Pc,T,P,volume,tolerance", STERNER_PITZER)
def test_the_sterner_pitzer_equations_of_water_and_carbon_dioxide(tmp_path, formula, Tc, Pc, T, P, volume, tolerance):
    doc = json.loads((SYNTHETIC / "s12-fluid_sterner_pitzer.json").read_text())
    record = doc["substances"][0]
    record["symbol"] = record["name"] = "fluid"
    record["formula"] = formula
    record["TPMethods"][1]["eos_gas_crit_props"]["values"][:2] = [Tc, Pc]
    path = tmp_path / "fluid.json"
    path.write_text(json.dumps(doc))
    result = tf.ThermoEngine(str(path)).thermoPropertiesSubstance(T, P, "fluid")
    assert result.volume.val == pytest.approx(volume, rel=tolerance)
    assert math.isfinite(result.gibbs_energy.val) and math.isfinite(result.enthalpy.val) and math.isfinite(result.entropy.val)
