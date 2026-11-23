# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The words of the v2 format checked against NEA TDB and IUPAC (decision 0031): each new word is read and written, and the
# old one (never released) is now an unknown key.

import pytest
import thermofun as tf

HEADER = {"thermohub": "2.0.0-draft.1"}


def mineral(**more):
    record = {"formula": "Ca", "class": "component", "aggregate_state": "crystal",
              "standard_state_properties": {"G": -1.0e6, "H": -1.1e6, "S": 50.0, "Cp": 80.0, "V": 3e-5}, "methods": [{"method": "ConstantCp"}]}
    record.update(more)
    return record


def database(**sections):
    return tf.Database(dict(HEADER, **sections))


def warnings(db, code):
    return [d for d in db.loadDiagnostics().all() if d.code == code]


def test_derivation_of_a_value_any_origin():
    G = {"value": -1.0e6, "origin": "estimated", "derivation": "by analogy with Sr+2"}
    db = database(substances={"A": mineral(standard_state_properties={"G": G})})
    assert db.get("/substances/A/standard_state_properties/G")["derivation"] == "by analogy with Sr+2"
    old = database(substances={"A": mineral(standard_state_properties={"G": {"value": -1.0e6, "calculation_path": "x"}})})
    assert warnings(old, "V2-QTY-003")


def test_quality_is_free_text():
    for word in ("core", "selected", "auxiliary"):
        db = database(substances={"A": mineral(standard_state_properties={"G": {"value": -1.0e6, "quality": word}})})
        assert db.get("/substances/A/standard_state_properties/G")["quality"] == word
        assert not warnings(db, "V2-VOC-001")


def test_reference_state_of_an_element():
    db = database(elements={"C": {"class": "element", "molar_mass": 12.011, "reference_state": "C(cr, graphite)"}})
    assert db.getElement("C").record().reference_state == "C(cr, graphite)"
    old = database(elements={"C": {"class": "element", "molar_mass": 12.011, "reference_form": "graphite"}})
    assert warnings(old, "V2-KEY-001")


@pytest.mark.parametrize("word, scale", [("amount_concentration", "molarity"), ("amount_fraction", "mole_fraction")])
def test_iupac_names_of_the_concentration_scales(word, scale):
    db = database(substances={"A": mineral(standard_state={"concentration_scale": word})})
    assert str(db.getSubstance("A").record().convention.standard_state.concentration_scale).split(".")[-1].lower() == scale.replace("_", "")


def test_dfG_and_dfH_are_G_and_H():
    db = database(substances={"A": mineral(standard_state_properties={"dfG": -1.0e6, "dfH": -1.1e6, "S": 50.0})})
    record = db.getSubstance("A").record()
    assert db.get("/substances/A/standard_state_properties/G") == -1.0e6
    assert db.get("/substances/A/standard_state_properties/H") == -1.1e6
    assert not warnings(db, "CNV-009") and not warnings(db, "V2-KEY-001")
    # H at Tr is dfH in every energy reference; G is dfG only in benson_helgeson and gibbs_helmholtz_formation
    bb = database(substances={"A": mineral(standard_state_properties={"dfG": -1.0e6, "dfH": -1.1e6, "S": 50.0}, energy_reference={"elements": "berman_brown"})})
    assert [d.path for d in warnings(bb, "CNV-009")] == ["/substances/A/standard_state_properties/dfG"]
    only_h = database(substances={"A": mineral(standard_state_properties={"G": -1.0e6, "dfH": -1.1e6}, energy_reference={"elements": "berman_brown"})})
    assert not warnings(only_h, "CNV-009")


def test_formula_of_a_method_definition_and_no_tier():
    method = {"kind": "symbolic", "operates_on": "substance", "contributes_to": ["G", "H", "S", "Cp"],
              "parameter_definitions": {"a": {"unit": "J/(mol.K)"}}, "formula": {"form": "cp", "expression": "a"},
              "data_sources": ["1932MAI/KEL"]}
    db = database(methods={"MyCp": method})
    definition = db.methods()["MyCp"]
    assert definition.formula_form == "cp" and definition.formula_expression == "a"
    assert definition.data_sources == ["1932MAI/KEL"]
    old = database(methods={"MyCp": dict(method, tier=1, citation="1932MAI/KEL")})
    assert {d.path.split("/")[-1] for d in warnings(old, "V2-KEY-001")} >= {"tier", "citation"}


def test_parameter_set_has_no_auxiliary_and_names_its_sources():
    good = {"method": "HKF88", "parameters": {"a1": 1.0}, "data_sources": "1988TAN/HEL"}
    db = database(parameter_sets={"P": good})
    assert db.parameterSets()["P"].data_sources == ["1988TAN/HEL"]
    old = database(parameter_sets={"P": dict(good, auxiliary={"Tc": 647.096}, reference="x")})
    assert {d.path.split("/")[-1] for d in warnings(old, "V2-KEY-001")} >= {"auxiliary", "reference"}


def test_interpolation_order_of_a_table():
    table = {"temperatures": [273.15, 298.15, 373.15, 473.15], "logK": [1.0, 1.1, 1.3, 1.6]}
    db = database(reactions={"R": {"equation": "A = B", "methods": [{"method": "LogKTable", "interpolation": "lagrange", "interpolation_order": 2, "table": table}]}})
    assert db.getReaction("R").record().methods[0].interpolation_order == 2
    old = database(reactions={"R": {"equation": "A = B", "methods": [{"method": "LogKTable", "interpolation": "lagrange", "order": 2, "table": table}]}})
    assert warnings(old, "V2-KEY-001")


def test_one_list_of_data_sources_on_a_record_and_the_dataset():
    db = database(thermodataset={"symbol": "x", "data_sources": ["1995ROB/HEM"]},
                  substances={"A": mineral(data_sources="1995ROB/HEM")}, data_sources={"1995ROB/HEM": {"title": "t"}})
    assert db.getSubstance("A").record().data_sources == ["1995ROB/HEM"]
    old = database(substances={"A": mineral(data_source="1995ROB/HEM")}, data_sources={"1995ROB/HEM": {"title": "t"}})
    assert warnings(old, "V2-KEY-001")
