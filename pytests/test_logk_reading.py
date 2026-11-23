# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# Which reference values the methods of log K read (logKReading in LogK_function_of_T): log K (or dG without log K) and dH,
# with dS derived from them, also when the record gives it; the implementation gems keeps the rule of GEMS (the given dH and
# dS). The errors come from the derivatives of the formula that runs, so a derived value keeps its correlation.

import math

import pytest
import thermofun as tf

PSI = "pytests/PsiTDB2020-subset-thermofun.json"
R, TR, LN10 = 8.31451, 298.15, math.log(10.0)
HEADER = {"thermohub": "2.0.0-draft.1"}


def component(symbol):
    return {"formula": "Ca", "class": "component", "aggregate_state": "crystal",
            "standard_state_properties": {"G": -1.0e6, "H": -1.1e6, "S": 50.0}, "methods": [{"method": "ConstantProperties"}]}


def one_reaction(properties, method="LogKConstantCp", implementation=None):
    application = {"method": method}
    if implementation:
        application["implementation"] = implementation
    return tf.Database(dict(HEADER, substances={"A": component("A"), "B": component("B")},
                            reactions={"R": {"equation": "A = B", "reaction_properties": properties, "methods": [application]}}))


def log_k(engine, T, symbol="R"):
    return engine.thermoPropertiesReaction(T, 1e5, symbol).log_equilibrium_constant


def gems(engine, method="LogKConstantCp"):
    p = engine.preferences
    p.method_implementation = {method: "gems"}
    engine.preferences = p
    return engine


def test_log_k_and_dh_are_read_and_ds_is_derived():
    """FeOH+2 of PSI 2020 gives log K -2.2 +- 0.02 and dH 43.3 +- 0.6 kJ/mol (selected), and dS and dG (derived from them)."""
    engine = tf.ThermoEngine(PSI)
    at_tr = log_k(engine, TR, "FeOH+2")
    assert at_tr.val == pytest.approx(-2.2, abs=1e-12)            # the given log K, not (dS - dH/Tr)/(R ln 10) = -2.19999998
    assert at_tr.err == pytest.approx(0.02, rel=1e-9)               # the error of log K alone; it was 600 (the error of dH in J/mol)
    T = 373.15
    by_dh = (1.0 / TR - 1.0 / T) / (R * LN10)                       # d log K / d dH of van 't Hoff (dCp = 0)
    assert log_k(engine, T, "FeOH+2").err == pytest.approx(math.hypot(0.02, 600.0 * by_dh), rel=1e-9)
    dS = engine.thermoPropertiesReaction(TR, 1e5, "FeOH+2").reaction_entropy
    assert dS.err == pytest.approx(math.hypot(600.0, R * TR * LN10 * 0.02) / TR, rel=1e-9)   # from log K and dH
    assert dS.sta[0] == tf.Status.calculated


def test_a_change_of_log_k_changes_the_result_and_a_change_of_ds_does_not():
    database = tf.Database(PSI)
    before = log_k(tf.ThermoEngine(database), 373.15, "FeOH+2").val
    database.set("/reactions/FeOH+2/reaction_properties/dS/value", 0.0)
    assert log_k(tf.ThermoEngine(database), 373.15, "FeOH+2").val == pytest.approx(before, abs=1e-12)
    database.set("/reactions/FeOH+2/reaction_properties/logK/value", -3.0)
    assert log_k(tf.ThermoEngine(database), 373.15, "FeOH+2").val == pytest.approx(before - 0.8, abs=1e-9)


def test_derivatives_by_the_values_read_and_a_derived_value_is_refused():
    engine = tf.ThermoEngine(PSI)
    T = 373.15

    def d(name):
        return engine.parameterSensitivities(T, 1e5, "FeOH+2", [("FeOH+2", "", name)], kind="reaction")["logK"][f"FeOH+2/{name}"]

    assert d("logK") == pytest.approx(1.0, rel=1e-12)
    assert d("dH") == pytest.approx(-(1.0 / T - 1.0 / TR) / (R * LN10), rel=1e-9)
    for derived in ("dS", "dG"):
        with pytest.raises(RuntimeError, match="derived from the others, not read"):
            d(derived)
    listed = {h.name() for h in engine.parameters("FeOH+2")}
    assert {"FeOH+2/logK", "FeOH+2/dH"} <= listed and not {"FeOH+2/dS", "FeOH+2/dG"} & listed


def test_the_implementation_gems_reads_dh_and_ds():
    engine = gems(tf.ThermoEngine(PSI))
    reaction = tf.Database(PSI).get("/reactions/FeOH+2/reaction_properties")
    dH, dS = reaction["dH"]["value"] * 1000.0, reaction["dS"]["value"]
    at_tr = log_k(engine, TR, "FeOH+2")
    assert at_tr.val == pytest.approx((dS - dH / TR) / (R * LN10), abs=1e-12)    # the old value, -2.19999998
    # dH and dS are the inputs, independent
    assert at_tr.err == pytest.approx(math.hypot(reaction["dS"]["uncertainty"], 600.0 / TR) / (R * LN10), rel=1e-9)
    by_ds = engine.parameterSensitivities(373.15, 1e5, "FeOH+2", [("FeOH+2", "", "dS")], kind="reaction")["logK"]["FeOH+2/dS"]
    assert by_ds == pytest.approx(1.0 / (R * LN10), rel=1e-12)
    assert "implementation: gems" in str(engine.route("FeOH+2", "reaction"))


def test_the_implementation_of_a_record_and_of_the_engine_option():
    properties = {"logK": -2.2, "dH": 43300.0, "dS": 100.0}
    assert log_k(tf.ThermoEngine(one_reaction(properties)), TR).val == pytest.approx(-2.2, abs=1e-12)
    old = (100.0 - 43300.0 / TR) / (R * LN10)
    assert log_k(tf.ThermoEngine(one_reaction(properties, implementation="gems")), TR).val == pytest.approx(old, abs=1e-12)
    assert log_k(gems(tf.ThermoEngine(one_reaction(properties))), TR).val == pytest.approx(old, abs=1e-12)
    engine = tf.ThermoEngine(one_reaction(properties))
    p = engine.preferences
    p.method_implementation = {"LogKConstantCp": "other"}
    with pytest.raises(ValueError, match="no implementation \"other\". Allowed: selected gems"):
        engine.preferences = p


def test_a_record_with_log_k_and_dh_has_the_error_of_log_k_at_tr():
    engine = tf.ThermoEngine(one_reaction({"logK": {"value": -2.2, "uncertainty": 0.02}, "dH": {"value": 43300.0, "uncertainty": 600.0}}))
    assert log_k(engine, TR).err == pytest.approx(0.02, rel=1e-9)
    dG = engine.thermoPropertiesReaction(TR, 1e5, "R").reaction_gibbs_energy
    assert dG.err == pytest.approx(R * TR * LN10 * 0.02, rel=1e-9)


def test_a_constant_log_k_from_dg_alone():
    engine = tf.ThermoEngine(one_reaction({"dG": 10000.0}, method="LogKConstant"))
    assert log_k(engine, 373.15).val == pytest.approx(-10000.0 / (R * TR * LN10), rel=1e-12)


def test_validate_reports_a_derived_value_that_differs():
    def codes(database):
        return [d for d in database.validate().all() if d.code == "V2-MTH-029"]

    agree = {"logK": -2.2, "dH": 43300.0, "dS": (43300.0 + R * TR * LN10 * -2.2) / TR}
    assert not codes(one_reaction(agree))
    off = codes(one_reaction(dict(agree, dS=agree["dS"] + 0.5, dG=12000.0)))
    assert [d.path for d in off] == ["/reactions/R/methods/0"] * 2
    assert "derives dS from log K and dH" in off[1].message and "derives dG from log K" in off[0].message
    assert not codes(one_reaction(dict(agree, dS=agree["dS"] + 0.5), implementation="gems"))


def test_validate_checks_log_k_constant():
    def codes(database):
        return [d for d in database.validate().all() if d.code == "V2-MTH-029"]

    assert codes(one_reaction({"logK": 2.0, "dG": 0.0}, method="LogKConstant"))           # dG is derived from log K, not read
    assert not codes(one_reaction({"logK": 2.0, "dG": -R * TR * LN10 * 2.0}, method="LogKConstant"))
    assert not codes(one_reaction({"logK": 2.0, "dG": 0.0}, method="LogKConstant", implementation="gems"))


def test_the_route_of_a_result_names_the_implementation_in_effect():
    engine = gems(tf.ThermoEngine(PSI))
    route = engine.thermoPropertiesReaction(373.15, 1e5, "FeOH+2").route
    assert [(m.method, m.implementation) for m in route.methods] == [("LogKConstantCp", "gems")]
