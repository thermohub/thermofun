# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The route of a result (decision 0030): every result says how it was calculated. The result is the sum of the contribution
of the source, of the contributions of the methods and of the changes of the conversions; engine.route(symbol) has the same
tree without numbers; routeTable gives it at many points."""

import glob
import math

import pytest

import thermofun as tf

AQ17 = "Resources/databases/aq17-thermofun.json"
PSI = "pytests/PsiTDB2020-subset-thermofun.json"
MINERALS = "docs/tutorials/mineral-methods.yaml"
FIELDS = {"G": "gibbs_energy", "H": "enthalpy", "S": "entropy", "Cp": "heat_capacity_cp", "V": "volume"}
REACTION_FIELDS = {"G": "reaction_gibbs_energy", "H": "reaction_enthalpy", "S": "reaction_entropy", "Cp": "reaction_heat_capacity_cp", "V": "reaction_volume"}
POINTS = [(298.15, 1e5), (373.15, 0.0), (523.15, 5e7), (873.15, 2e8)]


def close(a, b):
    return a == pytest.approx(b, rel=1e-9, abs=1e-9 * max(1.0, abs(b)))


def check_sum(result, fields):
    route = result.info.route
    total = route.total()
    for name, field in fields.items():
        value = getattr(result, field)
        if value.sta[0] != tf.Status.notdefined:
            assert close(total[name], value.val), (route.symbol, name, total[name], value.val, str(route))


@pytest.mark.parametrize("path", sorted(glob.glob("Resources/databases/*.json")) + [PSI, MINERALS])
def test_the_result_is_the_sum_of_its_route_for_every_bundled_record(path):
    engine = tf.ThermoEngine(path)
    database = engine.database()
    checked = 0
    for symbol in database.getSubstancesList():
        for T, P in POINTS:
            try:
                result = engine.thermoPropertiesSubstance(T, P, symbol)
            except Exception:
                continue
            assert result.info.route is not None and result.info.route.symbol == symbol
            check_sum(result, FIELDS)
            checked += 1
    for symbol in database.getReactionsList():
        for T, P in POINTS[:3]:
            try:
                result = engine.thermoPropertiesReaction(T, P, symbol)
            except Exception:
                continue
            check_sum(result, REACTION_FIELDS)
            checked += 1
    assert checked > 0


def test_conversions_of_the_conventions_of_the_results_are_rows_of_the_route():
    engine = tf.ThermoEngine(AQ17)
    conventions = engine.conventions
    conventions.energy_reference.elements = "berman_brown"
    conventions.standard_state.pressure = "1atm"
    engine.conventions = conventions
    for symbol in ("Calcite", "CO2", "Ca+2", "H2O@"):
        result = engine.thermoPropertiesSubstance(400.0, 1e7, symbol)
        check_sum(result, FIELDS)
    route = engine.thermoPropertiesSubstance(400.0, 1e7, "CO2").info.route
    assert [(c.what, c.detail) for c in route.conversions] == [("convention", "berman_brown, 1atm")]
    assert route.conversions[0].change["G"] != 0.0


def test_the_options_of_gases_and_the_fallback_are_rows_too():
    engine = tf.ThermoEngine(AQ17)
    preferences = engine.preferences
    preferences.round_to_uncertainty = True
    engine.setPreferences(preferences)
    for symbol in engine.database().getSubstancesList()[:40]:
        check_sum(engine.thermoPropertiesSubstance(350.0, 1e6, symbol), FIELDS)


def test_the_static_route_has_the_tree_of_the_calculated_one():
    engine = tf.ThermoEngine(PSI)

    def tree(route):
        return (route.symbol, route.kind, route.by, tuple(m.method for m in route.methods), tuple((n.role, n.coefficient, tree(n.route)) for n in route.needs))

    for symbol in engine.database().getSubstancesList()[:60] + engine.database().getReactionsList()[:30]:
        static = engine.route(symbol)
        assert static.T is None and static.P is None
        try:
            if engine.database().containsSubstance(symbol):
                calculated = engine.thermoPropertiesSubstance(323.15, 1e5, symbol).info.route
            else:
                calculated = engine.thermoPropertiesReaction(323.15, 1e5, symbol).info.route
        except Exception:
            continue
        # the solvent: the route without numbers in both; methods for the pressure: in the calculated route only those that run
        assert (calculated.symbol, calculated.kind, calculated.by) == (static.symbol, static.kind, static.by), symbol
        assert [n.role for n in calculated.needs] == [n.role for n in static.needs], symbol
        if calculated.by == "methods":
            assert [m.method for m in calculated.methods] == [m.method for m in static.methods], symbol


def test_a_record_from_its_defining_reaction_needs_the_reaction_and_the_other_reactants():
    engine = tf.ThermoEngine(PSI)
    symbol = next(s for s in engine.database().getSubstancesList() if engine.route(s).by == "reaction")
    route = engine.thermoPropertiesSubstance(373.15, 1e5, symbol).info.route
    roles = [n.role for n in route.needs]
    assert roles[0] == "defining_reaction" and set(roles[1:]) <= {"reactant", "solvent"}
    reaction = route.needs[0].route
    assert reaction.kind == "reaction" and reaction.T == 373.15


def test_a_reaction_from_its_reactants_needs_each_with_its_coefficient():
    engine = tf.ThermoEngine(AQ17)
    result = engine.thermoPropertiesReaction(373.15, 1e5, "Calcite + 2H+ = Ca+2 + CO2@ + H2O@")
    route = result.info.route
    assert route.by == "reactants" and route.kind == "reaction"
    coefficients = {n.route.symbol: n.coefficient for n in route.needs}
    assert coefficients == {"Calcite": -1.0, "H+": -2.0, "Ca+2": 1.0, "CO2@": 1.0, "H2O@": 1.0}
    assert sum(n.coefficient * n.route.total()["G"] for n in route.needs) == pytest.approx(result.reaction_gibbs_energy.val)


def test_hydrogen_ion_electron_and_water():
    engine = tf.ThermoEngine(PSI)
    assert engine.thermoPropertiesSubstance(298.15, 1e5, "H+").info.route.by == "hydrogen_ion"
    electron = engine.thermoPropertiesSubstance(373.15, 1e5, "e-").info.route
    assert electron.by == "hydrogen_electrode" and electron.needs[0].coefficient == 0.5 and electron.needs[0].route.symbol == "H2(g)"
    water = tf.ThermoEngine(AQ17).thermoPropertiesSubstance(373.15, 1e5, "H2O@").info.route
    assert water.by == "water" and [m.method for m in water.methods] == ["WaterIAPWS95", "JohnsonNorton91"]
    assert water.methods[0].contribution["G"] != 0.0 and water.methods[1].contribution["G"] == 0.0


def test_the_route_prints_as_yaml():
    text = str(tf.ThermoEngine(AQ17).thermoPropertiesSubstance(473.15, 1e7, "Ca+2").info.route)
    lines = text.splitlines()
    assert lines[:5] == ["symbol: Ca+2", "kind: substance", "T: 473.15", "P: 10000000", "by: methods"]
    assert "  - method: HKF88" in lines and "  - role: solvent" in lines
    static = str(tf.ThermoEngine(AQ17).route("Ca+2"))
    assert "T:" not in static and "contribution" not in static


def test_p_is_as_used_and_a_cache_hit_has_the_route():
    engine = tf.ThermoEngine(AQ17)
    first = engine.thermoPropertiesSubstance(373.15, 0.0, "Ca+2").info.route
    second = engine.thermoPropertiesSubstance(373.15, 0.0, "Ca+2").info.route
    assert first.P == pytest.approx(101418, rel=1e-3) and second.P == first.P
    assert str(first) == str(second)


def test_engine_route_notes_what_cannot_be_calculated():
    route = tf.ThermoEngine(AQ17).route("NoSuchRecord")
    assert route.notes and "no such substance" in route.notes[0].text


def test_route_table_one_row_for_each_point_and_the_columns_add_up():
    engine = tf.ThermoEngine(MINERALS)
    rows = engine.routeTable([300.0, 600.0, 900.0, 1200.0], 1e7, "Calcite")
    assert [r["T"] for r in rows] == [300.0, 600.0, 900.0, 1200.0] and all(r["P"] == 1e7 for r in rows)
    for row in rows:
        total = engine.thermoPropertiesSubstance(row["T"], row["P"], "Calcite")
        for name, field in FIELDS.items():
            parts = [v for k, v in row.items() if k.endswith(":" + name)]
            assert sum(parts) == pytest.approx(row[name], rel=1e-12, abs=1e-12)
            assert row[name] == pytest.approx(getattr(total, field).val, rel=1e-14, abs=1e-20)


def test_the_landau_part_is_zero_above_its_transition():
    # LandauHP98 of calcite: Tc0 = 966.85 degC, so no Cp from it far above
    below, above = tf.ThermoEngine(MINERALS).routeTable([800.0, 1400.0], 1e5, "Calcite")
    assert below["LandauHP98:Cp"] > 0.0
    assert above["LandauHP98:Cp"] == pytest.approx(0.0, abs=1e-9)


def test_route_table_lists_go_together_and_a_wrong_length_or_part_is_an_error():
    engine = tf.ThermoEngine(MINERALS)
    rows = engine.routeTable([400.0, 500.0], [1e5, 1e8], "Calcite")
    assert [(r["T"], r["P"]) for r in rows] == [(400.0, 1e5), (500.0, 1e8)]
    with pytest.raises(ValueError, match="same length"):
        engine.routeTable([400.0, 500.0], [1e5, 1e6, 1e7], "Calcite")
    with pytest.raises(ValueError, match="allowed: contribution, terms, variables"):
        engine.routeTable(400.0, 1e5, "Calcite", "steps")


def test_a_record_by_water_or_reaction_has_its_source_as_a_column():
    row = tf.ThermoEngine(AQ17).routeTable(400.0, 1e7, "H2O@")[0]
    assert row["WaterIAPWS95:G"] == row["G"] and row["JohnsonNorton91:G"] == 0.0
    engine = tf.ThermoEngine(PSI)
    symbol = next(s for s in engine.database().getSubstancesList() if engine.route(s).by == "reaction")
    row = engine.routeTable(373.15, 1e5, symbol)[0]
    assert "reaction:G" in row


def test_a_reaction_has_a_route_table():
    engine = tf.ThermoEngine(PSI)
    symbol = engine.database().getReactionsList()[0]
    row = engine.routeTable(373.15, 1e5, symbol, kind="reaction")[0]   # a substance has the same symbol
    assert engine.route(symbol, "reaction").kind == "reaction" and engine.route(symbol).kind == "substance"
    assert row["G"] == pytest.approx(engine.thermoPropertiesReaction(373.15, 1e5, symbol).reaction_gibbs_energy.val)


def test_the_catalog_says_which_methods_set_and_which_add():
    contributes = {m["id"]: m["contributes_to"] for m in tf.methods.catalog()}
    assert contributes["CpPolynomial"] == {"G": "sets", "H": "sets", "S": "sets", "Cp": "sets", "V": "sets"}
    assert contributes["LandauHP98"]["Cp"] == "adds" and contributes["LandauHP98"]["V"] == "sets"
    assert contributes["LogKPolynomial"]["logK"] == "sets"


def test_parameter_sensitivities_take_a_substance_or_a_reaction():
    engine = tf.ThermoEngine(PSI)
    reaction = engine.database().getReactionsList()[0]
    got = engine.parameterSensitivities(298.15, 1e5, reaction, [(reaction, "", "logK")], kind="reaction")
    assert set(got) >= {"logK", "dG"}
    got = engine.parameterSensitivities(298.15, 1e5, "Calcite", [("Calcite", "", "G")]) if engine.database().containsSubstance("Calcite") else None
    assert got is None or set(got) >= {"G", "H"}
    assert not hasattr(engine, "reactionParameterSensitivities")


def test_the_removed_functions_are_gone():
    engine = tf.ThermoEngine(AQ17)
    for name in ("explain", "contributions", "auxiliaryOutputs", "calculationPath", "thermoPropertiesSubstanceDetailed"):
        assert not hasattr(engine, name), name
    assert math.isfinite(engine.thermoPropertiesSubstance(T=298.15, P=1e5, symbol="Calcite").gibbs_energy.val)
