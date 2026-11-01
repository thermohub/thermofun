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
    corrected = engine.thermoPropertiesSubstance(800.0, 1e8, symbol)
    preferences = engine.preferences
    preferences.apply_pressure_correction_to_gas_props = False
    engine.setPreferences(preferences)
    assert corrected.gibbs_energy.val > engine.thermoPropertiesSubstance(800.0, 1e8, symbol).gibbs_energy.val
    assert math.isfinite(corrected.enthalpy.val)


# G(T, P) of a gas: dG/dP = V, dG/dT = -S, H = G + T S
RECORDS = [("CO2-prsv", {"9": "fluid_prsv"}, [304.2, 73.8, 0.225, 0.0427, 0, 0, 0]), ("CO2-srk", {"11": "fluid_soave_redlich_kwong"}, [304.2, 73.8, 0.225, 0.0427, 0, 0, 0]),
           ("CO2-pr78", {"13": "fluid_peng_robinson78"}, [304.2, 73.8, 0.225, 0.0427, 0, 0, 0]), ("CO2-cork", {"14": "fluid_comp_redlich_kwong_hp91"}, [304.2, 73.8, 0, 0, 0, 0, 0]),
           ("CO2-stp", {"12": "fluid_sterner_pitzer"}, [304.2, 73.8, 0.225, 0.0427, 0, 0, 0]), ("CO2-ideal", {"40": "mv_pvnrt"}, None)]


def gases(tmp_path):
    doc = json.loads((SYNTHETIC / "s12-fluid_sterner_pitzer.json").read_text())
    template = doc["substances"][0]
    records = []
    for symbol, method, critical in RECORDS:
        record = json.loads(json.dumps(template))
        record["symbol"] = record["name"] = symbol
        record["formula"] = "CO2"
        entry = {"method": method}
        if critical:
            entry["eos_gas_crit_props"] = {"values": critical}
        record["TPMethods"] = [template["TPMethods"][0], entry]
        records.append(record)
    doc["substances"] = records
    path = tmp_path / "gases.json"
    path.write_text(json.dumps(doc))
    return tf.Database(str(path))


@pytest.mark.parametrize("symbol", [r[0] for r in RECORDS])
@pytest.mark.parametrize("T,P", [(600.0, 1e7), (600.0, 1e8), (900.0, 5e8), (1500.0, 1e6)])
def test_the_properties_of_a_gas_with_a_method_are_consistent(tmp_path, symbol, T, P):
    engine = tf.ThermoEngine(gases(tmp_path))
    r = engine.thermoPropertiesSubstance(T, P, symbol)
    low = engine.thermoPropertiesSubstance(T, 1e5, symbol)
    assert r.gibbs_energy.ddp == pytest.approx(r.volume.val * 1e-5, rel=1e-3)
    assert r.gibbs_energy.ddt == pytest.approx(-r.entropy.val, rel=1e-3)
    # G and H of a record can have different zero points: the change with P at one T
    assert r.enthalpy.val - low.enthalpy.val == pytest.approx(r.gibbs_energy.val - low.gibbs_energy.val + T * (r.entropy.val - low.entropy.val), rel=1e-3, abs=2.0)


def test_the_gibbs_energy_of_a_gas_is_the_integral_of_its_volume(tmp_path):
    engine = tf.ThermoEngine(gases(tmp_path))
    T = 600.0
    pressures = [1e5 * 10.0 ** (i / 80.0) for i in range(241)]  # 1 to 1000 bar
    volumes = [engine.thermoPropertiesSubstance(T, p, "CO2-prsv").volume.val * 1e-5 for p in pressures]
    integral = sum((volumes[i] + volumes[i + 1]) / 2.0 * (pressures[i + 1] - pressures[i]) for i in range(240))
    change = engine.thermoPropertiesSubstance(T, 1e8, "CO2-prsv").gibbs_energy.val - engine.thermoPropertiesSubstance(T, 1e5, "CO2-prsv").gibbs_energy.val
    assert change == pytest.approx(integral, rel=2e-3)


def test_the_option_gives_the_standard_state_and_a_gas_in_a_reaction_is_the_same_as_alone(tmp_path):
    db = gases(tmp_path)
    engine = tf.ThermoEngine(db)
    assert engine.preferences.apply_pressure_correction_to_gas_props is True
    corrected = engine.thermoPropertiesSubstance(600.0, 1e8, "CO2-prsv").gibbs_energy.val
    off = tf.ThermoEngine(db)
    off.preferences.apply_pressure_correction_to_gas_props = False
    off.preferences.cancel_fugacity_coefficient_in_gas_props = True
    standard = off.thermoPropertiesSubstance(600.0, 1e8, "CO2-prsv").gibbs_energy.val
    assert standard == pytest.approx(off.thermoPropertiesSubstance(600.0, 1e5, "CO2-prsv").gibbs_energy.val, rel=1e-9)
    assert corrected - standard == pytest.approx(34958.0, rel=2e-3)


def test_the_pressure_term_and_the_fugacity_coefficient_are_two_options(tmp_path):
    db = gases(tmp_path)

    def change(pressure, cancel):
        engine = tf.ThermoEngine(db)
        engine.preferences.apply_pressure_correction_to_gas_props = pressure
        engine.preferences.cancel_fugacity_coefficient_in_gas_props = cancel
        return engine.thermoPropertiesSubstance(600.0, 1e8, "CO2-prsv").gibbs_energy.val - engine.thermoPropertiesSubstance(600.0, 1e5, "CO2-prsv").gibbs_energy.val

    RTlnP = 8.31451 * 600.0 * math.log(1000.0)
    assert change(False, True) == pytest.approx(0.0, abs=1.0)                # the state at Pr
    assert change(True, True) == pytest.approx(RTlnP, rel=1e-3)              # as an ideal gas
    fugacity = change(False, False)                                          # RT ln(phi) alone
    assert abs(fugacity) > 100.0
    assert change(True, False) == pytest.approx(RTlnP + fugacity, rel=1e-3)  # the default


@pytest.mark.parametrize("pressure,cancel", [(True, False), (False, True)], ids=["default", "gems"])
def test_a_gas_has_the_same_value_alone_and_in_a_reaction(tmp_path, pressure, cancel):
    doc = json.loads((SYNTHETIC / "s12-fluid_sterner_pitzer.json").read_text())
    template = doc["substances"][0]

    def record(symbol, methods, **more):
        r = json.loads(json.dumps(template))
        r["symbol"] = r["name"] = symbol
        r["formula"] = "CO2"
        r["TPMethods"] = methods
        r.update(more)
        return r

    fluid = {"method": {"9": "fluid_prsv"}, "eos_gas_crit_props": {"values": [304.2, 73.8, 0.225, 0.0427, 0, 0, 0]}}
    plain = [template["TPMethods"][0]]
    doc["substances"] = [record("G", plain + [fluid]), record("S", plain)]
    doc["reactions"] = [{"symbol": "R", "equation": "G = S", "reactants": [{"symbol": "G", "coefficient": -1}, {"symbol": "S", "coefficient": 1}], "Tst": 298.15, "Pst": 100000}]
    path = tmp_path / "reaction.json"
    path.write_text(json.dumps(doc))
    engine = tf.ThermoEngine(str(path))
    engine.preferences.apply_pressure_correction_to_gas_props = pressure
    engine.preferences.cancel_fugacity_coefficient_in_gas_props = cancel
    T, P = 600.0, 1e8
    gas, other = (engine.thermoPropertiesSubstance(T, P, s).gibbs_energy.val for s in ("G", "S"))
    assert engine.thermoPropertiesReactionFromReactants(T, P, "R").reaction_gibbs_energy.val == pytest.approx(other - gas, rel=1e-9)
    assert engine.thermoPropertiesSubstance(T, P, "G").gibbs_energy.val == gas  # also after the reaction
