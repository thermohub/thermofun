# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The named outputs of a method (the terms and intermediate functions of a model): HKF88 gives the g function, the Born
# coefficient and the Born terms of G, H, S, Cp and V. The terms must agree with each other as thermodynamics says.

import pathlib

import pytest

import thermofun as tf

AQ = str(pathlib.Path(__file__).parent.parent / "Resources" / "databases" / "aq17-thermofun.json")
NAMES = ["g", "re", "omega", "omega_reference", "born_G", "born_H", "born_S", "born_Cp", "born_V"]


def term(engine, symbol, T, P, name):
    return engine.auxiliaryOutputs(symbol, T, P)[0]["HKF88:" + name]


def test_the_catalog_lists_the_named_outputs_of_hkf88_and_nothing_for_the_others():
    listed = {m["id"]: [o["name"] for o in m["auxiliary_outputs"]] for m in tf.methods.catalog() if m["auxiliary_outputs"]}
    assert listed == {"HKF88": NAMES}
    assert all(o["si_unit"] and o["description"] for m in tf.methods.catalog() for o in m["auxiliary_outputs"])


def test_a_table_of_the_outputs_has_a_row_for_each_point():
    rows = tf.ThermoEngine(AQ).auxiliaryOutputs("Ca+2", [298.15, 473.15], 1e8)
    assert [r["T"] for r in rows] == [298.15, 473.15]
    assert all(set(r) == {"T", "P"} | {"HKF88:" + n for n in NAMES} for r in rows)


def test_a_substance_without_named_outputs_gives_only_t_and_p():
    assert tf.ThermoEngine(AQ).auxiliaryOutputs("Calcite", 373.15, 1e8) == [{"T": 373.15, "P": 1e8}]


def test_explain_has_the_outputs_of_the_method_that_gives_them():
    steps = tf.ThermoEngine(AQ).explain(473.15, 1e8, "Ca+2")
    assert set(steps[0]["auxiliary"]) == set(NAMES)


def test_the_born_terms_follow_from_the_born_term_of_g():
    engine = tf.ThermoEngine(AQ)
    T, P = 473.15, 1e8
    G = lambda T, P: term(engine, "Ca+2", T, P, "born_G")
    S, Cp, V, H = (term(engine, "Ca+2", T, P, n) for n in ("born_S", "born_Cp", "born_V", "born_H"))
    h, hp = 0.01, 1e3
    assert S == pytest.approx(-(G(T + h, P) - G(T - h, P)) / (2 * h), rel=1e-6)
    assert Cp == pytest.approx(T * (term(engine, "Ca+2", T + h, P, "born_S") - term(engine, "Ca+2", T - h, P, "born_S")) / (2 * h), rel=1e-6)
    assert V == pytest.approx((G(T, P + hp) - G(T, P - hp)) / (2 * hp), rel=1e-4)
    assert H == pytest.approx(G(T, P) + T * S, rel=1e-9)


def test_the_g_function_is_zero_at_low_temperature_and_a_neutral_species_has_no_shift():
    engine = tf.ThermoEngine(AQ)
    assert abs(term(engine, "Ca+2", 298.15, 1e5, "g")) < 1e-10
    assert term(engine, "Ca+2", 573.15, 1e8, "g") < 0.0
    assert term(engine, "Ca+2", 573.15, 1e8, "re") < term(engine, "Ca+2", 298.15, 1e5, "re")
    neutral = next(s for s in engine.database().getSubstancesList() if engine.database().getSubstance(s).charge() == 0 and s.endswith("@") and s != "H2O@")
    assert term(engine, neutral, 573.15, 1e8, "omega") == pytest.approx(term(engine, neutral, 573.15, 1e8, "omega_reference"))


@pytest.mark.parametrize("implementation", ["reaktoro", "gems"])
def test_both_implementations_give_the_outputs_and_their_terms_agree(implementation):
    engine = tf.ThermoEngine(AQ)
    preferences = engine.preferences
    preferences.method_implementation = {"HKF88": implementation}
    engine.setPreferences(preferences)
    T, P, h = 573.15, 1e8, 0.01
    G = lambda T: term(engine, "Ca+2", T, P, "born_G")
    assert term(engine, "Ca+2", T, P, "born_S") == pytest.approx(-(G(T + h) - G(T - h)) / (2 * h), rel=1e-6)
    assert term(engine, "Ca+2", T, P, "re") > 0.0


def test_the_two_implementations_give_nearly_the_same_terms():
    # the same equations, with the digits of the constants of each code
    values = {}
    for implementation in ("reaktoro", "gems"):
        engine = tf.ThermoEngine(AQ)
        preferences = engine.preferences
        preferences.method_implementation = {"HKF88": implementation}
        engine.setPreferences(preferences)
        values[implementation] = engine.auxiliaryOutputs("Ca+2", 473.15, 1e8)[0]
    for name in ("omega", "born_G", "re"):
        assert values["gems"]["HKF88:" + name] == pytest.approx(values["reaktoro"]["HKF88:" + name], rel=2e-3, abs=1.0), name


def test_a_named_output_is_a_column_of_a_table_with_its_unit():
    setup = {"database": AQ, "conditions": {"T": {"values": [200], "unit": "degC"}, "P": {"values": [1000], "unit": "bar"}},
             "tables": [{"name": "born", "records": ["Ca+2", "Calcite"],
                         "columns": [{"property": "HKF88:born_G", "unit": "kJ/mol"}, {"property": "HKF88:g"}]}]}
    results = tf.table(setup)
    table = results["born"]
    assert [c["label"] for c in table.columns] == ["HKF88:born_G(kJ/mol)", "HKF88:g(Ao)"]
    want = term(tf.ThermoEngine(AQ), "Ca+2", 473.15, 1e8, "born_G") / 1000.0
    assert table.value("Ca+2", 200, 1000, "HKF88:born_G(kJ/mol)") == pytest.approx(want, rel=1e-12)
    # Calcite does not use HKF88: an empty cell, and a problem that says why
    assert any(p[1] == "Calcite" and "no named output" in p[-1] for p in results.problems)


@pytest.mark.parametrize("column, text", [({"property": "HKF88:nope"}, "has no named output nope"),
                                          ({"property": "HKF88:g", "show": "ddt"}, "a value only")])
def test_a_wrong_column_of_a_named_output_says_why(column, text):
    setup = {"database": AQ, "conditions": {"T": [298.15], "P": [1e5]}, "tables": [{"name": "x", "records": ["Ca+2"], "columns": [column]}]}
    with pytest.raises(tf.tables.SetupError, match=text):
        tf.table(setup)
