# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The tools: the converter, the validator, the snapshot, the consistency report and the generated reference pages."""

import json
import math
import pathlib

import pytest

import thermofun as tf
from thermofun import consistency, convert, docs, validate

ROOT = pathlib.Path(__file__).resolve().parents[1]
LEGACY = ROOT / "pytests" / "test-aq17-gem-lma-thermofun.json"
REFERENCE = ROOT / "docs" / "reference"
DATA = str(ROOT / "Resources" / "databases")
HEAD = {"thermohub": "2.0.0-draft.1"}
R = 8.31451
T0, P0 = 298.15, 1e5


def values(path):
    """G and log K of all records at one point: the same numbers are what counts for a conversion."""
    e = tf.ThermoEngine(str(path))
    db = e.database()
    return ({s: e.thermoPropertiesSubstance(400.0, 1e7, s).gibbs_energy.val for s in db.getSubstancesList()},
            {r: e.thermoPropertiesReaction(400.0, 1e7, r).log_equilibrium_constant.val for r in db.getReactionsList()})


# --- the converter ---------------------------------------------------------------------------------------------------

@pytest.mark.parametrize("fmt", ["json", "yaml"])
def test_legacy_to_v2_with_exact_numbers_gives_the_same_results(tmp_path, fmt):
    out = tmp_path / ("out." + fmt)
    convert.convert_file(LEGACY, out, fmt=fmt, exact=True)
    assert values(out) == values(LEGACY)


def test_legacy_to_v2_with_short_decimals_is_close(tmp_path):
    out = tmp_path / "out.json"
    convert.convert_file(LEGACY, out)
    (ga, ka), (gb, kb) = values(out), values(LEGACY)
    assert ga == pytest.approx(gb, rel=1e-5)
    assert ka == pytest.approx(kb, rel=1e-3, abs=1e-3)


@pytest.mark.parametrize("fmt", ["json", "yaml"])
def test_split_gives_a_dataset_folder(tmp_path, fmt):
    folder = tmp_path / "dataset"
    convert.convert_file(LEGACY, folder, split=True, fmt=fmt, exact=True)
    assert (folder / ("thermodataset." + fmt)).exists()
    assert len(list((folder / "substances").glob("*." + fmt))) == tf.Database(str(LEGACY)).numberOfSubstances()
    assert values(folder) == values(LEGACY)


def test_v2_to_legacy(tmp_path):
    source = ROOT / "pytests" / "data" / "v2" / "hkf.json"
    out = tmp_path / "legacy.json"
    convert.convert_file(source, out, to="legacy")
    assert sorted(tf.Database(str(out)).getSubstancesList()) == sorted(tf.Database(str(source)).getSubstancesList())


def test_the_legacy_format_is_one_json_file(tmp_path):
    with pytest.raises(ValueError):
        convert.convert_file(LEGACY, tmp_path / "x", to="legacy", split=True)


def test_the_report_has_its_sections(tmp_path):
    report = tmp_path / "report.md"
    convert.convert_file(ROOT / "Resources" / "databases" / "cemdata18-thermofun.json", tmp_path / "x.json", report=str(report))
    text = report.read_text()
    for heading in ["## Summary", "## Quirks applied", "## Unread legacy keys", "## Unresolved references", "## Methods used", "## Uncertainty convention notice"]:
        assert heading in text
    assert "LEG-QRK" in text and "CpPolynomial" in text


def test_the_command_line(tmp_path):
    out = tmp_path / "cli.json"
    assert convert.main([str(LEGACY), str(out)]) == 0
    assert json.loads(out.read_text())["thermohub"]


@pytest.mark.parametrize("path", sorted((ROOT / "Resources" / "databases").glob("*.json")), ids=lambda p: p.stem)
def test_the_v2_copy_of_each_bundled_database_gives_the_same_results(tmp_path, path):
    # what CI generates, in the form of a person (YAML, short decimals), and in exact form
    exact = tmp_path / "exact.yaml"
    convert.convert_file(path, exact, fmt="yaml", exact=True)
    legacy = tf.ThermoEngine(str(path))
    copy = tf.ThermoEngine(str(exact))
    db = legacy.database()
    assert sorted(copy.database().getSubstancesList()) == sorted(db.getSubstancesList())
    assert sorted(copy.database().getReactionsList()) == sorted(db.getReactionsList())
    for symbol in db.getSubstancesList():
        if sum(map(ord, symbol)) % 7 == 0:   # a fixed part, to stay fast
            a = legacy.thermoPropertiesSubstance(400.0, 1e7, symbol).gibbs_energy.val
            b = copy.thermoPropertiesSubstance(400.0, 1e7, symbol).gibbs_energy.val
            assert a == b or (math.isnan(a) and math.isnan(b)), symbol
    short = tmp_path / "short.yaml"
    convert.convert_file(path, short, fmt="yaml")
    assert tf.Database(str(short)).numberOfSubstances() == db.numberOfSubstances()


# --- the validator -----------------------------------------------------------------------------------------------------

def test_validate_gives_exit_code_0_for_good_data(capsys):
    assert validate.main([str(ROOT / "pytests" / "data" / "v2" / "minimal.json")]) == 0


def test_validate_gives_exit_code_1_for_an_error(tmp_path, capsys):
    bad = tmp_path / "bad.json"
    bad.write_text(json.dumps(dict(HEAD, substances={"X": {"formula": "Ca", "convention": {"energy_reference": "nonsense"}}})))
    assert validate.main([str(bad)]) == 1
    assert "CNV-005" in capsys.readouterr().out


# --- the snapshot ---------------------------------------------------------------------------------------------------------

def test_the_snapshot_names_the_data():
    s = tf.ThermoEngine(str(LEGACY)).snapshot()
    assert s.format_version == "legacy" and s.thermofun_version
    assert len(s.content_hash) == 64 and set(s.content_hash) <= set("0123456789abcdef")


def test_the_hash_is_stable_and_follows_the_records():
    a, b = tf.ThermoEngine(str(LEGACY)), tf.ThermoEngine(str(LEGACY))
    assert a.snapshot().content_hash == b.snapshot().content_hash
    substance = a.database().getSubstance("Quartz")
    substance.setName("Quartz changed")
    a.database().setSubstance(substance)
    a2 = tf.ThermoEngine(a.database())
    assert a2.snapshot().content_hash != b.snapshot().content_hash


def test_a_v2_file_gives_its_dataset_and_the_implementations():
    e = tf.ThermoEngine(str(ROOT / "pytests" / "data" / "v2" / "minimal.json"))
    p = tf.EnginePreferences()
    p.method_implementation = {"HKF88": "gems"}
    e.setPreferences(p)
    s = e.snapshot()
    assert s.format_version.startswith("2.") and s.dataset == "minimal" and s.implementations == {"HKF88": "gems"}


# --- the consistency report ---------------------------------------------------------------------------------------------

def species(g, h, s, cp=20.0):
    return {"formula": "Ca", "class": "component", "aggregate_state": "crystal", "standard_state_properties": {"G": g, "H": h, "S": s, "Cp": cp, "V": 1e-5},
            "methods": [{"method": "ConstantCp"}]}


def two_ways(shift=0.0, method="ConstantReactionProperties"):
    """C = A + B, with a reaction that gives G of C from G of A and B; the log K of the reaction is shifted by `shift`."""
    gA, gB, gC = -1000.0, -2000.0, -3500.0
    dG = gC - gA - gB - shift * R * T0 * math.log(10)
    c = species(gC, -4000.0, 30.0)
    c["defining_reaction"] = "R"
    props = {"dG": dG, "dH": -800.0, "dS": 5.0, "dCp": -20.0, "dV": 0.0, "logK": -dG / (R * T0 * math.log(10))}
    reactions = {"R": {"stoichiometry": {"C": 1, "A": -1, "B": -1}, "reaction_properties": props, "methods": [{"method": method}]}}
    return tf.ThermoEngine(tf.Database(json.dumps(dict(HEAD, substances={"A": species(gA, -1100.0, 10.0), "B": species(gB, -2100.0, 15.0), "C": c}, reactions=reactions))))


def item(items, kind):
    return next(i for i in items if i.symbol in ("C", "R") and i.kind == kind)


def test_a_substance_that_its_reaction_reproduces_is_consistent():
    c = item(two_ways().consistencyReport([(T0, P0)]), "substance_vs_reaction")
    assert c.flag == "consistent" and abs(c.delta_G) < 1e-6


def test_a_shifted_log_k_is_inconsistent_by_rt_ln10():
    items = two_ways(shift=0.1).consistencyReport([(T0, P0)])
    c = item(items, "substance_vs_reaction")
    assert c.flag == "inconsistent"
    assert abs(c.delta_G) == pytest.approx(R * T0 * math.log(10) * 0.1, rel=1e-6)
    r = item(items, "reaction_vs_reactants")
    assert r.flag == "inconsistent" and abs(r.delta_logK) == pytest.approx(0.1, rel=1e-3)


def test_the_threshold_is_an_option():
    options = tf.ConsistencyOptions()
    options.delta_G_threshold = 1e6
    assert item(two_ways(shift=0.1).consistencyReport([(T0, P0)], options), "substance_vs_reaction").flag == "consistent"


def test_a_failed_calculation_stays_in_the_report():
    bad = species(-1.0, -1.0, 1.0)
    db = dict(HEAD, substances={"A": bad, "C": dict(species(-1.0, -1.0, 1.0), defining_reaction="R")},
              reactions={"R": {"stoichiometry": {"C": 1, "A": -1, "Missing": -1}, "reaction_properties": {"dG": 0.0, "logK": 0.0}, "methods": [{"method": "ConstantReactionProperties"}]}})
    items = tf.ThermoEngine(tf.Database(json.dumps(db))).consistencyReport([(T0, P0)])
    assert any(i.flag == "not calculated" and i.message for i in items)


def test_the_command_of_the_report(capsys):
    assert consistency.main([str(LEGACY), "--TP", "298.15,1e5"]) == 0
    assert "| Record | Kind |" in capsys.readouterr().out


# --- the generated pages --------------------------------------------------------------------------------------------------

def test_the_committed_pages_are_up_to_date():
    # run `python -m thermofun.docs generate docs/reference` after a change of the catalog
    assert docs.check(str(REFERENCE), DATA) == []


def test_a_page_for_each_method_with_what_10_section_2_asks():
    pages = docs.pages(DATA)
    catalog = tf.methods.catalog()
    for s in catalog:
        assert "methods/%s.md" % s["id"] in pages
    assert {"methods/index.md", "diagnostics.md", "engine-preferences.md"} <= set(pages)
    assert "Use another only to reproduce" in pages["methods/HKF88.md"]
    assert "solute_hkf88_gems" in pages["methods/HKF88.md"]
    assert "units: {energy: cal, pressure: bar}" in pages["methods/HKF88.md"]
    assert "a1: 0.2494" in pages["methods/HKF88.md"]
    assert "energy_reference" in pages["engine-preferences.md"] and "gibbs_helmholtz_formation" in pages["engine-preferences.md"]


def test_the_examples_are_read_by_the_reader():
    found = docs.examples(DATA)
    checked = 0
    for spec in tf.methods.catalog():
        key = docs._key(spec)
        if key not in found or not spec["implemented"]:
            continue
        lines = docs.example_lines(spec, found[key])
        body = "\n".join("      " + line for line in lines)
        if key[0] == "substances":
            text = "thermohub: 2.0.0-draft.1\nsubstances:\n  X:\n    formula: Ca\n    methods:\n" + body + "\n"
        else:
            text = "thermohub: 2.0.0-draft.1\nreactions:\n  X:\n    equation: A = B\n    methods:\n" + body + "\n"
        # the record has only this method, so it can be refused for what it lacks; never for its parameters or units
        try:
            problems = " ".join(d.code for d in tf.Database(text).loadDiagnostics())
        except RuntimeError as error:
            problems = str(error)
        for code in ("V2-UNIT", "V2-MTH-002", "V2-MTH-003", "V2-MTH-004"):
            assert code not in problems, (spec["id"], text)
        checked += 1
    assert checked >= 10
