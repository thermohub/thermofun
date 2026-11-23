# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""No stale result after a change of the settings or of the data.

The engine caches its results (memoization). Each test makes a first calculation, so that the result is in the
cache, then makes a change, and compares the next result with an engine made with the change from the start.
Before the fix, the engine gave the cached (old) result.
"""
import json
import os

import pytest
import thermofun as tf

HERE = os.path.dirname(os.path.abspath(__file__))
DB = os.path.join(HERE, "test-thermoengine-thermofun.json")
T, P = 373.15, 1e5
SYMBOLS = ["Cal", "H2O@", "Ca+2"]


def gibbs(engine, symbol):
    return engine.thermoPropertiesSubstance(T, P, symbol).gibbs_energy.val


def first_calculation(engine):
    return {s: gibbs(engine, s) for s in SYMBOLS}


def calcite_record(shift):
    """The record of Cal with the reference Gibbs energy changed by shift J/mol, as a JSON string."""
    with open(DB) as f:
        data = json.load(f)
    record = next(s for s in data["substances"] if s["symbol"] == "Cal")
    record["sm_gibbs_energy"]["values"][0] += shift
    return json.dumps(record)


@pytest.mark.parametrize("field, value", [("water_properties", "STEAM_TABLES"),
                                          ("apparent_properties", "BERMAN_BROWN")])
def test_conventions_changed_in_place(field, value):
    engine = tf.ThermoEngine(DB)
    before = first_calculation(engine)
    setattr(engine.conventions, field, value)

    expected = tf.ThermoEngine(DB)
    setattr(expected.conventions, field, value)
    for s in SYMBOLS:
        assert gibbs(engine, s) == gibbs(expected, s)
    assert any(gibbs(engine, s) != before[s] for s in SYMBOLS)


def test_conventions_set_as_a_whole():
    conventions = tf.EngineConventions()
    conventions.water_properties = "STEAM_TABLES"
    expected = tf.ThermoEngine(DB)
    expected.setConventions(conventions)

    engine = tf.ThermoEngine(DB)
    first_calculation(engine)
    engine.conventions = conventions
    assert engine.conventions.water_properties == "STEAM_TABLES"
    assert gibbs(engine, "H2O@") == gibbs(expected, "H2O@")

    engine.setConventions(tf.EngineConventions())
    assert gibbs(engine, "H2O@") == gibbs(tf.ThermoEngine(DB), "H2O@")


def test_preferences_set_as_a_whole():
    preferences = tf.EnginePreferences()
    preferences.enable_memoize = False
    engine = tf.ThermoEngine(DB)
    first_calculation(engine)
    engine.preferences = preferences
    assert engine.preferences.enable_memoize is False
    assert gibbs(engine, "Cal") == gibbs(tf.ThermoEngine(DB), "Cal")


def test_database_conventions_changed_in_place():
    engine = tf.ThermoEngine(DB)
    before = first_calculation(engine)
    engine.database().conventions.water_triple_point_reference = "NEA_HGK"

    expected = tf.ThermoEngine(DB)
    expected.database().conventions.water_triple_point_reference = "NEA_HGK"
    assert gibbs(engine, "H2O@") == gibbs(expected, "H2O@")
    assert gibbs(engine, "H2O@") != before["H2O@"]


def test_append_data_replaces_a_record():
    engine = tf.ThermoEngine(DB)
    before = first_calculation(engine)
    engine.appendData([calcite_record(1000.0)], "substance")
    assert gibbs(engine, "Cal") == pytest.approx(before["Cal"] + 1000.0, abs=1e-6)
    assert gibbs(engine, "Ca+2") == before["Ca+2"]


def test_record_set_through_the_database():
    engine = tf.ThermoEngine(DB)
    before = first_calculation(engine)
    engine.database().setSubstance(calcite_record(1000.0))
    assert gibbs(engine, "Cal") == pytest.approx(before["Cal"] + 1000.0, abs=1e-6)


def test_record_changed_through_a_reference():
    engine = tf.ThermoEngine(DB)
    before = first_calculation(engine)
    substance = engine.database().substance("Cal")
    reference = substance.thermoReferenceProperties()
    reference.gibbs_energy = tf.ThermoScalar(reference.gibbs_energy.val + 1000.0)
    substance.setThermoReferenceProperties(reference)
    assert gibbs(engine, "Cal") == pytest.approx(before["Cal"] + 1000.0, abs=1e-6)
