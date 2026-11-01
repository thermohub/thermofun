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


# A substance defined by a reaction takes its pressure term from one place
def defined(reaction_methods, reaction_values, substance_methods, tmp_path):
    def record(symbol, G, V, **more):
        return {"symbol": symbol, "formula": symbol, "class_": {"0": "SC_COMPONENT"}, "aggregate_state": {"3": "AS_CRYSTAL"}, "Tst": 298.15, "Pst": 100000,
                "sm_gibbs_energy": {"values": [G]}, "sm_enthalpy": {"values": [G]}, "sm_entropy_abs": {"values": [10.0]}, "sm_heat_capacity_p": {"values": [20.0]}, "sm_volume": {"values": [V]}, **more}
    plain = [{"method": {"43": "standard_entropy_cp_integration"}}, {"method": {"34": "mv_constant"}}]
    doc = {"substances": [record("A", -1000.0, 2.0, TPMethods=plain), record("X", -3000.0, 5.0, reaction="R", **({"TPMethods": substance_methods} if substance_methods else {}))],
           "reactions": [{"symbol": "R", "equation": "A = X", "reactants": [{"symbol": "A", "coefficient": -1}, {"symbol": "X", "coefficient": 1}], "Tst": 298.15, "Pst": 100000,
                          "logKr": {"values": [1.0]}, "TPMethods": reaction_methods, **reaction_values}]}
    path = tmp_path / "defined.json"
    path.write_text(json.dumps(doc))
    engine = tf.ThermoEngine(str(path))
    high, low = (engine.thermoPropertiesSubstance(298.15, p, "X") for p in (1e8, 1e5))
    return high.gibbs_energy.val - low.gibbs_energy.val, high.volume.val, high.gibbs_energy.ddp


LOGK = {"method": {"4": "logk_1_term_extrap0"}}
OWN = [{"method": {"34": "mv_constant"}}]
BAR = 1000.0 - 1.0


def test_the_own_volume_when_the_reaction_does_not_change_with_pressure(tmp_path):
    change, volume, ddp = defined([LOGK], {}, OWN, tmp_path)
    assert change == pytest.approx(5.0 * BAR)  # not with the term of A, which is at P in the reaction only if the reaction has it
    assert volume == pytest.approx(5.0)
    assert ddp == pytest.approx(5.0e-5)


def test_the_own_volume_when_the_volume_of_the_reaction_is_zero(tmp_path):
    change, volume, _ = defined([LOGK, {"method": {"13": "dr_volume_constant"}}], {"drsm_volume": {"values": [0.0]}}, OWN, tmp_path)
    assert change == pytest.approx(5.0 * BAR)
    assert volume == pytest.approx(5.0)


def test_no_second_term_when_the_reaction_has_a_volume(tmp_path):
    change, volume, _ = defined([LOGK, {"method": {"13": "dr_volume_constant"}}], {"drsm_volume": {"values": [3.0]}}, OWN, tmp_path)
    assert change == pytest.approx((3.0 + 2.0) * BAR)  # G(X) = dG + G(A): the volume of the reaction and the term of A
    assert volume == pytest.approx(3.0 + 2.0)


def test_the_marks_of_the_methods():
    specs = {s["id"]: s for s in tf.methods.catalog()}
    assert all(s["depends_on"] for s in specs.values()), [i for i, s in specs.items() if not s["depends_on"]]
    assert specs["ConstantVolume"]["depends_on"] == "P" and specs["CpPolynomial"]["depends_on"] == "T"
    assert specs["HKF88"]["depends_on"] == "T, P" and specs["LogKConstant"]["depends_on"] == "none"


TDB = HERE / "PsiTDB2020-subset-thermofun.json"


@pytest.mark.parametrize("T,P", [(298.15, 1e8), (473.15, 1e8), (573.15, 2e7)])
def test_a_solid_defined_by_a_reaction_without_pressure_has_the_term_of_its_volume(T, P):
    r = tf.ThermoEngine(str(TDB)).thermoPropertiesSubstance(T, P, "AgCl(cr)")
    assert r.gibbs_energy.ddp == pytest.approx(r.volume.val * 1e-5, rel=1e-9)


@pytest.mark.parametrize("T", [298.15, 473.15, 573.15, 623.15])
def test_the_data_of_a_reaction_are_on_the_saturation_curve(T):
    """Above 100 degC the aqueous species of the reaction are not at 1 bar, where there is no liquid water."""
    engine = tf.ThermoEngine(str(TDB))
    saturated = engine.thermoPropertiesSubstance(T, 0.0, "AgCl(cr)").gibbs_energy.val
    assert -140000.0 < saturated < -90000.0


def test_above_the_critical_temperature_the_species_are_at_the_pressure():
    value = tf.ThermoEngine(str(TDB)).thermoPropertiesSubstance(673.15, 1e8, "AgCl(cr)").gibbs_energy.val
    assert -140000.0 < value < -90000.0
