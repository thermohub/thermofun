# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The YAML of the v2 format, read in C++: the same records as JSON, and diagnostics with the line and the column."""

import glob
import json
import os
import time

import pytest
import thermofun as tf

V2 = "pytests/data/v2/"
HEADER = "thermohub: 2.0.0-draft.1\n"


def to_yaml(node, indent=0):
    """Block YAML of a JSON tree. Numbers and strings as JSON writes them, which YAML reads the same."""
    pad = " " * indent
    lines = []
    if isinstance(node, dict):
        for key, value in node.items():
            head = pad + json.dumps(key) + ":"
            if isinstance(value, (dict, list)) and value:
                lines += [head] + to_yaml(value, indent + 2)
            else:
                lines.append(head + " " + json.dumps(value))
    else:
        for value in node:
            if isinstance(value, (dict, list)) and value:
                inner = to_yaml(value, indent + 2)
                lines.append(pad + "- " + inner[0].lstrip())
                lines += inner[1:]
            else:
                lines.append(pad + "- " + json.dumps(value))
    return lines


def yaml_text(document):
    return "\n".join(to_yaml(document)) + "\n"


FIXTURES = sorted(glob.glob(V2 + "*.json"))


@pytest.mark.parametrize("path", FIXTURES, ids=os.path.basename)
def test_yaml_and_json_give_equal_records(path, tmp_path):
    with open(path) as f:
        document = json.load(f)
    target = tmp_path / "data.yaml"
    target.write_text(yaml_text(document))
    from_json, from_yaml = tf.Database(path), tf.Database(str(target))
    assert from_yaml.thermoDataSet() == from_json.thermoDataSet()
    assert from_yaml.getSubstancesList() == from_json.getSubstancesList()
    for symbol in from_json.getSubstancesList():
        assert from_yaml.getSubstance(symbol).record() == from_json.getSubstance(symbol).record(), symbol
    for symbol in from_json.getReactionsList():
        assert from_yaml.getReaction(symbol).record() == from_json.getReaction(symbol).record(), symbol
    for symbol in from_json.getElementsList():
        assert from_yaml.getElement(symbol).record() == from_json.getElement(symbol).record(), symbol
    assert from_yaml.methods() == from_json.methods()
    assert from_yaml.parameterSets() == from_json.parameterSets()
    assert from_yaml.dataSources() == from_json.dataSources()


def test_yaml_as_people_write_it():
    text = HEADER + """
# a comment
units: {energy: cal}
thermodataset:
  symbol: written
  conventions: {water_triple_point_reference: NEA_HGK}
defaults: &minerals
  class: component
  aggregate_state: crystal
substances:
  A:
    <<: *minerals
    formula: Ca
    standard_state_properties:
      G: -1000           # in cal
      H: {value: -4.184, unit: kJ/mol}
      S: "10 J/(mol.K)"
      Cp: 1.0e1
      V: 1.0e-5
    methods:
      - method: ConstantCp
"""
    db = tf.Database(text.replace("defaults: &minerals", "_defaults: &minerals"))
    assert db.thermoDataSet().symbol == "written"
    assert db.conventions.water_triple_point_reference == "NEA_HGK"
    record = db.getSubstance("A").record()
    assert record.class_ == "component" and record.aggregate_state == "crystal"
    p = tf.ThermoEngine(db).thermoPropertiesSubstance(298.15, 1e5, "A")
    assert p.gibbs_energy.val == pytest.approx(-4184.0)
    assert p.enthalpy.val == pytest.approx(-4184.0)
    assert p.entropy.val == pytest.approx(10.0)


@pytest.mark.parametrize("yaml_value, expected", [("1.5", 1.5), ("-2", -2), ("1e3", 1000), (".5", 0.5), ("+7.0E-1", 0.7)])
def test_plain_numbers(yaml_value, expected):
    db = tf.Database(HEADER + "substances:\n  A:\n    standard_state_properties:\n      G: " + yaml_value + "\n")
    assert db.getSubstance("A").record().standard_state_properties[tf.PropertyName.G].value == pytest.approx(expected)


def test_a_word_in_quotes_stays_a_word():
    db = tf.Database(HEADER + 'substances:\n  A:\n    name: "1.5"\n    formula: Ca\n')
    assert db.getSubstance("A").name() == "1.5"


def test_a_diagnostic_of_a_yaml_file_has_its_line_and_column(tmp_path):
    target = tmp_path / "data.yaml"
    target.write_text(HEADER + "substances:\n  A:\n    aggregate_state: crystall\n    formula: Ca\n")
    db = tf.Database(str(target))
    found = [d for d in db.loadDiagnostics().all() if d.code == "V2-VOC-001"]
    assert found[0].line == 4 and found[0].column > 1 and found[0].file == str(target)
    assert "data.yaml:4:" in str(found[0])


def test_a_diagnostic_of_a_json_file_has_its_line_too(tmp_path):
    target = tmp_path / "data.json"
    target.write_text('{"thermohub": "2.0.0-draft.1",\n "substances": {\n  "A": {\n   "aggregate_state": "crystall"}}}\n')
    found = [d for d in tf.Database(str(target)).loadDiagnostics().all() if d.code == "V2-VOC-001"]
    assert found[0].line == 4


def test_a_symbol_with_a_slash_has_its_line(tmp_path):
    target = tmp_path / "data.yaml"
    target.write_text(HEADER + 'data_sources:\n  1998HOL/POW:\n    title: t\n  my/data:\n    title: t\n')
    found = [d for d in tf.Database(str(target)).loadDiagnostics().all() if d.code == "V2-SRC-001"]
    assert found[0].line == 5


def test_an_error_gives_its_code_and_its_line(tmp_path):
    target = tmp_path / "data.yaml"
    target.write_text(HEADER + "substances:\n  A:\n    methods:\n      - method: HKF88\n        parameters: {zz: 1}\n")
    with pytest.raises(RuntimeError, match=r"V2-MTH-002"):
        tf.Database(str(target))


@pytest.mark.parametrize("text", ["a: [1, 2", "a: {b: 1\nc: 2\n", "\tthermohub: x\n"])
def test_a_text_that_is_not_yaml_is_refused(text, tmp_path):
    target = tmp_path / "bad.yaml"
    target.write_text(text)
    with pytest.raises(RuntimeError, match="V2-YAML-001"):
        tf.Database(str(target))


def test_a_key_written_twice_gives_a_warning(tmp_path):
    target = tmp_path / "data.yaml"
    target.write_text(HEADER + "substances:\n  A:\n    formula: Ca\n  A:\n    formula: Mg\n")
    db = tf.Database(str(target))
    assert [d for d in db.loadDiagnostics().all() if d.code == "V2-ID-003"]
    assert db.getSubstance("A").formula() == "Mg"


def test_two_documents_are_refused(tmp_path):
    target = tmp_path / "data.yaml"
    target.write_text("---\n" + HEADER + "---\n" + HEADER)
    with pytest.raises(RuntimeError, match="V2-YAML-001"):
        tf.Database(str(target))


def test_yaml_without_the_version_key_is_not_a_v2_file(tmp_path):
    target = tmp_path / "data.yaml"
    target.write_text("substances:\n  A:\n    formula: Ca\n")
    assert tf.Database(str(target)).numberOfSubstances() == 1


def test_a_file_that_is_not_there_is_named():
    with pytest.raises(RuntimeError, match="no-such-file.yaml"):
        tf.Database("no-such-file.yaml")


def seconds(function, repeat=3):
    best = float("inf")
    for _ in range(repeat):
        start = time.perf_counter()
        function()
        best = min(best, time.perf_counter() - start)
    return best


def test_yaml_of_heracles_loads_faster_than_the_legacy_json(tmp_path):
    import importlib.util
    spec = importlib.util.spec_from_file_location("make_v2_fixtures", V2 + "make_v2_fixtures.py")
    fixtures = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(fixtures)
    path = "Resources/databases/heracles-thermofun.json"
    db = tf.Database(path)
    document = fixtures.document(path, db.getSubstancesList(), db.getReactionsList(), db.getElementsList(), {"symbol": "heracles"})
    target = tmp_path / "heracles.yaml"
    target.write_text(yaml_text(document))
    legacy, yaml = seconds(lambda: tf.Database(path)), seconds(lambda: tf.Database(str(target)))
    assert tf.Database(str(target)).numberOfSubstances() == db.numberOfSubstances()
    assert yaml < legacy, f"YAML {yaml:.3f} s, legacy JSON {legacy:.3f} s"
