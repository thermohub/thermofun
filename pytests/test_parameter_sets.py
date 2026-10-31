# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The parameters of a method can come from a parameter set of the file: the same results as inline, and an inline value goes first.

import json

import pytest
import thermofun as tf

T, P = 473.15, 5e7


def with_set():
    """Na+ takes its HKF parameters from a set."""
    doc = json.loads(tf.Database("pytests/data/v2/hkf.json").toJson(readable=False))
    method = next(m for m in doc["substances"]["Na+"]["methods"] if m["method"] == "HKF88")
    parameters = method.pop("parameters")
    method["parameter_set"] = "Na_hkf"
    doc["parameter_sets"] = {"Na_hkf": {"method": "HKF88", "parameters": parameters}}
    return doc, method, parameters


def gibbs(doc):
    return tf.ThermoEngine(tf.Database(json.dumps(doc))).thermoPropertiesSubstance(T, P, "Na+").gibbs_energy.val


def test_a_set_gives_the_results_of_inline_values():
    doc, _, _ = with_set()
    inline = json.loads(tf.Database("pytests/data/v2/hkf.json").toJson(readable=False))
    assert gibbs(doc) == gibbs(inline)


def test_an_inline_value_goes_before_the_set():
    doc, method, parameters = with_set()
    plain = gibbs(doc)
    number = parameters["omega"]
    method["parameters"] = {"omega": number * 1.5}
    assert gibbs(doc) != plain
    whole = json.loads(tf.Database("pytests/data/v2/hkf.json").toJson(readable=False))
    next(m for m in whole["substances"]["Na+"]["methods"] if m["method"] == "HKF88")["parameters"]["omega"] = number * 1.5
    assert gibbs(doc) == gibbs(whole)


def test_a_handle_on_a_value_of_a_set_changes_only_this_record():
    doc, _, parameters = with_set()
    second = json.loads(json.dumps(doc["substances"]["Na+"]))
    second["name"] = "copy"
    doc["substances"]["K2"] = second
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    before = engine.thermoPropertiesSubstance(T, P, "K2").gibbs_energy.val
    omega = engine.parameter("Na+", "HKF88", "omega")
    start = omega.value()
    assert start != 0.0
    omega.setValue(start * 1.5)
    assert engine.thermoPropertiesSubstance(T, P, "K2").gibbs_energy.val == before
    assert engine.thermoPropertiesSubstance(T, P, "Na+").gibbs_energy.val != before


def test_the_derivatives_and_the_errors_read_the_set():
    doc, _, parameters = with_set()
    doc["parameter_sets"]["Na_hkf"]["parameters"]["c1"] = {"value": parameters["c1"], "uncertainty": 1.0}
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    d = engine.parameterSensitivities(T, P, "Na+", [("Na+", "HKF88", "c1")])["Cp"]["Na+/HKF88/c1"]
    assert d != 0.0
    engine.preferences.propagate_parameter_errors = True
    assert engine.thermoPropertiesSubstance(T, P, "Na+").heat_capacity_cp.err == pytest.approx(abs(d) * 1.0, rel=1e-10)
