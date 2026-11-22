# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# Reading and changing a value of a record by its path (also a key of a value written as a number), a dict as a database, the
# summary, and the text of V2-MTH-004.

import pathlib

import pytest

import thermofun as tf

AQ = str(pathlib.Path(__file__).parent.parent / "Resources" / "databases" / "aq17-thermofun.json")


def test_a_value_is_read_by_its_path():
    database = tf.Database(AQ)
    assert database.get("/substances/Calcite/standard_state_properties/G") == pytest.approx(database.getSubstance("Calcite").thermoReferenceProperties().gibbs_energy.val)
    assert database.get("/substances/Ca+2/methods/0/method") == "HKF88"
    assert isinstance(database.get("/substances/Ca+2/methods/0/parameters"), dict)
    assert database.get("/substances/Calcite")["formula"] == "CaCO3"


def test_a_value_is_set_by_its_path_and_the_engine_made_after_it_uses_it():
    database = tf.Database(AQ)
    before = tf.ThermoEngine(database).thermoPropertiesSubstance(373.15, 1e8, "Ca+2").volume.val
    database.set("/substances/Ca+2/methods/0/parameters/omega", 2.0e5)
    assert database.get("/substances/Ca+2/methods/0/parameters/omega") == 2.0e5
    assert tf.ThermoEngine(database).thermoPropertiesSubstance(373.15, 1e8, "Ca+2").volume.val != before
    assert tf.ThermoEngine(AQ).thermoPropertiesSubstance(373.15, 1e8, "Ca+2").volume.val == before   # the file does not change


def test_an_uncertainty_and_a_list_of_methods_are_set_as_python_objects():
    database = tf.Database(AQ)
    G = database.get("/substances/Calcite/standard_state_properties/G")
    database.set("/substances/Calcite/standard_state_properties/G", {"value": G, "uncertainty": 1500.0})
    assert tf.ThermoEngine(database).thermoPropertiesSubstance(373.15, 1e8, "Calcite").gibbs_energy.err == pytest.approx(1500.0)
    database.set("/substances/H2O@/methods", [{"method": "WaterIAPWS95"}, {"method": "HelgesonKirkham74"}])
    assert [m["method"] for m in database.get("/substances/H2O@/methods")] == ["WaterIAPWS95", "HelgesonKirkham74"]
    assert tf.ThermoEngine(database).calculationPath("H2O@")["methods"] == ["WaterIAPWS95", "HelgesonKirkham74"]


def test_the_uncertainty_of_a_value_written_as_a_number_is_set_alone():
    database = tf.Database(AQ)
    S = "/substances/Calcite/standard_state_properties/S"
    value = database.get(S)
    assert isinstance(value, float) and database.get(S + "/value") == value
    database.set(S + "/uncertainty", 15.0)
    assert database.get(S) == {"value": value, "uncertainty": 15.0}
    assert database.get(S + "/uncertainty") == 15.0
    result = tf.ThermoEngine(database).thermoPropertiesSubstance(373.15, 1e5, "Calcite")
    assert result.entropy.err == pytest.approx(15.0)
    assert result.gibbs_energy.err == pytest.approx(15.0 * (373.15 - 298.15))
    with pytest.raises(Exception, match="no value"):
        database.get("/substances/Calcite/standard_state_properties/G/uncertainty")


def test_a_record_is_added_by_its_path():
    database = tf.Database(AQ)
    database.set("/substances/Mine", {"formula": "CaCO3", "class": "component", "aggregate_state": "crystal",
                                       "standard_state_properties": {"G": -1.1e6, "H": -1.2e6, "S": 90.0, "Cp": 83.0, "V": 3.6e-5},
                                       "methods": [{"method": "ConstantCp"}, {"method": "ConstantVolume"}]})
    assert database.containsSubstance("Mine")
    assert tf.ThermoEngine(database).thermoPropertiesSubstance(298.15, 1e5, "Mine").heat_capacity_cp.val == pytest.approx(83.0)


@pytest.mark.parametrize("path, text", [
    ("substances/Calcite", "does not start"),
    ("/substances", "no symbol"),
    ("/minerals/Calcite", "not elements, substances or reactions"),
    ("/substances/NoSuch/formula", "no substance NoSuch"),
    ("/substances/Calcite/nothing", "no value at /nothing"),
])
def test_a_wrong_path_says_what_is_wrong(path, text):
    with pytest.raises(ValueError, match=text):
        tf.Database(AQ).get(path)


def test_a_dict_in_the_v2_format_is_a_database():
    database = tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"A": {"formula": "Al2O3", "class": "component", "aggregate_state": "crystal",
                                                                             "standard_state_properties": {"G": -1582000, "H": -1675700, "S": 50.92, "Cp": 79.0, "V": 2.558e-5},
                                                                             "methods": [{"method": "ConstantCp"}, {"method": "ConstantVolume"}]}}})
    assert database.getSubstancesList() == ["A"]


def test_the_summary_counts_the_substances_by_class_and_state():
    summary = tf.Database(AQ).summary()
    counts = {(c.substance_class, c.aggregate_state): c.substances for c in summary.classes}
    assert counts[("aqueous_solute", "aqueous")] == 48
    assert sum(counts.values()) == summary.substances == 161
    assert "aqueous_solute" in str(summary) and str(summary).startswith(summary.name)


def test_a_missing_parameter_says_what_it_is_and_which_method_needs_it():
    # HKF88 without omega: a block with some parameters but not all the required ones is refused at load
    na = {"formula": "Na+", "charge": 1, "class": "aqueous_solute", "aggregate_state": "aqueous", "standard_state_properties": {"G": -261881.0},
          "methods": [{"method": "HKF88", "parameters": {"a1": 0.1839, "a2": -228.5, "a3": 3.256, "a4": -27260, "c1": 18.18, "c2": -29810}}]}
    with pytest.raises(RuntimeError) as error:
        tf.Database({"substances": {"Na+": na}})
    message = str(error.value)
    assert "V2-MTH-004" in message and "HKF88" in message and "omega" in message and "Born coefficient" in message and "cal/mol" in message


def test_a_dict_can_leave_out_the_version_and_is_not_changed():
    records = {"substances": {"A": {"formula": "Al2O3", "class": "component", "aggregate_state": "crystal", "standard_state_properties": {"G": -1.5e6}}}}
    assert tf.Database(records).getSubstancesList() == ["A"]
    assert "thermohub" not in records                       # the dict of the caller
    assert tf.Database.formatVersion().startswith("2.")


def test_text_with_maps_of_records_and_no_version_is_refused_with_the_reason():
    # before, it was read as a legacy file, and gave one substance with no symbol
    with pytest.raises(RuntimeError, match="V2-VER-003"):
        tf.Database('{"substances": {"A": {"formula": "Ca"}}}')


def test_the_class_has_clearer_aliases_that_are_written_with_the_word_of_the_format():
    database = tf.Database({"substances": {
        "M": {"formula": "CaCO3", "class": "condensed_species", "aggregate_state": "crystal", "standard_state_properties": {"G": -1e6}},
        "W": {"formula": "H2O@", "class": "solvent", "aggregate_state": "aqueous", "standard_state_properties": {"G": -237183.0}}}})
    assert database.get("/substances/M/class") == "component"
    assert database.get("/substances/W/class") == "water_solvent"
    assert not [d for d in database.loadDiagnostics().all() if d.code == "V2-VOC-001"]
