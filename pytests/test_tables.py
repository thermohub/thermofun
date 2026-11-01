# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# Tables from a setup, with the same numbers as ThermoBatch, and each thing that Output and BatchPreferences can do.

import csv
import json
import math
import pathlib

import pytest
import thermofun as tf

ROOT = pathlib.Path(__file__).resolve().parent.parent
AQ = str(ROOT / "Resources" / "databases" / "aq17-thermofun.json")
POINTS = [[25.0, 1.0], [100.0, 100.0]]


def setup(**more):
    return {"database": AQ, "conditions": {"T": {"values": [25.0, 100.0], "unit": "degC"}, "P": {"values": [1.0, 100.0], "unit": "bar"}, "pairs": True},
            "tables": [{"name": "t", "records": ["Ca+2", "Calcite"], "columns": [{"property": "G", "unit": "kJ/mol", "digits": 3}, {"property": "S", "digits": 2}], **more}]}


def batch():
    b = tf.ThermoBatch(tf.ThermoEngine(AQ))
    b.setPropertiesUnits(["temperature", "pressure"], ["degC", "bar"])
    b.setPropertyUnitDigit("gibbs_energy", "kJ/mol", 3)
    b.setPropertyDigit("entropy", 2)
    return b


def rows_of(path):
    with open(path) as f:
        return list(csv.reader(f))


def test_the_numbers_are_those_of_toCSV(tmp_path):
    batch().thermoPropertiesSubstance(POINTS, ["Ca+2", "Calcite"], ["gibbs_energy", "entropy"]).toCSV(str(tmp_path / "old.csv"))
    table = tf.table(setup())["t"]
    table.write(str(tmp_path / "new.csv"))
    old, new = rows_of(tmp_path / "old.csv"), rows_of(tmp_path / "new.csv")
    assert len(old) == len(new)
    for a, b in zip(old[1:], new[1:]):
        assert a[0] == b[0]
        assert [float(x) for x in a[1:]] == pytest.approx([float(x) for x in b[1:]], abs=1e-9)
        assert a[3:] == b[3:]       # the same text for the same digits


def test_to2DVectorDouble_toDouble_toThermoScalar():
    table = tf.table(setup())["t"]
    assert len(table.values()) == 4
    engine = tf.ThermoEngine(AQ)
    direct = engine.thermoPropertiesSubstance(298.15, 1e5, "Ca+2").gibbs_energy
    assert table.value("Ca+2", 25.0, 1.0, "G") == pytest.approx(direct.val / 1000.0, rel=1e-12)
    assert table.values()[0][0] == pytest.approx(direct.val / 1000.0, rel=1e-12)   # unlike the list of Output, in your unit
    scalar = table.scalar("Ca+2", 25.0, 1.0, "G")
    assert scalar.val == direct.val and scalar.ddt == direct.ddt


def test_toCSVTransposed():
    table = tf.table(setup(layout="transposed"))["t"]
    lines = table.grid()
    assert lines[0][:3] == ["Property", "T(degC)", "P(bar)"] and lines[0][3:] == ["Ca+2", "Calcite"]
    assert len(lines) == 1 + 2 * 2                          # each column at each T, P
    assert float(lines[1][3]) == pytest.approx(table.value("Ca+2", 25.0, 1.0, "G"), abs=1e-3)


def test_toCSVPropertyGrid_in_both_directions():
    spec = {"database": AQ, "conditions": {"T": {"values": [25.0, 100.0], "unit": "degC"}, "P": {"values": [1000.0, 2000.0, 3000.0], "unit": "bar"}},
            "tables": [{"name": "t", "records": ["Ca+2"], "columns": [{"property": "G", "digits": 1}], "layout": "grid"}]}
    label, lines = tf.table(spec)["t"].property_grids()[0]
    assert lines[0][1] == "T(degC)_P(bar)" and len(lines[0]) == 2 + 3 and len(lines) == 1 + 2
    spec["tables"][0]["across"] = "T"
    _, across = tf.table(spec)["t"].property_grids()[0]
    assert across[0][1] == "P(bar)_T(degC)" and len(across[0]) == 2 + 2 and len(across) == 1 + 3
    assert lines[1][2] == across[1][2]                       # T = 25, P = 1000 in both


@pytest.mark.parametrize("settings,text", [({"format": "scientific", "digits": 3}, "e+"), ({"format": "fixed", "digits": 1}, "."), ({"separator": ";"}, ";")])
def test_number_format_and_separator(settings, text):
    spec = setup(**settings)
    for column in spec["tables"][0]["columns"]:
        column.pop("digits", None)
    assert text in tf.table(spec)["t"].to_text("csv").splitlines()[1]


def test_the_order_of_the_loops():
    by_record = [r[0] for r in tf.table(setup())["t"].grid()[1:]]
    by_condition = [(r[0], r[1]) for r in tf.table(setup(loop="conditions"))["t"].grid()[1:]]
    assert by_record == ["Ca+2", "Ca+2", "Calcite", "Calcite"]
    assert [c[1] for c in by_condition[:2]] == ["25", "25"]
    spec = setup()
    spec["conditions"] = {"T": {"values": [25.0, 100.0], "unit": "degC"}, "P": {"values": [1.0, 100.0], "unit": "bar"}, "order": "P_first"}
    points = [(row[1], row[2]) for row in tf.table(spec)["t"].rows[:2]]
    assert points == [(25.0, 1.0), (100.0, 1.0)]


def test_the_properties_of_the_solvent_are_a_table():
    spec = setup(records=["H2O@"], columns=[{"property": "density", "digits": 3}, {"property": "epsilon", "digits": 3}, "bornQ"])
    table = tf.table(spec)["t"]
    engine = tf.ThermoEngine(AQ)
    assert table.value("H2O@", 25.0, 1.0, "density") == pytest.approx(engine.propertiesSolvent(298.15, 1e5, "H2O@").density.val)
    assert table.value("H2O@", 25.0, 1.0, "epsilon") == pytest.approx(engine.electroPropertiesSolvent(298.15, 1e5, "H2O@").epsilon.val)


def test_reactions_from_their_reactants_and_substances_from_their_reactions():
    equation = "Calcite = Ca+2 + CO3-2"
    spec = setup(records=[], reactions=[equation], columns=["logK"], source="reactants")
    got = tf.table(spec)["t"].value(equation, 25.0, 1.0, "logK")
    assert got == pytest.approx(tf.ThermoEngine(AQ).thermoPropertiesReactionFromReactants(298.15, 1e5, equation).log_equilibrium_constant.val)


def test_a_value_that_is_not_defined():
    doc = {"thermohub": "2.0.0-draft.1", "substances": {"X": {"formula": "Ca", "class": "component", "standard_state_properties": {"G": -1000.0}}}}
    spec = {"database": tf.Database(json.dumps(doc)), "conditions": {"T": [298.15], "P": [1e5]}, "tables": [{"name": "t", "records": ["X"], "columns": ["V"], "not_defined": "n/a"}]}
    table = tf.table(spec)["t"]
    assert table.grid()[1][3] == "n/a"
    assert math.isnan(table.values()[0][0])
    spec["tables"][0]["write_not_defined"] = False
    assert tf.table(spec)["t"].grid()[1][3] != "n/a"


def test_err_derivatives_and_status():
    spec = setup(records=["Ca+2"], columns=[{"property": "G", "show": "ddt"}, {"property": "G", "show": "ddp"}, {"property": "G", "show": "err", "unit": "kJ/mol"}, {"property": "G", "show": "status"}])
    table = tf.table(spec)["t"]
    direct = tf.ThermoEngine(AQ).thermoPropertiesSubstance(298.15, 1e5, "Ca+2").gibbs_energy
    cells = table.rows[0][3]
    assert cells[0].value == direct.ddt and cells[1].value == direct.ddp
    assert cells[2].value == pytest.approx(direct.err / 1000.0)
    assert cells[3].value == "calculated"


def test_units_of_each_kind():
    spec = setup(records=["Calcite"], columns=[{"property": "V", "unit": "cm3/mol", "digits": 3}, {"property": "S", "unit": "J/(mol K)"}, {"property": "H", "unit": "kcal/mol"}])
    table = tf.table(spec)["t"]
    engine = tf.ThermoEngine(AQ).thermoPropertiesSubstance(298.15, 1e5, "Calcite")
    assert table.value("Calcite", 25.0, 1.0, "V") == pytest.approx(engine.volume.val * 10.0, rel=1e-9)
    assert table.value("Calcite", 25.0, 1.0, "H") == pytest.approx(engine.enthalpy.val / 4184.0, rel=1e-9)


def test_a_bad_record_is_a_problem_and_the_rest_runs():
    spec = setup(records=["Ca+2", "NoSuch"])
    results = tf.table(spec)
    assert len(results["t"].rows) == 2
    assert any("NoSuch" in str(p) for p in results.problems)


@pytest.mark.parametrize("bad,text", [({"columns": []}, "no columns"), ({"records": []}, "no records"), ({"columns": ["Nope"]}, "no property Nope"), ({"layout": "x"}, "layout")])
def test_a_wrong_setup_says_why(bad, text):
    with pytest.raises(tf.tables.SetupError, match=text):
        table = tf.table(setup(**bad))["t"]
        table.to_text()


def test_markdown_and_latex(tmp_path):
    table = tf.table(setup())["t"]
    md, tex = table.write(str(tmp_path / "a.md")), table.write(str(tmp_path / "a.tex"))
    assert open(md).read().startswith("| Symbol |")
    text = open(tex).read()
    assert "\\begin{tabular}" in text and "\\end{tabular}" in text


def test_a_yaml_setup_with_a_database_next_to_it(tmp_path):
    (tmp_path / "db.json").write_text(pathlib.Path(AQ).read_text())
    (tmp_path / "setup.yaml").write_text("""
database: db.json
conditions:
  T: {from: 25, to: 75, step: 25, unit: degC}
  P: {values: [saturation], unit: bar}
tables:
  - name: species
    records: [Ca+2]
    columns:
      - {property: G, unit: kJ/mol, digits: 3}
    output: out.csv
""")
    results = tf.table(str(tmp_path / "setup.yaml"))
    assert [r[1] for r in results["species"].rows] == [25, 50, 75]
    assert results["species"].rows[0][2] == "saturation"
    assert results["species"].grid()[1][2] == "saturation"


def test_the_command_writes_the_files(tmp_path, monkeypatch):
    (tmp_path / "setup.yaml").write_text(f"database: {AQ}\nconditions:\n  T: [298.15]\n  P: [100000.0]\ntables:\n  - name: s\n    records: [Ca+2]\n    columns: [G]\n    output: out.md\n")
    monkeypatch.chdir(tmp_path)
    assert tf.tables.main(["setup.yaml"]) == 0
    assert (tmp_path / "out.md").read_text().startswith("| Symbol")


def test_each_template_makes_its_tables():
    names = tf.tables.templates()
    assert {"species-properties", "reaction-properties", "logk-grid", "water-properties", "uncertainties", "derivatives", "model-comparison"} <= set(names)
    records = {"species-properties": {"records": ["Ca+2", "Calcite"]}, "reaction-properties": {"reactions": ["Calcite = Ca+2 + CO3-2"]}, "logk-grid": {"reactions": ["Calcite = Ca+2 + CO3-2"]},
               "water-properties": {}, "uncertainties": {"records": ["Ca+2"]}, "derivatives": {"records": ["Ca+2"]}, "model-comparison": {"records": ["Na+"]}}
    for name, given in records.items():
        results = tf.table(template=name, database=AQ, **given)
        for table in results.values():
            assert table.rows and table.to_text("csv"), name
            assert not results.problems, (name, results.problems[:2])


def test_a_template_needs_your_records():
    with pytest.raises(tf.tables.SetupError, match="no records"):
        tf.table(template="species-properties", database=AQ)


def test_settings_of_a_template_from_python(tmp_path):
    made = tf.tables.template("species-properties", database=AQ, records=["Ca+2"])
    made.column("G", unit="J/mol", digits=1)
    made.remove_column("V")
    made.add_column("A", unit="kJ/mol", digits=2)
    made.set(T={"values": [25.0, 50.0], "unit": "degC"}, layout="transposed", output=str(tmp_path / "x.csv"))
    results = made.run()
    table = results["species"]
    assert table.columns[0]["label"] == "G(J/mol)" and [c["property"] for c in table.columns][-1] == "A"
    assert table.value("Ca+2", 25.0, "saturation", "G") == pytest.approx(tf.ThermoEngine(AQ).thermoPropertiesSubstance(298.15, 0.0, "Ca+2").gibbs_energy.val)
    assert results.write() == [str(tmp_path / "x.csv")]


def test_a_changed_template_can_be_kept_and_read_back(tmp_path):
    made = tf.tables.template("reaction-properties", database=AQ, reactions=["Calcite = Ca+2 + CO3-2"])
    made.column("logK", digits=5)
    path = made.save(str(tmp_path / "mine.yaml"))
    again = tf.tables.load(path)
    assert again.run()["reactions"].to_text("csv") == made.run()["reactions"].to_text("csv")


def test_compare_two_dielectric_models():
    results = tf.table(template="model-comparison", database=AQ, records=["Na+"], P={"values": [1.0], "unit": "bar"}, T={"values": [25.0], "unit": "degC"})
    table = results["comparison"]
    assert [c["label"] for c in table.columns] == ["V(J/bar) [JN91]", "Cp(J/K/mol) [JN91]", "V(J/bar) [HK74]", "Cp(J/K/mol) [HK74]"]
    cells = table.rows[0][3]
    assert cells[0].value != cells[2].value


def test_compare_options_of_the_engine():
    spec = setup(records=["Ca+2"], columns=[{"property": "G"}], compare=[{"label": "a"}, {"label": "b", "options": {"round_to_uncertainty": True}}])
    assert len(tf.table(spec)["t"].columns) == 2


def test_an_engine_or_a_database_can_be_given():
    engine = tf.ThermoEngine(AQ)
    spec = setup()
    del spec["database"]
    assert tf.table(spec, database=engine)["t"].rows
    assert tf.table(spec, database=tf.Database(AQ))["t"].rows
