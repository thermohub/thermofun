# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Database.validate, dataset folders and include, and the legacy writer."""

import json
import os
import pathlib

import pytest
import thermofun as tf

from test_v2_writer import BUNDLED, compare
from test_v2_yaml import yaml_text

HEAD = {"thermohub": "2.0.0-draft.1"}
V2 = pathlib.Path("pytests/data/v2").resolve()


def document(**sections):
    return json.dumps(dict(HEAD, **sections))


def mineral(**more):
    record = {"formula": "Ca", "class": "component", "aggregate_state": "crystal", "standard_state_properties": {"G": 1.0, "H": 1.0, "S": 1.0, "Cp": 1.0, "V": 1e-5},
              "methods": [{"method": "ConstantCp"}]}
    record.update(more)
    return record


def codes(db):
    return sorted({d.code for d in db.validate().all()})


# --- validate --------------------------------------------------------------------------------------

def test_a_good_database_has_no_problems():
    assert codes(tf.Database(document(substances={"A": mineral()}))) == []


@pytest.mark.parametrize("record, code", [
    (mineral(defining_reaction="R", methods=[]), "V2-REF-001"),
    (mineral(applicability={"t_min": 500, "t_max": 300}), "V2-APP-001"),
    (mineral(methods=[{"method": "ConstantCp", "applicability": {"p_min": 5, "p_max": 1}}]), "V2-APP-001"),
    (mineral(methods=[{"method": "ConstantCp", "parameter_set": "nope"}]), "V2-REF-004"),
    (mineral(data_source="nope"), "V2-REF-005"),
    (mineral(covariances=[{"paths": ["/standard_state_properties/G", "/standard_state_properties/H"], "matrix": [1.0, 0.5, 0.2, 1.0]}]), "V2-COV-001"),
    (mineral(covariances=[{"paths": ["/standard_state_properties/G", "/standard_state_properties/H"], "matrix": [1.0, 2.0, 2.0, 1.0]}]), "V2-COV-002"),
    (mineral(covariances=[{"paths": ["/standard_state_properties/G", "/standard_state_properties/U"], "matrix": [1.0, 0.0, 0.0, 1.0]}]), "V2-COV-004"),
    (mineral(methods=[{"method": "ConstantCp"}, {"method": "MurnaghanHP98", "parameters": {"alpha0": 1e-5, "kappa0": 1e10}}, {"method": "ConstantVolume"}, {"method": "BirchMurnaghan47", "parameters": {"a1": 1e-5, "kt00": 1e10, "kp": 4}}]), None),
])
def test_a_problem_of_a_substance_has_its_code(record, code):
    extra = {"data_sources": {"1998HOL/POW": {"title": "t"}}} if code == "V2-REF-005" else {}
    db = tf.Database(document(substances={"A": record}, **extra))
    if code:
        assert code in codes(db)
        found = next(d for d in db.validate().all() if d.code == code)
        assert found.record == "A" and found.path.startswith("/substances/A")


def test_good_covariances_have_no_problem():
    paths = ["/standard_state_properties/G", "/methods/0/parameters/a"]
    record = mineral(**{"class": "aqueous_solute"}, methods=[{"method": "CpExtendedUNIQUAC", "parameters": {"a": 1.0, "b": 1.0, "c": 1.0, "alpha": 1.0, "beta": 1.0}}], covariances=[{"paths": paths, "matrix": [[1.0, 0.5], [0.5, 1.0]]}])
    assert "V2-COV-004" not in codes(tf.Database(document(substances={"A": record})))


def test_two_base_methods_are_refused_at_the_load():
    with pytest.raises(RuntimeError, match="V2-MTH-012"):
        tf.Database(document(substances={"A": mineral(methods=[{"method": "ConstantCp"}, {"method": "ConstantProperties"}])}))


def test_a_method_for_another_class_is_refused_at_the_load():
    with pytest.raises(RuntimeError, match="V2-MTH-005"):
        tf.Database(document(substances={"A": mineral(methods=[{"method": "HKF88", "parameters": {"a1": 1, "a2": 1, "a3": 1, "a4": 1, "c1": 1, "c2": 1, "omega": 1}}])}))


def test_a_missing_required_parameter_is_reported():
    # a load accepts a block without any parameter, as a legacy record can have; validate says it
    db = tf.Database(document(substances={"A": mineral(methods=[{"method": "CpPolynomial", "segments": [{"t_min": 273.15, "t_max": 500}]}])}))
    assert "V2-MTH-004" in codes(db)


def test_segments_with_a_gap_are_reported():
    segments = [{"t_min": 273.15, "t_max": 400, "parameters": {"a0": 1.0}}, {"t_min": 500, "t_max": 600, "parameters": {"a0": 1.0}}]
    assert "V2-MTH-006" in codes(tf.Database(document(substances={"A": mineral(methods=[{"method": "CpPolynomial", "segments": segments}])})))


def test_problems_of_a_reaction():
    db = tf.Database(document(elements={"Ca": {"molar_mass": 40.0}, "O": {"molar_mass": 16.0}}, substances={"A": mineral(formula="Ca"), "B": mineral(formula="O")},
                              reactions={"unbalanced": {"equation": "A = B"}, "missing": {"equation": "A = C"}}))
    found = {d.record: d.code for d in db.validate().all()}
    assert found["unbalanced"] == "V2-RXN-002"
    assert found["missing"] == "V2-REF-002"


def test_a_record_without_methods_and_values_is_refused_at_the_load():
    with pytest.raises(RuntimeError, match="V2-MTH-012"):
        tf.Database(document(substances={"A": {"formula": "Ca"}}))


def test_a_cycle_of_defining_reactions_is_named():
    db = tf.Database(document(substances={"A": mineral(defining_reaction="ra", methods=[]), "B": mineral(defining_reaction="rb", methods=[])},
                              reactions={"ra": {"equation": "A = B"}, "rb": {"equation": "B = A"}}))
    cycle = [d for d in db.validate().all() if d.code == "V2-REF-010"]
    assert cycle and "->" in cycle[0].message


def test_a_published_dataset_needs_a_version():
    db = tf.Database(BUNDLED[0])
    assert all(d.code != "V2-DS-001" for d in db.validate().all())
    with pytest.raises(RuntimeError, match="V2-DS-001"):
        tf.Database(document(thermodataset={"symbol": "x", "status": "published"}))


@pytest.mark.parametrize("path", BUNDLED, ids=os.path.basename)
def test_bundled_databases_have_only_the_known_problems(path):
    with open("pytests/data/validate_bundled.expected.json") as f:
        expected = json.load(f)[os.path.basename(path)]
    found = {}
    for d in tf.Database(path).validate().all():
        found[d.code] = found.get(d.code, 0) + 1
    assert found == expected


# --- folders ---------------------------------------------------------------------------------------

def write(path, data, as_yaml):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(yaml_text(data) if as_yaml else json.dumps(data))


def make_dataset(root, as_yaml):
    ext = ".yaml" if as_yaml else ".json"
    full = json.loads((V2 / "minimal.json").read_text())
    folder = root / "datasets" / "minimal" / "v1.0.0"
    write(folder / ("thermodataset" + ext), dict(HEAD, thermodataset=full["thermodataset"]), as_yaml)
    write(folder / "elements" / ("all" + ext), dict(HEAD, elements=full["elements"]), as_yaml)
    names = sorted(full["substances"])
    write(folder / "substances" / ("a" + ext), dict(HEAD, substances={n: full["substances"][n] for n in names[:2]}), as_yaml)
    write(folder / "substances" / "more" / ("b" + ext), dict(HEAD, substances={n: full["substances"][n] for n in names[3:]}), as_yaml)
    write(folder / "substances" / ("single" + ext), dict(full["substances"][names[2]], symbol=names[2], record_type="substance"), as_yaml)
    write(folder / "reactions" / ("r" + ext), dict(HEAD, reactions=full.get("reactions", {})), as_yaml)
    write(root / "bibliography" / ("sources" + ext), dict(HEAD, data_sources={"1998HOL/POW": {"title": "a source"}}), as_yaml)
    write(root / "methods" / "definitions" / ("m" + ext), dict(HEAD, methods={"ConstantVolume": {"kind": "reference", "operates_on": "substance"}}), as_yaml)
    return folder


@pytest.mark.parametrize("as_yaml", [False, True], ids=["json", "yaml"])
def test_a_dataset_folder_gives_the_database_of_the_file(tmp_path, as_yaml):
    folder = make_dataset(tmp_path, as_yaml)
    db, whole = tf.Database(str(folder)), tf.Database(str(V2 / "minimal.json"))
    assert db.getSubstancesList() == whole.getSubstancesList()
    assert db.getReactionsList() == whole.getReactionsList()
    assert db.getElementsList() == whole.getElementsList()
    assert db.thermoDataSet().symbol == whole.thermoDataSet().symbol
    assert "1998HOL/POW" in db.dataSources() and "ConstantVolume" in db.methods()
    a, b = tf.ThermoEngine(db), tf.ThermoEngine(whole)
    for symbol in whole.getSubstancesList():
        assert a.thermoPropertiesSubstance(373.15, 1e6, symbol).gibbs_energy.val == b.thermoPropertiesSubstance(373.15, 1e6, symbol).gibbs_energy.val


def test_a_folder_without_a_dataset_file_is_named(tmp_path):
    with pytest.raises(RuntimeError, match="no thermodataset"):
        tf.Database(str(tmp_path))


def test_a_problem_in_a_folder_names_the_file(tmp_path):
    folder = make_dataset(tmp_path, True)
    bad = folder / "substances" / "bad.yaml"
    bad.write_text(yaml_text(HEAD) + "substances:\n  Z:\n    aggregate_state: crystall\n    standard_state_properties: {G: 1}\n")
    found = [d for d in tf.Database(str(folder)).loadDiagnostics().all() if d.code == "V2-VOC-001"]
    assert found[0].file.endswith("bad.yaml") and found[0].line == 4


@pytest.mark.parametrize("as_yaml", [False, True], ids=["json", "yaml"])
def test_include(tmp_path, as_yaml):
    ext = ".yaml" if as_yaml else ".json"
    write(tmp_path / "parts" / ("a" + ext), dict(HEAD, substances={"A": mineral()}), as_yaml)
    write(tmp_path / "parts" / ("b" + ext), dict(HEAD, substances={"B": mineral()}), as_yaml)
    write(tmp_path / ("main" + ext), dict(HEAD, include=["parts/*" + ext, "parts/a" + ext], substances={"C": mineral()}), as_yaml)
    db = tf.Database(str(tmp_path / ("main" + ext)))
    assert db.getSubstancesList() == ["A", "B", "C"]
    assert not [d for d in db.loadDiagnostics().all() if d.code == "V2-ID-003"]


def test_the_file_that_includes_replaces_what_it_includes(tmp_path):
    write(tmp_path / "base.yaml", dict(HEAD, substances={"A": mineral(formula="Ca")}), True)
    write(tmp_path / "main.yaml", dict(HEAD, include=["base.yaml"], substances={"A": mineral(formula="Mg")}), True)
    assert tf.Database(str(tmp_path / "main.yaml")).getSubstance("A").formula() == "Mg"


def test_an_include_cycle_is_refused(tmp_path):
    write(tmp_path / "a.yaml", dict(HEAD, include=["b.yaml"]), True)
    write(tmp_path / "b.yaml", dict(HEAD, include=["a.yaml"]), True)
    with pytest.raises(RuntimeError, match="V2-INC-001"):
        tf.Database(str(tmp_path / "a.yaml"))


def test_an_included_file_that_is_not_there_is_named(tmp_path):
    write(tmp_path / "a.yaml", dict(HEAD, include=["nothing.yaml"]), True)
    with pytest.raises(RuntimeError, match="nothing.yaml"):
        tf.Database(str(tmp_path / "a.yaml"))


# --- legacy writer ---------------------------------------------------------------------------------

@pytest.mark.parametrize("path", BUNDLED + ["pytests/test-aq17-gem-lma-thermofun.json"], ids=os.path.basename)
def test_a_record_written_from_its_fields_gives_the_results_of_the_file(path):
    db = tf.Database(path)
    again = tf.Database(db.toLegacyJson(use_original_text=False))
    assert again.numberOfSubstances() == db.numberOfSubstances()
    assert again.numberOfReactions() == db.numberOfReactions()
    assert again.conventions.water_triple_point_reference == db.conventions.water_triple_point_reference
    assert compare(db, again, 1e-10) > 0


def test_an_unedited_record_is_written_as_its_text():
    path = "pytests/test-database-thermofun.json"
    written, original = json.loads(tf.Database(path).toLegacyJson()), json.load(open(path))
    key = lambda r: r["symbol"]  # noqa: E731
    assert sorted(written["substances"], key=key) == sorted(original["substances"], key=key)
    assert written["thermodataset"] == original.get("thermodataset", "")


@pytest.mark.parametrize("name", ["minimal.json", "hkf.json", "minerals.json", "gases.json"])
def test_a_v2_file_written_as_legacy_gives_the_same_results(name):
    db = tf.Database(str(V2 / name))
    assert compare(db, tf.Database(db.toLegacyJson()), 1e-9) > 0


def test_what_the_legacy_format_cannot_hold_is_named():
    record = mineral(data_source="x", methods=[{"method": "ConstantProperties"}], covariances=[{"paths": ["/standard_state_properties/G"], "matrix": [1.0]}])
    db = tf.Database(document(substances={"A": record}, reactions={"R": {"equation": "A = B", "methods": [{"method": "LogKTable", "table": {"temperatures": [273.15, 298.15, 373.15, 473.15], "logK": [1.0, 1.1, 1.2, 1.3]}}]}}))
    messages = " ".join(d.message for d in db.legacyWriteProblems().all())
    assert all(d.code == "LEG-WRT-001" for d in db.legacyWriteProblems().all())
    for word in ("ConstantProperties", "data_source", "covariance", "table"):
        assert word in messages


def test_save_in_the_legacy_format(tmp_path):
    db = tf.Database(str(V2 / "minimal.json"))
    target = tmp_path / "old.json"
    db.save(str(target), format=tf.DataFormat.Legacy)
    written = json.loads(target.read_text())
    assert "thermohub" not in written and written["substances"]
    assert tf.Database(str(target)).numberOfSubstances() == db.numberOfSubstances()


def test_the_text_of_an_edited_record_is_written_from_its_fields():
    source = '{"symbol": "A", "formula": "Ca", "sm_gibbs_energy": {"values": [-5.0]}}'
    s = tf.Substance(source)
    assert s.jsonString() == source
    s.setFormula("Mg")
    written = json.loads(s.jsonString())
    assert written["formula"] == "Mg" and written["sm_gibbs_energy"]["values"] == [-5.0]
    assert tf.Substance(s.jsonString()).formula() == "Mg"

    v2 = tf.Substance(json.dumps(dict(mineral(), symbol="B")))
    v2.setFormula("Mg")
    written = json.loads(v2.jsonString())
    assert written["symbol"] == "B" and written["formula"] == "Mg" and "standard_state_properties" in written
    assert tf.Substance(v2.jsonString()).formula() == "Mg"
