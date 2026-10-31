# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The v2 writer: legacy to v2 loses nothing, a second write gives the same text, and the file is good to read."""

import glob
import json
import os
import re
import struct

import pytest
import thermofun as tf

BUNDLED = sorted(glob.glob(os.path.join("Resources", "databases", "*-thermofun.json")))
TESTED = BUNDLED + ["pytests/test-aq17-gem-lma-thermofun.json", "pytests/test-thermoengine-thermofun.json", "tests/extra_models/extra_models_db.json"] + sorted(glob.glob("pytests/data/legacy-synthetic/*.json"))
POINTS = [(298.15, 1e5), (373.15, 1e7), (473.15, 5e7), (673.15, 1e8)]
SUBSTANCE = ["gibbs_energy", "enthalpy", "entropy", "heat_capacity_cp", "volume", "internal_energy", "helmholtz_energy"]
REACTION = ["log_equilibrium_constant", "reaction_gibbs_energy", "reaction_enthalpy", "reaction_entropy", "reaction_heat_capacity_cp", "reaction_volume"]


def outcome(function, *args):
    try:
        return function(*args)
    except RuntimeError:
        return None


def compare(legacy_db, other_db, tolerance, derivatives=1e-8, energy=None):
    legacy, other = tf.ThermoEngine(legacy_db), tf.ThermoEngine(other_db)
    checked = 0
    for T, P in POINTS:
        for kind, symbols, fields in (("thermoPropertiesSubstance", legacy_db.getSubstancesList(), SUBSTANCE),
                                      ("thermoPropertiesReaction", legacy_db.getReactionsList(), REACTION)):
            for symbol in symbols:
                want = outcome(getattr(legacy, kind), T, P, symbol)
                got = outcome(getattr(other, kind), T, P, symbol)
                assert (want is None) == (got is None), (symbol, T, P)
                if want is None:
                    continue
                checked += 1
                for field in fields:
                    a, b = getattr(got, field), getattr(want, field)
                    assert a.val == pytest.approx(b.val, rel=tolerance, abs=energy or tolerance), (symbol, T, P, field)
                    assert a.ddt == pytest.approx(b.ddt, rel=derivatives, abs=derivatives), (symbol, T, P, field, "ddt")
                    assert a.ddp == pytest.approx(b.ddp, rel=derivatives, abs=derivatives), (symbol, T, P, field, "ddp")
                    assert a.err == pytest.approx(b.err, rel=derivatives, abs=derivatives), (symbol, T, P, field, "err")
    return checked


@pytest.mark.parametrize("path", TESTED, ids=os.path.basename)
def test_the_exact_file_gives_the_results_of_the_legacy_file(path):
    legacy = tf.Database(path)
    v2 = tf.Database(legacy.toJson(readable=False))
    assert v2.numberOfSubstances() == legacy.numberOfSubstances()
    assert v2.numberOfReactions() == legacy.numberOfReactions()
    assert v2.numberOfElements() == legacy.numberOfElements()
    assert compare(legacy, v2, 1e-10) >= 0


@pytest.mark.parametrize("path", BUNDLED, ids=os.path.basename)
def test_the_readable_file_stays_within_the_precision_of_the_source(path):
    legacy = tf.Database(path)
    # float rounding of a parameter is up to 6e-8 relative; it grows where terms nearly cancel, most in the derivatives.
    # A small value of A or U can differ by 0.1 J/mol: its terms are large (decision 0023).
    assert compare(legacy, tf.Database(legacy.toJson()), 2e-5, derivatives=1e-3, energy=0.1) > 0


@pytest.mark.parametrize("path", BUNDLED, ids=os.path.basename)
@pytest.mark.parametrize("readable", [True, False], ids=["readable", "exact"])
def test_a_second_write_gives_the_same_text(path, readable):
    first = tf.Database(path).toJson(readable=readable)
    second = tf.Database(first).toJson(readable=readable)
    assert second == first
    yaml = tf.Database(path).toYaml(readable=readable)
    assert tf.Database(yaml).toYaml(readable=readable) == yaml


@pytest.mark.parametrize("path", BUNDLED, ids=os.path.basename)
def test_yaml_and_json_give_equal_records(path):
    db = tf.Database(path)
    from_json, from_yaml = tf.Database(db.toJson()), tf.Database(db.toYaml())
    for symbol in db.getSubstancesList():
        assert from_yaml.getSubstance(symbol).record() == from_json.getSubstance(symbol).record(), symbol
    for symbol in db.getReactionsList():
        assert from_yaml.getReaction(symbol).record() == from_json.getReaction(symbol).record(), symbol
    assert from_yaml.toJson() == from_json.toJson()


def float32(x):
    return struct.pack("f", x)


def changed_numbers(exact, readable, path=""):
    """The numbers that shortening changes, as (path, exact, readable)."""
    if isinstance(exact, dict):
        return [c for key in exact for c in changed_numbers(exact[key], readable[key], path + "/" + key)]
    if isinstance(exact, list):
        return [c for a, b in zip(exact, readable) for c in changed_numbers(a, b, path)]
    if isinstance(exact, (int, float)) and not isinstance(exact, bool) and exact != readable:
        return [(path, exact, readable)]
    return []


@pytest.mark.parametrize("path", BUNDLED, ids=os.path.basename)
def test_a_short_decimal_holds_the_same_float(path):
    """Shortening loses nothing: the short decimal and the value of the legacy file are the same single-precision number."""
    db = tf.Database(path)
    changes = changed_numbers(json.loads(db.toJson(readable=False)), json.loads(db.toJson()))
    assert changes
    for where, exact, readable in changes:
        assert float32(exact) == float32(readable), where
        assert abs(exact - readable) <= 6e-8 * abs(exact), where


@pytest.mark.parametrize("path", BUNDLED, ids=os.path.basename)
def test_the_exact_file_holds_the_same_doubles(path):
    db = tf.Database(path)
    again = tf.Database(db.toJson(readable=False))
    for symbol in db.getSubstancesList():
        a, b = db.getSubstance(symbol).record(), again.getSubstance(symbol).record()
        for name, q in a.standard_state_properties.items():
            assert b.standard_state_properties[name].value == q.value, (symbol, name)
        for x, y in zip(a.methods, b.methods):
            for name, q in y.parameters.items():
                assert q.value == x.parameters[name].value, (symbol, x.method, name)


def aq17():
    return tf.Database("Resources/databases/aq17-thermofun.json")


def test_no_float_noise_in_a_readable_file():
    text = aq17().toYaml()
    db = tf.Database("pytests/test-aq17-gem-lma-thermofun.json")
    assert "S: -56.484\n" in db.toYaml()
    assert "-56.484001159668" not in db.toYaml()
    assert "-56.484001159668" in db.toJson(readable=False)
    # no number has the long tail of a converted float
    noisy = re.findall(r"\d\.\d{10,}", text)
    assert len(noisy) < len(text) / 2e4, noisy[:10]


def test_a_readable_file_has_the_units_of_the_source():
    db = tf.Database("pytests/test-aq17-gem-lma-thermofun.json")
    document = json.loads(db.toJson())
    ca = document["substances"]["Ca+2"]
    assert ca["methods"][0]["units"] == {"energy": "cal", "pressure": "bar"}
    assert ca["methods"][0]["parameters"]["a2"] == pytest.approx(-725.2)
    assert ca["standard_state_properties"]["V"] == {"value": pytest.approx(-1.84387, rel=1e-5), "unit": "J/bar"}
    assert ca["standard_state_properties"]["G"] == -552821
    text = db.toYaml()
    assert "m3/mol" not in text
    assert "_legacy" not in text
    assert not re.search(r"uncertainty: 0(\.0)?[,}\s]", text)


def test_keys_are_in_the_order_of_the_format():
    document = json.loads(aq17().toJson())
    keys = list(document)
    assert keys[:3] == ["thermohub", "generator", "thermodataset"]
    assert keys.index("elements") < keys.index("substances") < keys.index("reactions") if "reactions" in keys else True
    written = {"thermohub": "2.0.0-draft.1", "substances": {"A": {"formula": "Ca", "standard_state_properties": {"G": {"unit": "kJ/mol", "uncertainty": 1.0, "value": 2.0}}}}}
    quantity = json.loads(tf.Database(json.dumps(written)).toJson())["substances"]["A"]["standard_state_properties"]["G"]
    assert list(quantity) == ["value", "uncertainty", "unit"]


def test_optional_parameters_at_their_default_are_not_written():
    document = json.loads(tf.Database("pytests/test-aq17-gem-lma-thermofun.json").toJson())
    segment = document["substances"]["Quartz"]["methods"][0]["segments"][0]["parameters"]
    assert not [k for k, v in segment.items() if v == 0]


def test_the_yaml_is_short_lines_with_block_records():
    lines = tf.Database("pytests/test-aq17-gem-lma-thermofun.json").toYaml().splitlines()
    assert lines[0].startswith("thermohub:")
    assert "substances:" in lines
    assert sum(len(line) > 130 for line in lines) < len(lines) / 100


@pytest.mark.parametrize("word", ["Al(OH)2+", "H2O@", "Ca+2", "1998HOL/POW", "true", "null", "1.5", "1e3", "a: b", "# x", "x #y", "-5", "~", "", "é", "[a]", "{a}", "O2(g)", "yes"])
def test_a_word_reads_back_from_yaml(word):
    document = {"thermohub": "2.0.0-draft.1", "substances": {"A": {"name": word, "formula": "Ca", "standard_state_properties": {"G": 1}}}}
    db = tf.Database(json.dumps(document))
    assert tf.Database(db.toYaml()).getSubstance("A").name() == word


@pytest.mark.parametrize("symbol", ["Al(OH)2+", "H2O@", "Fe|3|2O4", "1.5", "true", "A B", "a:b", "#x"])
def test_a_symbol_reads_back_from_yaml(symbol):
    db = tf.Database(json.dumps({"thermohub": "2.0.0-draft.1", "substances": {symbol: {"formula": "Ca", "standard_state_properties": {"G": 1}}}}))
    assert tf.Database(db.toYaml()).getSubstancesList() == [symbol]


def test_values_with_uncertainty_units_and_metadata_read_back():
    q = {"value": -237.183, "unit": "kJ/mol", "uncertainty": {"expanded": 0.2, "coverage_factor": 2, "distribution": "normal", "degrees_of_freedom": 12}, "quality": "core", "origin": "calculated", "source": "2018LOT/MAT"}
    document = {"thermohub": "2.0.0-draft.1", "substances": {"A": {"formula": "Ca", "standard_state_properties": {"G": q, "H": {"value": 1.0, "unit": "kcal/mol", "uncertainty": 0.0}}}}}
    db = tf.Database(json.dumps(document))
    again = tf.Database(db.toYaml())
    a, b = db.getSubstance("A").record(), again.getSubstance("A").record()
    ga, gb = a.standard_state_properties[tf.PropertyName.G], b.standard_state_properties[tf.PropertyName.G]
    assert gb.value == ga.value and gb.unit_as_written == "kJ/mol"
    assert gb.uncertainty.standard == pytest.approx(ga.uncertainty.standard, rel=1e-14)
    assert gb.uncertainty.coverage_factor_as_written == 2.0 and gb.uncertainty.distribution == "normal" and gb.uncertainty.degrees_of_freedom == 12
    assert gb.meta.quality == "core" and gb.meta.origin == "calculated" and gb.meta.source == "2018LOT/MAT"
    assert b.standard_state_properties[tf.PropertyName.H].uncertainty.standard == 0.0     # a defined zero stays


def test_project_records_read_back():
    db = tf.Database("pytests/data/v2/methods.json")
    text = db.toJson()
    assert tf.Database(text).toJson() == text
    again = tf.Database(text)
    assert again.parameterSets() == db.parameterSets()
    assert again.dataSources() == db.dataSources()
    assert again.methods()["CpPolynomial"].citation == "1998HOL/POW"
    assert again.thermoDataSet().solvent == "WATER"


def test_implementation_interpolation_covariance_and_unknown_keys_read_back():
    document = {"thermohub": "2.0.0-draft.1",
                "substances": {"A": {"formula": "Ca", "_mine": 1, "standard_state_properties": {"G": {"value": 1.0, "uncertainty": 1.0}, "H": {"value": 2.0, "uncertainty": 1.0}},
                                     "covariances": [{"paths": ["/standard_state_properties/G", "/standard_state_properties/H"], "matrix": [[1.0, 0.5], [0.5, 1.0]]}],
                                     "methods": [{"method": "ConstantCp", "_note": "x"}]}},
                "reactions": {"R": {"equation": "A = B", "methods": [{"method": "LogKTable", "interpolation": "cubic_spline", "order": 2,
                                                                      "table": {"temperatures": [273.15, 298.15, 373.15, 473.15], "logK": [1.0, 1.1, 1.2, 1.3]}}]}}}
    db = tf.Database(json.dumps(document))
    written = json.loads(tf.Database(db.toYaml()).toJson())
    assert written["substances"]["A"]["_mine"] == 1
    assert written["substances"]["A"]["methods"][0]["_note"] == "x"
    assert written["substances"]["A"]["covariances"][0]["matrix"] == [[1.0, 0.5], [0.5, 1.0]]
    assert written["reactions"]["R"]["equation"] == "A = B"
    assert written["reactions"]["R"]["methods"][0]["interpolation"] == "cubic_spline"
    assert written["reactions"]["R"]["methods"][0]["order"] == 2


@pytest.mark.parametrize("name, parse", [("data.yaml", False), ("data.yml", False), ("data.json", True)])
def test_save_chooses_the_format_by_the_name(name, parse, tmp_path):
    db = tf.Database("pytests/data/v2/minimal.json")
    target = str(tmp_path / name)
    db.save(target)
    text = open(target).read()
    if parse:
        assert json.loads(text)["thermohub"].startswith("2.")
    else:
        assert text.startswith("thermohub:")
    assert tf.Database(target).numberOfSubstances() == db.numberOfSubstances()


def test_save_to_a_place_that_does_not_exist_gives_an_error(tmp_path):
    with pytest.raises(RuntimeError, match="cannot be written"):
        tf.Database("pytests/data/v2/minimal.json").save(str(tmp_path / "no" / "such" / "data.yaml"))
