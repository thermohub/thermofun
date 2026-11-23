# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# Batch calculations (decision 0032): the same numbers as ThermoBatch, each thing that Output and BatchPreferences can do,
# the four blocks of a setup, the templates and the comparison of models and databases.

import csv
import json
import math
import pathlib

import pytest
import thermofun as tf

ROOT = pathlib.Path(__file__).resolve().parent.parent
AQ = str(ROOT / "Resources" / "databases" / "aq17-thermofun.json")
POINTS = [[25.0, 1.0], [100.0, 100.0]]


def setup(output=None, engine=None, conditions=None, **result):
    made = {"database": AQ, "conditions": {"T": {"values": [25.0, 100.0], "unit": "degC"}, "P": {"values": [1.0, 100.0], "unit": "bar"}, "points": "pairs"},
            "results": [{"name": "t", "substances": ["Ca+2", "Calcite"], "properties": [{"name": "G", "unit": "kJ/mol", "digits": 3}, {"name": "S", "digits": 2}], **result}]}
    if output:
        made["output"] = output
    if engine:
        made["engine"] = engine
    if conditions:
        made["conditions"].update(conditions)
    return made


def batch():
    b = tf.ThermoBatch(tf.ThermoEngine(AQ))
    b.setPropertiesUnits(["temperature", "pressure"], ["degC", "bar"])
    b.setPropertyUnitDigit("gibbs_energy", "kJ/mol", 3)
    b.setPropertyDigit("entropy", 2)
    return b


def rows_of(path):
    with open(path) as f:
        return list(csv.reader(line for line in f if not line.startswith("#")))


def test_the_numbers_are_those_of_toCSV(tmp_path):
    batch().thermoPropertiesSubstance(POINTS, ["Ca+2", "Calcite"], ["gibbs_energy", "entropy"]).toCSV(str(tmp_path / "old.csv"))
    table = tf.batch(setup())["t"]
    table.write(str(tmp_path / "new.csv"))
    old, new = rows_of(tmp_path / "old.csv"), rows_of(tmp_path / "new.csv")
    assert len(old) == len(new)
    for a, b in zip(old[1:], new[1:]):
        assert a[0] == b[0]
        assert [float(x) for x in a[1:]] == pytest.approx([float(x) for x in b[1:]], abs=1e-9)
        assert a[3:] == b[3:]       # the same text for the same digits


def test_to2DVectorDouble_toDouble_toThermoScalar():
    table = tf.batch(setup())["t"]
    assert isinstance(table, tf.ResultTable)
    assert len(table.values()) == 4
    engine = tf.ThermoEngine(AQ)
    direct = engine.thermoPropertiesSubstance(298.15, 1e5, "Ca+2").gibbs_energy
    assert table.value("Ca+2", 25.0, 1.0, "G") == pytest.approx(direct.val / 1000.0, rel=1e-12)
    assert table.values()[0][0] == pytest.approx(direct.val / 1000.0, rel=1e-12)   # unlike the list of Output, in your unit
    scalar = table.scalar("Ca+2", 25.0, 1.0, "G")
    assert scalar.val == direct.val and scalar.ddt == direct.ddt


def test_toCSVTransposed_is_the_layout_wide():
    table = tf.batch(setup(output={"layout": "wide"}))["t"]
    lines = table.grid()
    assert lines[0][:3] == ["Property", "T(degC)", "P(bar)"] and lines[0][3:] == ["Ca+2", "Calcite"]
    assert len(lines) == 1 + 2 * 2                          # each property at each T, P
    assert float(lines[1][3]) == pytest.approx(table.value("Ca+2", 25.0, 1.0, "G"), abs=1e-3)


def test_toCSVPropertyGrid_in_both_directions():
    spec = {"database": AQ, "conditions": {"T": {"values": [25.0, 100.0], "unit": "degC"}, "P": {"values": [1000.0, 2000.0, 3000.0], "unit": "bar"}},
            "output": {"layout": "grid"}, "results": [{"name": "t", "substances": ["Ca+2"], "properties": [{"name": "G", "digits": 1}]}]}
    label, lines = tf.batch(spec)["t"].property_grids()[0]
    assert lines[0][1] == "T(degC)_P(bar)" and len(lines[0]) == 2 + 3 and len(lines) == 1 + 2
    spec["output"]["grid_across"] = "T"
    _, across = tf.batch(spec)["t"].property_grids()[0]
    assert across[0][1] == "P(bar)_T(degC)" and len(across[0]) == 2 + 2 and len(across) == 1 + 3
    assert lines[1][2] == across[1][2]                       # T = 25, P = 1000 in both


@pytest.mark.parametrize("output,text", [({"number_format": "scientific", "digits": 3}, "e+"), ({"number_format": "fixed", "digits": 1}, "."), ({"csv_separator": ";"}, ";")])
def test_number_format_and_separator(output, text):
    spec = setup(output=output)
    for property in spec["results"][0]["properties"]:
        property.pop("digits", None)
    assert text in tf.batch(spec)["t"].to_text("csv").splitlines()[1]


def test_the_order_of_the_rows():
    by_substance = [r[0] for r in tf.batch(setup())["t"].grid()[1:]]
    by_condition = [(r[0], r[1]) for r in tf.batch(setup(output={"rows_by": "condition"}))["t"].grid()[1:]]
    assert by_substance == ["Ca+2", "Ca+2", "Calcite", "Calcite"]
    assert [c[1] for c in by_condition[:2]] == ["25", "25"]
    spec = setup()
    spec["conditions"] = {"T": {"values": [25.0, 100.0], "unit": "degC"}, "P": {"values": [1.0, 100.0], "unit": "bar"}}
    assert [(row[1], row[2]) for row in tf.batch(spec)["t"].rows[:2]] == [(25.0, 1.0), (25.0, 100.0)]    # vary_first: P, the default
    spec["conditions"]["vary_first"] = "T"
    assert [(row[1], row[2]) for row in tf.batch(spec)["t"].rows[:2]] == [(25.0, 1.0), (100.0, 1.0)]


def test_the_properties_of_the_solvent_are_a_result():
    spec = setup(substances=["H2O@"], properties=[{"name": "density", "digits": 3}, {"name": "epsilon", "digits": 3}, "bornQ"])
    table = tf.batch(spec)["t"]
    engine = tf.ThermoEngine(AQ)
    assert table.value("H2O@", 25.0, 1.0, "density") == pytest.approx(engine.propertiesSolvent(298.15, 1e5, "H2O@").density.val)
    assert table.value("H2O@", 25.0, 1.0, "epsilon") == pytest.approx(engine.electroPropertiesSolvent(298.15, 1e5, "H2O@").epsilon.val)


def test_reactions_from_their_reactants_by_the_option_property_source():
    equation = "Calcite = Ca+2 + CO3-2"
    spec = setup(substances=[], reactions=[equation], properties=["logK"], engine={"options": {"property_source": "reactants"}})
    got = tf.batch(spec)["t"].value(equation, 25.0, 1.0, "logK")
    assert got == pytest.approx(tf.ThermoEngine(AQ).thermoPropertiesReactionFromReactants(298.15, 1e5, equation).log_equilibrium_constant.val)


def test_a_value_that_is_not_defined():
    doc = {"thermohub": "2.0.0-draft.1", "substances": {"X": {"formula": "Ca", "class": "component", "standard_state_properties": {"G": -1000.0}}}}
    spec = {"database": tf.Database(json.dumps(doc)), "conditions": {"T": [298.15], "P": [1e5]}, "output": {"undefined": "n/a"},
            "results": [{"name": "t", "substances": ["X"], "properties": ["V"]}]}
    table = tf.batch(spec)["t"]
    assert table.grid()[1][3] == "n/a"
    assert math.isnan(table.values()[0][0])
    spec["output"]["undefined"] = "number"
    assert tf.batch(spec)["t"].grid()[1][3] != "n/a"


def test_uncertainty_derivatives_and_status():
    spec = setup(substances=["Ca+2"], properties=[{"name": "G", "show": "ddt"}, {"name": "G", "show": "ddp"}, {"name": "G", "show": "uncertainty", "unit": "kJ/mol"}, {"name": "G", "show": "status"}])
    table = tf.batch(spec)["t"]
    direct = tf.ThermoEngine(AQ).thermoPropertiesSubstance(298.15, 1e5, "Ca+2").gibbs_energy
    cells = table.rows[0][3]
    assert cells[0].value == direct.ddt and cells[1].value == direct.ddp
    assert cells[2].value == pytest.approx(direct.err / 1000.0)
    assert cells[3].value == "calculated"
    assert [c["label"] for c in table.columns] == ["G_ddT(J/mol)", "G_ddP(J/mol)", "G_uncertainty(kJ/mol)", "G_status"]


def test_units_of_each_kind():
    spec = setup(substances=["Calcite"], properties=[{"name": "V", "unit": "cm3/mol", "digits": 3}, {"name": "S", "unit": "J/(mol K)"}, {"name": "H", "unit": "kcal/mol"}])
    table = tf.batch(spec)["t"]
    engine = tf.ThermoEngine(AQ).thermoPropertiesSubstance(298.15, 1e5, "Calcite")
    assert table.value("Calcite", 25.0, 1.0, "V") == pytest.approx(engine.volume.val * 1e6, rel=1e-9)
    assert table.value("Calcite", 25.0, 1.0, "H") == pytest.approx(engine.enthalpy.val / 4184.0, rel=1e-9)


def test_the_units_of_t_and_p_in_the_output():
    table = tf.batch(setup(output={"units": {"T": "K", "P": "MPa"}, "digits": {"T": 2}}))["t"]
    lines = table.grid()
    assert lines[0][1:3] == ["T(K)", "P(MPa)"]
    assert lines[1][1:3] == ["298.15", "0.1"]
    assert table.value("Ca+2", 25.0, 1.0, "G") == tf.batch(setup())["t"].value("Ca+2", 25.0, 1.0, "G")   # rows by the conditions as given


def test_a_bad_record_is_a_problem_and_the_rest_runs():
    results = tf.batch(setup(substances=["Ca+2", "NoSuch"]))
    assert len(results["t"].rows) == 2
    assert any("NoSuch" in str(p) for p in results.problems)


@pytest.mark.parametrize("bad,text", [({"properties": []}, "no properties"), ({"substances": []}, "no substances"), ({"properties": ["Nope"]}, "no property Nope"),
                                      ({"output": {"layout": "x"}}, "layout is one of long, wide, grid")])
def test_a_wrong_setup_says_why(bad, text):
    with pytest.raises(tf.BatchSetupError, match=text):
        tf.batch(setup(**bad))["t"].to_text()


@pytest.mark.parametrize("where,key,value", [
    ("top", "tables", []), ("top", "options", {}), ("top", "units", {}), ("top", "digits", 2), ("top", "format", "fixed"),
    ("result", "records", ["Ca+2"]), ("result", "columns", ["G"]), ("result", "layout", "wide"), ("result", "solvent", "H2O@"), ("result", "source", "reaction"),
    ("result", "output", {"output": "x.csv"}), ("result", "output", {"format": "fixed"}), ("result", "output", {"separator": ";"}),
    ("result", "output", {"not_defined": "-"}), ("result", "output", {"write_not_defined": False}), ("result", "output", {"across": "T"}),
    ("result", "output", {"loop": "conditions"}), ("result", "output", {"format_file": "md"}), ("conditions", "pairs", True), ("conditions", "order", "P_first"),
    ("property", "property", "G"), ("property", "format", "fixed"),
])
def test_each_old_key_is_unknown(where, key, value):
    spec = setup()
    if where == "top":
        spec[key] = value
    elif where == "result":
        spec["results"][0][key] = value
    elif where == "conditions":
        spec["conditions"][key] = value
    else:
        spec["results"][0]["properties"] = [{"name": "G", key: value}]
    old = key if key != "output" else next(iter(value))
    with pytest.raises(tf.BatchSetupError, match=f"has no key {old}"):
        tf.batch(spec)


@pytest.mark.parametrize("block,key,value,text", [("output", "layout", "transposed", "layout is one of"), ("property", "show", "err", "show is one of value, uncertainty"),
                                                  ("conditions", "points", "both", "points is one of grid, pairs"), ("output", "rows_by", "records", "rows_by is one of substance, condition")])
def test_each_old_word_is_unknown(block, key, value, text):
    spec = setup()
    if block == "property":
        spec["results"][0]["properties"] = [{"name": "G", key: value}]
    else:
        spec.setdefault(block, {})[key] = value
    with pytest.raises(tf.BatchSetupError, match=text):
        tf.batch(spec)


def test_markdown_and_latex(tmp_path):
    table = tf.batch(setup(output={"provenance": "none"}))["t"]
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
results:
  - name: species
    substances: [Ca+2]
    properties:
      - {name: G, unit: kJ/mol, digits: 3}
    output: {file: out.csv}
""")
    results = tf.batch(str(tmp_path / "setup.yaml"))
    assert [r[1] for r in results["species"].rows] == [25, 50, 75]
    assert results["species"].rows[0][2] == "saturation"        # the key of the point, for value() and scalar()
    # the text shows the pressure that the calculation used at the saturation point, in bar
    lines = [l.split(",") for l in results["species"].to_text("csv").splitlines() if l and not l.startswith("#")][1:]
    for line, T in zip(lines, (25.0, 50.0, 75.0)):
        assert float(line[2]) == pytest.approx(tf.ThermoEngine(AQ).saturationPressure(T + 273.15) / 1e5, rel=1e-5)


def test_the_command_writes_the_files(tmp_path, monkeypatch, capsys):
    (tmp_path / "setup.yaml").write_text(f"database: {AQ}\nconditions:\n  T: [298.15]\n  P: [100000.0]\nresults:\n  - name: s\n    substances: [Ca+2]\n    properties: [G]\n    output: {{file: out.md}}\n")
    monkeypatch.chdir(tmp_path)
    assert tf.batch.main(["setup.yaml"]) == 0
    assert "| Symbol" in (tmp_path / "out.md").read_text()
    assert tf.batch.main(["--template", "blank"]) == 0
    assert "<path of the database>" in capsys.readouterr().out


def test_each_template_makes_its_results(tmp_path):
    names = tf.templates()
    assert set(names) == {"species-properties", "reaction-properties", "logk-grid", "logk-saturation", "water-properties", "uncertainties", "derivatives",
                          "model-comparison", "database-comparison", "species-grid", "blank", "conditions-grid", "conditions-reference",
                          "conditions-saturation", "output-all-digits", "output-publication"}
    calcite = "Calcite = Ca+2 + CO3-2"
    given = {"species-properties": {"substances": ["Ca+2", "Calcite"]}, "reaction-properties": {"reactions": [calcite]}, "logk-grid": {"reactions": [calcite]},
             "logk-saturation": {"reactions": [calcite]}, "water-properties": {}, "uncertainties": {"substances": ["Ca+2"]}, "derivatives": {"substances": ["Ca+2"]},
             "model-comparison": {"substances": ["Na+"]}, "species-grid": {"substances": ["Calcite"]}}
    for name, more in given.items():
        results = tf.batch(template=name, database=AQ, **more)
        for table in results.values():
            assert table.rows and table.to_text("csv"), name
            # every value is there; a note of a point outside the range of a model is allowed (logk-grid reaches 400 degC at
            # 500 bar, where the water is below the density range of HKF: the reaction says so, via Ca+2, decision 0029)
            missing = [p for p in results.problems if "Outside the range of the model" not in p[-1]]
            assert not missing, (name, missing[:2])
    second = tmp_path / "second.json"
    second.write_text(pathlib.Path(AQ).read_text())
    made = tf.template("database-comparison", database=AQ, substances=["Calcite"])
    made.set("/results/0/compare/1/database", str(second))
    assert len(made.run()["databases"].columns) == 3


def test_a_template_needs_your_records():
    with pytest.raises(tf.BatchSetupError, match="no substances"):
        tf.batch(template="species-properties", database=AQ)


def test_blank_has_placeholders_that_name_their_paths():
    made = tf.template("blank")
    assert "<path of the database>" in str(made)
    with pytest.raises(tf.BatchSetupError, match=r"placeholders: /database \(<path of the database>\), /conditions/T/values/0"):
        made.run()
    made.set("/database", AQ).set("/conditions/T/values", [25, 100]).set("/conditions/P/values", [1]).set("/results/0/name", "mine").set("/results/0/substances", ["Calcite"])
    assert made.get("/results/0/name") == "mine"
    assert len(made.run()["mine"].rows) == 2


def test_settings_of_a_template_from_python(tmp_path):
    made = tf.template("species-properties", database=AQ, substances=["Ca+2"])
    made.property("G", unit="J/mol", digits=1)
    made.remove_property("V")
    made.add_property("A", unit="kJ/mol", digits=2)
    made.fill(T={"values": [25.0, 50.0], "unit": "degC"}, layout="wide", file=str(tmp_path / "x.csv"))
    results = made.run()
    table = results["species"]
    assert table.columns[0]["label"] == "G(J/mol)" and [c["property"] for c in table.columns][-1] == "A"
    assert table.value("Ca+2", 25.0, "saturation", "G") == pytest.approx(tf.ThermoEngine(AQ).thermoPropertiesSubstance(298.15, 0.0, "Ca+2").gibbs_energy.val)
    assert results.write() == [str(tmp_path / "x.csv")]


def test_a_changed_template_can_be_kept_and_read_back(tmp_path):
    made = tf.template("reaction-properties", database=AQ, reactions=["Calcite = Ca+2 + CO3-2"])
    made.property("logK", digits=5)
    path = made.save(str(tmp_path / "mine.yaml"))
    again = tf.batch.load(path)
    assert again.run()["reactions"].to_text("csv") == made.run()["reactions"].to_text("csv")


def test_compare_two_dielectric_models_with_their_difference():
    results = tf.batch(template="model-comparison", database=AQ, substances=["Na+"], P={"values": [1.0], "unit": "bar"}, T={"values": [25.0], "unit": "degC"})
    table = results["comparison"]
    assert [c["label"] for c in table.columns] == ["V(J/bar) [JN91]", "Cp(J/K/mol) [JN91]", "V(J/bar) [HK74]", "Cp(J/K/mol) [HK74]",
                                                    "V(J/bar) [HK74 - JN91]", "Cp(J/K/mol) [HK74 - JN91]"]
    cells = table.rows[0][3]
    assert cells[0].value != cells[2].value
    assert cells[4].value == pytest.approx(cells[2].value - cells[0].value, rel=1e-12)


def test_compare_two_databases_with_their_difference(tmp_path):
    doc = json.loads(tf.Database(AQ).toJson(True))
    G = doc["substances"]["Calcite"]["standard_state_properties"]["G"]
    doc["substances"]["Calcite"]["standard_state_properties"]["G"] = G - 1000.0 if isinstance(G, float) else {**G, "value": G["value"] - 1000.0}
    other = tmp_path / "other.json"
    other.write_text(json.dumps(doc))
    spec = setup(substances=["Calcite"], properties=[{"name": "G", "unit": "kJ/mol"}], compare=[{"label": "aq17"}, {"label": "other", "database": str(other)}],
                 compare_difference=True)
    table = tf.batch(spec)["t"]
    assert [c["label"] for c in table.columns] == ["G(kJ/mol) [aq17]", "G(kJ/mol) [other]", "G(kJ/mol) [other - aq17]"]
    assert table.rows[0][3][2].value == pytest.approx(-1.0, abs=1e-9)


def test_compare_an_engine_block():
    spec = setup(substances=["Ca+2"], properties=[{"name": "G"}], compare=[{"label": "a"}, {"label": "b", "engine": {"options": {"round_to_uncertainty": True}}}])
    assert len(tf.batch(spec)["t"].columns) == 2


def test_a_result_gives_its_own_blocks_key_by_key():
    spec = setup(output={"number_format": "fixed", "digits": 4})
    spec["results"].append({"name": "u", "substances": ["Calcite"], "properties": ["G"], "output": {"digits": 1}, "conditions": {"T": {"values": [50.0], "unit": "degC"}, "P": {"values": [1.0], "unit": "bar"}}})
    results = tf.batch(spec)
    line = results["u"].grid()[1]
    assert line[1] == "50" and len(line[3].split(".")[1]) == 1        # fixed from the top, 1 digit from the result
    assert len(results["t"].grid()[1][3].split(".")[1]) == 3            # the property gives its own digits


def test_an_engine_or_a_database_can_be_given():
    engine = tf.ThermoEngine(AQ)
    spec = setup()
    del spec["database"]
    assert tf.batch(spec, database=engine)["t"].rows
    assert tf.batch(spec, database=tf.Database(AQ))["t"].rows


def test_a_value_with_a_message_is_a_problem_of_the_result():
    results = tf.batch(template=["species-properties", "conditions-grid"], database=AQ, substances=["Ca+2"])
    assert any("HKF model: outside" in message for *_, message in results.problems)    # the solute is outside the range of the density of water
    assert not any(message.endswith(";") or message.endswith(" ") for *_, message in results.problems)


def test_a_value_outside_the_range_of_its_model_is_marked_and_listed():
    engine = tf.ThermoEngine(AQ)
    inside = engine.thermoPropertiesSubstance(298.15, 1e5, "Ca+2").gibbs_energy
    outside = engine.thermoPropertiesSubstance(673.15, 5e7, "Ca+2").gibbs_energy      # water below 600 kg/m3
    assert inside.info.validity == tf.Validity.within and outside.info.validity == tf.Validity.extrapolated
    assert outside.sta[0] == tf.Status.calculated                                    # the value is there
    results = tf.batch(template="species-properties", database=AQ, substances=["Ca+2"], T={"values": [400], "unit": "degC"}, P={"values": [500], "unit": "bar"})
    assert any("Outside the range of the model" in message for *_, message in results.problems)


def test_the_pressure_used_at_saturation():
    results = tf.batch(template=["species-properties", "conditions-saturation"], database=AQ, substances=["Ca+2"])
    table = results["species"]
    assert table.pressure_used("Ca+2", 100, "saturation") == pytest.approx(tf.ThermoEngine(AQ).saturationPressure(373.15) / 1e5, rel=1e-6)
    with pytest.raises(tf.BatchSetupError, match="no row"):
        table.pressure_used("Ca+2", 101, "saturation")


def test_the_old_names_are_gone():
    for name in ("table", "tables", "TableSetup", "Table"):
        assert not hasattr(tf, name), name
    assert callable(tf.batch) and tf.batch.__name__ == "thermofun.batch"
