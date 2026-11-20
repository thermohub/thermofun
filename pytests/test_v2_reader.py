# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The v2 reader: the fixtures of pytests/data/v2 were made from legacy records, so they must give the same properties.

import glob
import importlib.util
import json
import os
import unittest

import pytest

import thermofun as tf

HERE = os.path.dirname(os.path.abspath(__file__))
V2 = "pytests/data/v2/"
SOURCES = {
    "minimal.json": "pytests/test-database-thermofun.json",
    "hkf.json": "pytests/test-aq17-gem-lma-thermofun.json",
    "minerals.json": "pytests/test-aq17-gem-lma-thermofun.json",
    "gases.json": "Resources/databases/mines16-thermofun.json",
    "reactions.json": "pytests/test-aq17-gem-lma-thermofun.json",
}
POINTS = [(298.15, 1e5), (373.15, 1e7), (473.15, 5e7)]
SUBSTANCE = ["gibbs_energy", "enthalpy", "entropy", "heat_capacity_cp", "volume"]
REACTION = ["log_equilibrium_constant", "reaction_gibbs_energy", "reaction_enthalpy", "reaction_entropy",
            "reaction_heat_capacity_cp", "reaction_volume"]


def engine(path):
    e = tf.ThermoEngine(path)
    p = e.preferences
    p.enable_memoize = False
    e.setPreferences(p)
    return e


@pytest.mark.parametrize("name", sorted(SOURCES))
def test_fixture_gives_the_properties_of_the_legacy_records(name):
    v2, legacy = engine(V2 + name), engine(SOURCES[name])
    db = v2.database()
    assert db.numberOfSubstances() + db.numberOfReactions() > 0
    for symbol in db.getSubstancesList():
        for T, P in POINTS:
            try:
                want = legacy.thermoPropertiesSubstance(T, P, symbol)
            except Exception:
                continue
            got = v2.thermoPropertiesSubstance(T, P, symbol)
            for n in SUBSTANCE:
                assert getattr(got, n).val == pytest.approx(getattr(want, n).val, rel=1e-9, abs=1e-9), (name, symbol, T, P, n)
    for symbol in db.getReactionsList():
        for T, P in POINTS:
            try:
                want = legacy.thermoPropertiesReaction(T, P, symbol)
            except Exception:
                continue
            got = v2.thermoPropertiesReaction(T, P, symbol)
            for n in REACTION:
                assert getattr(got, n).val == pytest.approx(getattr(want, n).val, rel=1e-9, abs=1e-9), (name, symbol, T, P, n)


def test_the_dataset_and_the_records_of_a_v2_file():
    db = tf.Database(V2 + "minimal.json")
    assert db.thermoDataSet().symbol == "minimal"
    assert db.thermoDataSet().schema_version.startswith("2.")
    record = db.getSubstance("H2O@").record()
    assert record.source_format == tf.DataFormat.V2
    assert record.original_text
    assert record.symbol == "H2O@"
    assert db.containsElement("C")


def test_a_single_v2_record():
    with open(V2 + "hkf.json") as f:
        data = json.load(f)
    text = json.dumps(dict(data["substances"]["Na+"], symbol="Na+"))
    substance = tf.Substance(text)
    assert substance.record().formula == data["substances"]["Na+"]["formula"]
    assert substance.methodGenEOS() != 0


def test_a_list_of_records_is_read_like_a_map():
    with open(V2 + "hkf.json") as f:
        data = json.load(f)
    data["substances"] = [dict(v, symbol=k) for k, v in data["substances"].items()]
    db = tf.Database(json.dumps(data))
    assert db.numberOfSubstances() == len(data["substances"])


def invalid_files():
    return sorted(f for f in os.listdir(V2 + "invalid") if f.endswith(".json"))


@pytest.mark.parametrize("name", invalid_files())
def test_an_invalid_file_is_refused_with_its_code(name):
    # the name of the file starts with the code that the reader must report, for example V2-MTH-002-unknown-parameter.json
    code = "-".join(name.split("-")[:3])
    with pytest.raises(Exception) as error:
        tf.Database(V2 + "invalid/" + name)
    assert code in str(error.value)


def test_project_records_are_read():
    db = tf.Database(V2 + "methods.json")
    assert "CpPolynomial" in db.methods()
    assert db.methods()["CpPolynomial"].citation == "1998HOL/POW"
    assert db.parameterSets()["hkf_na"].method == "HKF88"
    assert db.parameterSets()["hkf_na"].parameters["a1"].value == 1.0e-6
    assert set(db.dataSources()) == {"1998HOL/POW", "mydata"}
    assert db.dataSources()["1998HOL/POW"].year == 1998


def test_the_solvent_of_the_dataset_is_the_solvent_of_the_engine():
    engine = tf.ThermoEngine(tf.Database(V2 + "methods.json"))
    assert engine.solventSymbol() == "WATER"
    engine = tf.ThermoEngine(V2 + "minimal.json")
    assert engine.solventSymbol() == "H2O@"


# --- the new keys ----------------------------------------------------------------------------------

def one_substance(**more):
    record = {"symbol": "X", "formula": "Ca", "class": "component", "aggregate_state": "crystal",
              "standard_state_properties": {"G": -1000.0, "H": -1000.0, "S": 10.0, "Cp": 20.0, "V": 1e-5}}
    record.update(more)
    return record


def test_implementation_and_interpolation_keys():
    hkf = {"method": "HKF88", "implementation": "gems",
           "parameters": {"a1": 1e-6, "a2": 1e-3, "a3": 1e-5, "a4": 1e-2, "c1": 10.0, "c2": 1e4, "omega": 1e5}}
    s = tf.Substance(json.dumps(one_substance(methods=[hkf])))
    assert s.record().methods[0].implementation == "gems"
    r = tf.Reaction(json.dumps({"symbol": "R", "equation": "A = B", "reaction_properties": {"logK": 1.0},
                                "methods": [{"method": "LogKTable", "interpolation": "spline", "order": 2,
                                             "table": {"temperatures": [273.15, 298.15, 373.15, 473.15], "logK": [1.0, 1.1, 1.3, 1.6]}}]}))
    application = r.record().methods[0]
    assert application.interpolation_order == 2
    assert r.reactants() == {"A": -1.0, "B": 1.0}


def test_a_wrong_implementation_is_refused():
    with pytest.raises(RuntimeError, match="V2-MTH-009"):
        tf.Substance(json.dumps(one_substance(methods=[{"method": "ConstantCp", "implementation": "nowhere"}])))


def test_covariance_blocks():
    paths = ["/standard_state_properties/G", "/standard_state_properties/H"]
    s = tf.Substance(json.dumps(one_substance(covariances=[{"paths": paths, "matrix": [[4.0, 1.0], [1.0, 9.0]]}])))
    block = s.record().covariance[0]
    assert block.paths == paths and block.matrix == [4.0, 1.0, 1.0, 9.0]
    with pytest.raises(RuntimeError, match="V2-COV-003"):
        tf.Substance(json.dumps(one_substance(covariances=[{"paths": paths, "matrix": [1.0, 2.0, 3.0]}])))


def test_a_diagnostic_has_the_path_of_the_value():
    db = tf.Database(json.dumps({"thermohub": "2.0.0-draft.1",
                                 "substances": {"X": one_substance(aggregate_state="crystall", symbol="X")}}))
    found = [d for d in db.loadDiagnostics().all() if d.code == "V2-VOC-001"]
    assert found and found[0].record == "X" and found[0].path == "/substances/X/aggregate_state"
    assert "crystal" in found[0].message


def test_a_record_cannot_prefer_the_setting_record():
    s = tf.Substance(json.dumps(one_substance(preferred_property_source="record")))
    assert s.record().preferred_property_source is None


def test_a_v2_record_edited_with_an_old_setter():
    s = tf.Substance(json.dumps(one_substance(methods=[{"method": "ConstantCp"}])))
    s.setMethod_P(tf.MethodCorrP_Thrift.CPM_CON)
    methods = {m.method for m in s.record().methods}
    assert methods == {"ConstantCp", "ConstantVolume"}
    db = tf.Database()
    db.addSubstance(s)
    assert tf.ThermoEngine(db).thermoPropertiesSubstance(298.15, 1e5, "X").gibbs_energy.val == pytest.approx(-1000.0)


# --- whole databases -------------------------------------------------------------------------------

def whole_database_fixture():
    spec = importlib.util.spec_from_file_location("make_v2_fixtures", os.path.join(HERE, "data", "v2", "make_v2_fixtures.py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def assert_same(got, want, fields, what):
    for n in fields:
        a, b = getattr(got, n), getattr(want, n)
        assert a.val == pytest.approx(b.val, rel=1e-9, abs=1e-9), (what, n, "val")
        assert a.ddt == pytest.approx(b.ddt, rel=1e-8, abs=1e-8), (what, n, "ddt")
        assert a.ddp == pytest.approx(b.ddp, rel=1e-8, abs=1e-8), (what, n, "ddp")
        assert a.err == pytest.approx(b.err, rel=1e-8, abs=1e-8), (what, n, "err")
        assert a.sta == b.sta, (what, n, "status")


@pytest.mark.parametrize("path", sorted(glob.glob(os.path.join("Resources", "databases", "*-thermofun.json"))), ids=os.path.basename)
def test_a_whole_database_as_v2_gives_the_results_of_the_legacy_file(path):
    """All records of a bundled database, written in the v2 format and read back."""
    fixtures = whole_database_fixture()
    legacy_db = tf.Database(path)
    symbols = legacy_db.getSubstancesList()
    document = fixtures.document(path, symbols, legacy_db.getReactionsList(), legacy_db.getElementsList(), {"symbol": "whole"})
    document["thermodataset"]["conventions"] = {"water_triple_point_reference": legacy_db.conventions.water_triple_point_reference}
    v2_db = tf.Database(json.dumps(document))
    assert v2_db.numberOfSubstances() == legacy_db.numberOfSubstances()
    assert v2_db.numberOfReactions() == legacy_db.numberOfReactions()
    assert v2_db.numberOfElements() == legacy_db.numberOfElements()
    assert all(d.severity != tf.Severity.Error for d in v2_db.loadDiagnostics().all())

    def outcome(function, *args):
        try:
            return function(*args)
        except RuntimeError:
            return None

    legacy, v2 = tf.ThermoEngine(legacy_db), tf.ThermoEngine(v2_db)
    for T, P in POINTS:
        for symbol in symbols:
            want, got = outcome(legacy.thermoPropertiesSubstance, T, P, symbol), outcome(v2.thermoPropertiesSubstance, T, P, symbol)
            assert (want is None) == (got is None), (symbol, T, P)
            if want is not None:
                assert_same(got, want, SUBSTANCE, (symbol, T, P))
        for symbol in legacy_db.getReactionsList():
            want, got = outcome(legacy.thermoPropertiesReaction, T, P, symbol), outcome(v2.thermoPropertiesReaction, T, P, symbol)
            assert (want is None) == (got is None), (symbol, T, P)
            if want is not None:
                assert_same(got, want, REACTION, (symbol, T, P))
