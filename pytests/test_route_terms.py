# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The terms and variables of a method on the route of a result (decision 0030): HKF88 gives the Born terms of G, H, S, Cp
# and V (terms) and the g function, the effective radius and the Born coefficient (variables). The terms must agree with
# each other as thermodynamics says.

import pathlib

import pytest

import thermofun as tf

AQ = str(pathlib.Path(__file__).parent.parent / "Resources" / "databases" / "aq17-thermofun.json")
TERMS = ["born_G", "born_H", "born_S", "born_Cp", "born_V"]
VARIABLES = ["g", "re", "omega", "omega_reference"]


def term(engine, symbol, T, P, name):
    part = "terms" if name in TERMS else "variables"
    return engine.routeTable(T, P, symbol, part)[0]["HKF88:" + name]


def test_the_catalog_lists_the_terms_and_variables_of_hkf88_and_nothing_for_the_others():
    terms = {m["id"]: [(o["name"], o["property"]) for o in m["terms"]] for m in tf.methods.catalog() if m["terms"]}
    variables = {m["id"]: [o["name"] for o in m["variables"]] for m in tf.methods.catalog() if m["variables"]}
    assert terms == {"HKF88": [(n, n[5:]) for n in TERMS]}
    assert variables == {"HKF88": VARIABLES}
    assert all(o["si_unit"] and o["description"] for m in tf.methods.catalog() for o in m["terms"] + m["variables"])


def test_a_table_of_the_terms_has_a_row_for_each_point():
    engine = tf.ThermoEngine(AQ)
    rows = engine.routeTable([298.15, 473.15], 1e8, "Ca+2", "terms")
    assert [r["T"] for r in rows] == [298.15, 473.15]
    assert all(set(r) == {"T", "P"} | {"HKF88:" + n for n in TERMS} for r in rows)
    assert all(set(r) == {"T", "P"} | {"HKF88:" + n for n in VARIABLES} for r in engine.routeTable([298.15, 473.15], 1e8, "Ca+2", "variables"))


def test_a_substance_without_terms_gives_only_t_and_p():
    assert tf.ThermoEngine(AQ).routeTable(373.15, 1e8, "Calcite", "terms") == [{"T": 373.15, "P": 1e8}]


def test_the_route_of_every_result_has_them():
    engine = tf.ThermoEngine(AQ)
    method = engine.thermoPropertiesSubstance(473.15, 1e8, "Ca+2").info.route.methods[0]
    assert list(method.terms) == TERMS and list(method.variables) == VARIABLES
    assert "    terms: {born_G: " in str(engine.thermoPropertiesSubstance(473.15, 1e8, "Ca+2").info.route)


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
        values[implementation] = engine.routeTable(473.15, 1e8, "Ca+2", "terms")[0] | engine.routeTable(473.15, 1e8, "Ca+2", "variables")[0]
    for name in ("omega", "born_G", "re"):
        assert values["gems"]["HKF88:" + name] == pytest.approx(values["reaktoro"]["HKF88:" + name], rel=2e-3, abs=1.0), name


def test_a_term_is_a_property_of_a_batch_with_its_unit():
    setup = {"database": AQ, "conditions": {"T": {"values": [200], "unit": "degC"}, "P": {"values": [1000], "unit": "bar"}},
             "results": [{"name": "born", "substances": ["Ca+2", "Calcite"],
                          "properties": [{"name": "terms:HKF88:born_G", "unit": "kJ/mol"}, {"name": "variables:HKF88:g"},
                                         {"name": "contribution:HKF88:G"}]}]}
    results = tf.batch(setup)
    table = results["born"]
    assert [c["label"] for c in table.columns] == ["terms:HKF88:born_G(kJ/mol)", "variables:HKF88:g(Ao)", "contribution:HKF88:G(J/mol)"]
    engine = tf.ThermoEngine(AQ)
    want = term(engine, "Ca+2", 473.15, 1e8, "born_G") / 1000.0
    assert table.value("Ca+2", 200, 1000, "terms:HKF88:born_G(kJ/mol)") == pytest.approx(want, rel=1e-12)
    assert table.value("Ca+2", 200, 1000, "contribution:HKF88:G(J/mol)") == pytest.approx(engine.thermoPropertiesSubstance(473.15, 1e8, "Ca+2").gibbs_energy.val, rel=1e-12)
    # Calcite does not use HKF88: an empty cell, and a problem that says why
    assert any(p[1] == "Calcite" and "does not use that method" in p[-1] for p in results.problems)


@pytest.mark.parametrize("property, text", [({"name": "terms:HKF88:nope"}, "has no term nope"),
                                            ({"name": "variables:HKF88:g", "show": "ddt"}, "a value only"),
                                            ({"name": "contribution:HKF88:logK"}, "a contribution is to G, H, S, Cp or V")])
def test_a_wrong_property_of_a_route_says_why(property, text):
    setup = {"database": AQ, "conditions": {"T": [298.15], "P": [1e5]}, "results": [{"name": "x", "substances": ["Ca+2"], "properties": [property]}]}
    with pytest.raises(tf.BatchSetupError, match=text):
        tf.batch(setup)
