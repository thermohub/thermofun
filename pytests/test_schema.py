# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The JSON Schema of format 2.0 (review of v2, S12, decision 9): up to date, the same keys as the reader, and every v2 file of
# the repository and the v2 form of every bundled database valid.

import glob
import json
import pathlib
import re

import pytest
import thermofun as tf
from thermofun import schema

jsonschema = pytest.importorskip("jsonschema")
yaml = pytest.importorskip("yaml")

ROOT = pathlib.Path(__file__).resolve().parent.parent
COMMITTED = ROOT / "ThermoFun" / "IO" / "V2" / "thermohub.schema.json"
SCHEMA = schema.schema()
VALIDATOR = jsonschema.Draft202012Validator(SCHEMA)


def test_the_committed_schema_is_up_to_date():
    assert COMMITTED.read_text() == schema.text(), "run: python python/src/thermofun/schema.py --write"


def reader_key_sets():
    """The sets of keys that the C++ reader knows, by object."""
    sets = []
    for name in ("V2ReadRecords.cpp", "V2ReadMethods.cpp", "V2Reader.cpp"):
        text = (ROOT / "ThermoFun" / "IO" / "V2" / name).read_text()
        for body in re.findall(r"std::set<std::string>\s+\w+\s*=\s*\{(.*?)\};", text, re.S):
            sets.append(set(re.findall(r'"([^"]+)"', body)))
    return sets


# a key of each object that only it has, to find its set in the reader
FINGERPRINT = {"element": "isotope_mass", "substance": "aggregate_state", "reaction": "equation", "method_definition": "operates_on",
               "parameter_set": "formula_units", "data_source": "doi", "thermodataset": "conventions", "method_application": "segments"}
NOT_IN_THE_FORMAT = {"standard_state_convention"}   # read only to warn (V2-KEY-003)


@pytest.mark.parametrize("name", sorted(FINGERPRINT))
def test_an_object_has_the_keys_that_the_reader_knows(name):
    found = [s for s in reader_key_sets() if FINGERPRINT[name] in s]
    assert len(found) == 1, name
    # record_type: the loader takes it off a file of one record before the reader sees the record
    assert set(SCHEMA["$defs"][name]["properties"]) - {"record_type"} == found[0] - NOT_IN_THE_FORMAT - {"record_type"}


def test_the_top_level_has_the_keys_that_the_reader_knows():
    found = [s for s in reader_key_sets() if "thermohub" in s]
    assert len(found) == 1 and set(SCHEMA["$defs"]["document"]["properties"]) == found[0]


def errors(document):
    return sorted("%s: %s" % ("/".join(map(str, e.absolute_path)), e.message[:200]) for e in VALIDATOR.iter_errors(document))


FILES = sorted(glob.glob(str(ROOT / "examples" / "v2" / "*.yaml")) + glob.glob(str(ROOT / "pytests" / "data" / "v2" / "*.json")))


@pytest.mark.parametrize("path", FILES, ids=lambda p: pathlib.Path(p).name)
def test_a_v2_file_of_the_repository_is_valid(path):
    text = pathlib.Path(path).read_text()
    document = yaml.safe_load(text) if path.endswith(".yaml") else json.loads(text)
    assert errors(document) == []


@pytest.mark.parametrize("path", sorted(glob.glob(str(ROOT / "Resources" / "databases" / "*.json"))), ids=lambda p: pathlib.Path(p).stem)
def test_the_v2_form_of_a_bundled_database_is_valid(path):
    for readable in (False, True):
        assert errors(json.loads(tf.Database(path).toJson(readable))) == []


def test_a_wrong_document_is_not_valid():
    bad = {"thermohub": "2.0", "substances": {"A": {"formula": "Ca", "standard_state_properties": {"G": {"value": 1, "uncertainty": {"standard": 1, "distribution": "uniform"}}}}}}
    assert errors(bad)
    assert errors({"thermohub": "2.0", "substances": {"A": {"formula": "Ca", "colour": "red"}}})
    assert not errors({"thermohub": "2.0", "substances": {"A": {"formula": "Ca", "_colour": "red"}}})


def test_a_file_of_one_record_is_valid(tmp_path):
    record = {"record_type": "substance", "symbol": "A", "formula": "Ca", "standard_state_properties": {"G": -1000}}
    assert errors(record) == []
    assert errors(dict(record, record_type="mineral"))


@pytest.mark.parametrize("path", sorted(glob.glob(str(ROOT / "examples" / "v2" / "methods" / "*.yaml"))), ids=lambda p: pathlib.Path(p).name)
def test_a_method_source_of_the_examples_is_a_valid_method_definition(path):
    validator = jsonschema.Draft202012Validator({**SCHEMA, "$ref": "#/$defs/method_definition", "oneOf": None} | {"oneOf": [{"$ref": "#/$defs/method_definition"}]})
    assert sorted(e.message[:200] for e in validator.iter_errors(yaml.safe_load(pathlib.Path(path).read_text()))) == []
