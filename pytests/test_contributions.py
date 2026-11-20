# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# engine.contributions: the increment of each method, at many points, in columns that add up to the value.

import pytest
import thermofun as tf

FILE = "docs/tutorials/mineral-methods.yaml"
PROPERTIES = ("G", "H", "S", "Cp", "V")


@pytest.fixture(scope="module")
def engine():
    return tf.ThermoEngine(FILE)


def test_one_row_for_each_point_and_the_columns_add_up(engine):
    rows = engine.contributions("Calcite", T=[300.0, 600.0, 900.0, 1200.0], P=1e7)
    assert [r["T"] for r in rows] == [300.0, 600.0, 900.0, 1200.0] and all(r["P"] == 1e7 for r in rows)
    for row in rows:
        for name in PROPERTIES:
            parts = [v for k, v in row.items() if k.startswith(name + ":")]
            assert sum(parts) == pytest.approx(row[name], rel=1e-12, abs=1e-12)
            total = engine.thermoPropertiesSubstance(row["T"], row["P"], "Calcite")
            assert row[name] == getattr(total, {"G": "gibbs_energy", "H": "enthalpy", "S": "entropy", "Cp": "heat_capacity_cp", "V": "volume"}[name]).val


def test_the_columns_are_the_steps_of_explain(engine):
    row = engine.contributions("Calcite", 900.0, 1e7)[0]
    steps = engine.explain(900.0, 1e7, "Calcite")
    assert [k[3:] for k in row if k.startswith("Cp:") and k != "Cp:other"] == [s["method"] for s in steps]
    assert row["Cp:" + steps[0]["method"]] == steps[0]["Cp"]
    assert row["Cp:" + steps[1]["method"]] == pytest.approx(steps[1]["Cp"] - steps[0]["Cp"])


def test_the_landau_part_is_zero_above_its_transition(engine):
    # LandauHP98 of calcite: Tc0 = 966.85 degC, so no Cp from it far above
    below, above = engine.contributions("Calcite", [800.0, 1400.0], 1e5)
    assert below["Cp:LandauHP98"] > 0.0
    assert above["Cp:LandauHP98"] == pytest.approx(0.0, abs=1e-9)


def test_lists_of_t_and_p_go_together_and_a_wrong_length_is_an_error(engine):
    rows = engine.contributions("Calcite", [400.0, 500.0], [1e5, 1e8])
    assert [(r["T"], r["P"]) for r in rows] == [(400.0, 1e5), (500.0, 1e8)]
    with pytest.raises(ValueError, match="same length"):
        engine.contributions("Calcite", [400.0, 500.0], [1e5, 1e6, 1e7])


def test_the_catalog_says_which_methods_set_and_which_add():
    effects = {m["id"]: m["effects"] for m in tf.methods.catalog()}
    assert effects["CpPolynomial"]["G"] == "set"
    assert effects["LandauHP98"]["Cp"] == "add" and effects["LandauHP98"]["V"] == "set"
    assert effects["LogKPolynomial"] == {}


def test_a_substance_without_methods_of_its_own_has_the_totals_in_other():
    engine = tf.ThermoEngine("Resources/databases/aq17-thermofun.json")
    row = engine.contributions("H2O@", 400.0, 1e7)[0]
    assert set(k for k in row if ":" in k) == {name + ":other" for name in PROPERTIES}
    assert row["G:other"] == row["G"]
