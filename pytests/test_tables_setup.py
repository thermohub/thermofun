# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# Rules that choose records, templates of your own, the record of how a result was made, and a setup as a ThermoBatch.

import csv
import pathlib

import pytest
import thermofun as tf

ROOT = pathlib.Path(__file__).resolve().parent.parent
AQ = str(ROOT / "Resources" / "databases" / "aq17-thermofun.json")


def setup(**more):
    return {"database": AQ, "conditions": {"T": {"values": [25.0, 100.0], "unit": "degC"}, "P": {"values": [1.0, 100.0], "unit": "bar"}, "pairs": True},
            "tables": [{"name": "t", "records": ["Ca+2", "Calcite"], "columns": [{"property": "G", "unit": "kJ/mol", "digits": 3}, {"property": "S", "digits": 2}], **more}]}


# --- rules ----------------------------------------------------------------------------------------------------------------

def test_a_rule_chooses_by_class_elements_and_charge():
    database = tf.Database(AQ)
    substances = {s: database.getSubstance(s) for s in database.getSubstancesList()}
    aqueous = tf.tables.select_substances(database, {"class": "aqueous_solute"})
    assert aqueous == sorted(s for s, x in substances.items() if x.substanceClass() == tf.SubstanceClass.AQSOLUTE)
    assert "Calcite" not in aqueous

    with_ca = tf.tables.select_substances(database, {"elements": ["Ca"]})
    assert "Ca+2" in with_ca and "Calcite" in with_ca and "Na+" not in with_ca
    assert all("Ca" in substances[s].formula() for s in with_ca)
    assert "Calcite" in tf.tables.select_substances(database, {"elements": ["Ca", "C"]})
    assert "Ca+2" not in tf.tables.select_substances(database, {"elements": ["Ca", "C"]})

    only = tf.tables.select_substances(database, {"only_elements": ["Ca", "C", "O"]})
    assert "Calcite" in only and "Ca+2" in only and "CaOH+" not in only      # the charge is not an element

    assert tf.tables.select_substances(database, {"charge": 2, "elements": ["Ca"]}) == [s for s in with_ca if substances[s].charge() == 2]
    assert tf.tables.select_substances(database, "{match: 'Ca*', exclude: [Calcite, 'CaC*']}") == [s for s in sorted(substances) if s.startswith("Ca") and s != "Calcite" and not s.startswith("CaC")]


def test_a_rule_chooses_reactions():
    database = tf.Database(str(ROOT / "pytests" / "test-aq17-gem-lma-thermofun.json"))
    names = database.getReactionsList()
    if not names:
        pytest.skip("no reactions in this file")
    first = database.getReaction(names[0])
    species = list(first.reactants())[0]
    chosen = tf.tables.select_reactions(database, {"involving": [species]})
    assert names[0] in chosen
    assert all(species in database.getReaction(r).reactants() for r in chosen)


def test_a_table_takes_a_rule_a_list_or_both():
    rule = {"class": "aqueous_solute", "elements": ["Ca"]}
    table = tf.table(setup(records=rule))["t"]
    expected = tf.tables.select_substances(tf.Database(AQ), rule)
    assert sorted({r[0] for r in table.rows}) == expected

    mixed = tf.table(setup(records=["Calcite", rule, "Ca+2"]))["t"]
    assert [r[0] for r in mixed.rows[::2]] == ["Calcite"] + expected          # a symbol once, in the order given

    assert table.value("Ca+2", 25.0, 1.0, "G") == tf.table(setup())["t"].value("Ca+2", 25.0, 1.0, "G")
    saved = tf.tables.load(setup(records=rule))
    assert "class: aqueous_solute" in saved.to_yaml()


@pytest.mark.parametrize("rule, words", [
    ({"class": "aqueous_solut"}, "Did you mean \"aqueous_solute\""),
    ({"classs": "aqueous_solute"}, "Did you mean class"),
    ({"elements": ["Xx"]}, "chooses no substance"),
    ({}, "is empty"),
])
def test_a_wrong_rule_says_why(rule, words):
    with pytest.raises(tf.tables.SetupError, match=words):
        tf.table(setup(records=rule))


def test_a_misspelled_key_says_why():
    with pytest.raises(tf.tables.SetupError, match="Did you mean layout"):
        tf.table(setup(layuot="grid"))
    with pytest.raises(tf.tables.SetupError, match="Did you mean unit"):
        tf.table({**setup(), "tables": [{"name": "t", "records": ["Ca+2"], "columns": [{"property": "G", "units": "kJ/mol"}]}]})


# --- templates --------------------------------------------------------------------------------------------------------------

def test_a_template_of_your_own(tmp_path, monkeypatch):
    (tmp_path / "my-table.yaml").write_text("# My own table\nconditions:\n  T: {values: [25], unit: degC}\n  P: {values: [1], unit: bar}\ntables:\n  - name: mine\n    columns: [G]\n")
    (tmp_path / "species-properties.yaml").write_text("# Mine replaces it\nconditions:\n  T: {values: [50], unit: degC}\n  P: {values: [1], unit: bar}\ntables:\n  - name: species\n    columns: [S]\n")
    assert "my-table" not in tf.tables.templates()
    monkeypatch.setenv("THERMOFUN_TEMPLATE_PATH", str(tmp_path))
    assert tf.tables.template_folders() == [str(tmp_path)]
    assert tf.tables.templates()["my-table"] == "My own table"
    assert tf.tables.template_source("my-table") == str(tmp_path / "my-table.yaml")
    assert tf.tables.template_source("logk-grid") == "library"
    results = tf.table(template="my-table", database=AQ, records=["Ca+2"])
    assert results["mine"].rows[0][0] == "Ca+2"
    assert any(str(tmp_path) in t for t in results["mine"].provenance.templates)
    assert tf.table(template="species-properties", database=AQ, records=["Ca+2"])["species"].columns[0]["property"] == "S"


def test_a_setup_names_templates_and_changes_them(tmp_path):
    file = tmp_path / "s.yaml"
    file.write_text(f"template: [species-properties, batch-reference, output-publication]\ndatabase: {AQ}\nrecords: [Ca+2]\ntables:\n  - name: species\n    layout: transposed\n  - name: extra\n    columns: [V]\n")
    results = tf.table(str(file))
    species = results["species"]
    assert [r[1:3] for r in species.rows] == [(25.0, 1.0)]
    assert species.columns[0]["unit"] == "kJ/mol" and species.grid()[0][0] == "Property"
    assert "extra" in results and results["extra"].rows[0][0] == "Ca+2"
    assert [t for t in species.provenance.templates] == ["species-properties", "batch-reference", "output-publication"]


def test_a_template_that_uses_itself_says_so(tmp_path, monkeypatch):
    (tmp_path / "a.yaml").write_text("template: b\n")
    (tmp_path / "b.yaml").write_text("template: a\n")
    monkeypatch.setenv("THERMOFUN_TEMPLATE_PATH", str(tmp_path))
    with pytest.raises(tf.tables.SetupError, match="uses itself"):
        tf.tables.template("a")


def test_saved_setup_gives_the_same_table_without_the_templates(tmp_path):
    made = tf.tables.template(["species-properties", "batch-reference"], database=AQ, records={"elements": ["Ca"], "class": "aqueous_solute"})
    path = made.save(str(tmp_path / "saved.yaml"))
    assert "template:" not in pathlib.Path(path).read_text().replace("from_templates:", "")
    assert tf.table(path)["species"].to_text() == made.run()["species"].to_text()


def test_units_and_digits_for_each_property():
    made = setup()
    made["tables"][0]["columns"] = ["G", "S"]
    made["units"] = {"G": "kJ/mol"}
    made["digits"] = {"G": 1, "entropy": 3}
    table = tf.table(made)["t"]
    assert table.columns[0]["unit"] == "kJ/mol"
    line = table.grid()[1]
    assert len(line[3].split(".")[1]) == 1 and len(line[4].split(".")[1]) == 3


# --- how it was made -----------------------------------------------------------------------------------------------------------

def test_the_top_of_a_file_says_how_it_was_made(tmp_path):
    table = tf.table(setup())["t"]
    lines = table.provenance.lines()
    assert lines[0] == "database: " + AQ
    joined = "\n".join(lines)
    assert "ThermoFun " in joined and "physical constants " in joined and "sha256 " in joined and "water: H2O@:" in joined and "conventions: " in joined
    snapshot = table.provenance.calculations[0][1]
    assert snapshot.physical_constants == tf.methods.physicalConstants()["name"]
    assert snapshot.water_methods and snapshot.content_hash == tf.ThermoEngine(AQ).snapshot().content_hash

    for name, start, end in [("a.csv", "# database", None), ("a.md", "<!--", "-->"), ("a.tex", "% database", None)]:
        text = pathlib.Path(table.write(str(tmp_path / name))).read_text()
        assert text.startswith(start)
        if end:
            assert end in text
    # the numbers are as without the lines
    rows = list(csv.reader(line for line in open(tmp_path / "a.csv") if not line.startswith("#")))
    assert rows == list(csv.reader(table.to_text().splitlines()))
    assert table.to_text("csv", with_provenance=True).startswith("# database")


def test_a_file_is_the_same_when_made_again(tmp_path):
    a, b = tmp_path / "a.csv", tmp_path / "b.csv"
    tf.table(setup())["t"].write(str(a))
    tf.table(setup())["t"].write(str(b))
    assert a.read_text() == b.read_text()      # no date in it: a change in the file is a change in the results or the data


def test_the_record_can_be_a_separate_file_or_nothing(tmp_path):
    path = tf.table(setup(provenance="file"))["t"].write(str(tmp_path / "f.csv"))
    assert pathlib.Path(path).read_text().startswith("Symbol,")
    side = (tmp_path / "f.csv.provenance.yaml").read_text()
    assert "sha256: " in side and "setup:" in side and "columns:" in side

    path = tf.table(setup(provenance="none"))["t"].write(str(tmp_path / "n.csv"))
    assert pathlib.Path(path).read_text().startswith("Symbol,")
    assert not (tmp_path / "n.csv.provenance.yaml").exists()

    with pytest.raises(tf.tables.SetupError, match="header, file or none"):
        tf.table(setup(provenance="maybe"))


def test_the_record_names_each_engine_of_a_comparison_and_the_options():
    made = setup(options={"round_to_uncertainty": True})
    lines = tf.table(made)["t"].provenance.lines()
    assert any(line.startswith("options:") and "round_to_uncertainty = true" in line for line in lines)
    two = setup(compare=[{"label": "A"}, {"label": "B", "options": {"fallback_to_reference_properties": True}}])
    lines = tf.table(two)["t"].provenance.lines()
    assert any(line.startswith("[A] ThermoFun") for line in lines) and any(line.startswith("[B] options:") for line in lines)


# --- a setup as a batch ----------------------------------------------------------------------------------------------------------

def batch_by_hand():
    b = tf.ThermoBatch(tf.ThermoEngine(AQ))
    b.setPropertiesUnits(["temperature", "pressure"], ["degC", "bar"])
    b.setPropertyUnitDigit("gibbs_energy", "kJ/mol", 3)
    b.setPropertyDigit("entropy", 2)
    return b


def test_a_setup_makes_the_batch_that_you_would_write(tmp_path):
    made = tf.tables.load(setup(provenance="none"))
    calculation = made.batch()
    assert calculation.symbols == ["Ca+2", "Calcite"] and calculation.properties == ["gibbs_energy", "entropy"] and not calculation.for_reactions
    assert calculation.points == [[25.0, 1.0], [100.0, 100.0]]
    calculation.run().toCSV(str(tmp_path / "setup.csv"))
    batch_by_hand().thermoPropertiesSubstance([[25.0, 1.0], [100.0, 100.0]], ["Ca+2", "Calcite"], ["gibbs_energy", "entropy"]).toCSV(str(tmp_path / "hand.csv"))
    assert (tmp_path / "setup.csv").read_text() == (tmp_path / "hand.csv").read_text()


def test_a_batch_has_the_numbers_of_the_table(tmp_path):
    made = tf.tables.load(setup(provenance="none"))
    assert made.batch().run().to2DVectorDouble()[0][0] == pytest.approx(tf.ThermoEngine(AQ).thermoPropertiesSubstance(298.15, 1e5, "Ca+2").gibbs_energy.val, rel=1e-12)
    rows = list(csv.reader(open(made.batch().write(str(tmp_path / "b.csv")))))
    table = list(csv.reader(made.run()["t"].to_text().splitlines()))
    assert len(rows) == len(table)
    for a, b in zip(rows[1:], table[1:]):
        assert a[0] == b[0] and [float(x) for x in a[1:]] == pytest.approx([float(x) for x in b[1:]], abs=1e-9)


def test_a_batch_takes_a_rule_a_template_and_the_layout(tmp_path):
    made = tf.tables.template(["species-properties", "batch-saturation"], database=AQ, records={"class": "aqueous_solute", "elements": ["Ca"]}, provenance="none")
    calculation = made.batch("species")
    assert "Ca+2" in calculation.symbols and all(p[1] == 0.0 for p in calculation.points)
    values = calculation.run().to2DVectorDouble()
    assert len(values) == len(calculation.symbols) * len(calculation.points)
    # at the saturation pressure, as in the engine
    assert values[0][0] == pytest.approx(tf.ThermoEngine(AQ).thermoPropertiesSubstance(298.15, 0.0, calculation.symbols[0]).gibbs_energy.val, rel=1e-12)

    made.set(layout="transposed")
    rows = pathlib.Path(made.batch("species").write(str(tmp_path / "t.csv"))).read_text().splitlines()
    assert rows[0].startswith("Property,")


def test_a_batch_of_reactions():
    made = tf.tables.template(["reaction-properties", "batch-reference"], database=AQ, reactions=["Calcite = Ca+2 + CO3-2"], provenance="none")
    made.set(columns=["logK", "dG"])
    calculation = made.batch()
    assert calculation.for_reactions and calculation.properties == ["logKr", "reaction_gibbs_energy"]


def test_a_batch_writes_how_it_was_made(tmp_path):
    made = tf.tables.template(["species-properties", "batch-reference"], database=AQ, records=["Ca+2"])
    calculation = made.batch()
    path = calculation.write(str(tmp_path / "p.csv"))
    text = pathlib.Path(path).read_text()
    assert text.startswith("# database: " + AQ) and "# templates: species-properties, batch-reference" in text and "sha256 " in text
    assert calculation.provenance.calculations[0][1].content_hash in text

    # a batch of your own: off by default, as before
    b = tf.ThermoBatch(AQ)
    out = b.thermoPropertiesSubstance([[298.15, 1e5]], ["Ca+2"], ["gibbs_energy"])
    out.toCSV(str(tmp_path / "o.csv"))
    assert (tmp_path / "o.csv").read_text().startswith("Symbol,")
    assert any("sha256" in line for line in out.provenance())
    b.preferences.writeProvenance = True
    b.thermoPropertiesSubstance([[298.15, 1e5]], ["Ca+2"], ["gibbs_energy"]).toCSV(str(tmp_path / "o2.csv"))
    assert (tmp_path / "o2.csv").read_text().startswith("# ThermoFun ")


def test_the_settings_go_on_a_batch_of_your_own():
    b = tf.ThermoBatch(AQ)
    tf.tables.load(setup(separator=";", format="scientific")).apply_to(b)
    assert b.preferences.separator == ";" and b.preferences.isScientific and not b.preferences.isFixed
    assert b.propertyUnits()["gibbs_energy"] == "kJ/mol" and b.propertyUnits()["temperature"] == "degC"
    assert b.propertyDigits()["entropy"] == 2


def test_a_batch_rule_on_a_batch_of_your_own():
    b = tf.ThermoBatch(AQ)
    symbols = b.selectSubstances({"elements": ["Ca"], "class": "aqueous_solute"})
    assert "Ca+2" in symbols
    assert b.selectSubstances("{elements: [Ca], class: aqueous_solute}") == symbols


@pytest.mark.parametrize("more, words", [
    ({"compare": [{"label": "a"}]}, "compare"),
    ({"columns": [{"property": "G", "show": "err"}]}, "needs a table"),
    ({"columns": ["density"]}, "use a table"),
    ({"reactions": ["Calcite = Ca+2 + CO3-2"]}, "records and reactions"),
])
def test_what_a_batch_cannot_do_it_says(more, words):
    made = tf.tables.load(setup(**more))
    with pytest.raises(tf.tables.SetupError, match=words):
        made.batch()


def test_a_setup_of_two_tables_asks_which():
    made = setup()
    made["tables"].append({"name": "u", "records": ["Na+"], "columns": ["G"]})
    with pytest.raises(tf.tables.SetupError, match="Name one table"):
        tf.tables.load(made).batch()
    assert tf.tables.load(made).batch("u").symbols == ["Na+"]


@pytest.mark.parametrize("across", ["P", "T"])
def test_a_batch_grid_has_the_axis_of_the_setup(tmp_path, monkeypatch, across):
    made = tf.tables.load({"database": AQ, "conditions": {"T": {"values": [25.0, 50.0, 75.0], "unit": "degC"}, "P": {"values": [1.0, 100.0], "unit": "bar"}},
                           "tables": [{"name": "t", "records": ["Ca+2"], "columns": [{"property": "G", "digits": 3}], "layout": "grid", "across": across, "provenance": "none"}]})
    monkeypatch.chdir(tmp_path)
    made.batch().write("g.csv")
    rows = list(csv.reader(open(tmp_path / "gibbs_energy_g.csv")))
    assert len(rows[0]) - 2 == (3 if across == "T" else 2)
    assert len(rows) - 1 == (2 if across == "T" else 3)
    table = made.run()["t"].property_grids()[0][1]
    assert [len(table[0]), len(table)] == [len(rows[0]), len(rows)]
    assert [float(x) for x in rows[1][2:]] == pytest.approx([float(x) for x in table[1][2:]], abs=1e-9)
