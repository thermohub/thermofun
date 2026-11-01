# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The Shomate equations of the NIST WebBook as a symbolic method, checked against the NIST-JANAF tables.

import json
import pathlib

import pytest

sympy = pytest.importorskip("sympy")

import thermofun as tf  # noqa: E402
from thermofun import method_tool as methods  # noqa: E402

DATA = json.loads((pathlib.Path(__file__).resolve().parent / "data" / "sources" / "janaf-shomate.json").read_text())
LETTERS = "ABCDEFG"

RECORD = {"kind": "symbolic", "tier": 1, "operates_on": "substance", "contributes_to": ["G", "H", "S", "Cp"],
          "parameter_definitions": {n: {"unit": "1"} for n in LETTERS}, "source": {"form": "shomate"}}
BUILT = methods.build(RECORD)


def piece_for(species, T):
    for piece in DATA["species"][species]["pieces"]:
        if piece["t_min"] <= T <= piece["t_max"]:
            return piece
    return None


CASES = [(name, row) for name, s in DATA["species"].items() for row in s["rows"] if piece_for(name, row["T"])]


@pytest.mark.parametrize("name,row", CASES, ids=[f"{n}-{r['T']}" for n, r in CASES])
def test_the_form_reproduces_the_janaf_table(name, row):
    piece = piece_for(name, row["T"])
    case = {"T": row["T"], "P": 1e5, "parameters": {n: piece[n] for n in LETTERS}}
    got = methods.properties(BUILT["expression"]["G"], BUILT, case)
    assert got["Cp"] == pytest.approx(row["Cp"], abs=0.03)
    assert got["S"] == pytest.approx(row["S"], abs=0.05)
    # H of the form is on the assigned scale: the enthalpy of formation at 298.15 K plus H - H(298.15) of the table
    assert got["H"] == pytest.approx((DATA["species"][name]["hf298"] + row["dH"]) * 1000.0, abs=100.0)   # J/mol


def test_the_pieces_join_at_their_limits():
    for name, s in DATA["species"].items():
        pieces = s["pieces"]
        for lower, upper in zip(pieces, pieces[1:]):
            T = lower["t_max"]
            a = methods.properties(BUILT["expression"]["G"], BUILT, {"T": T, "parameters": {n: lower[n] for n in LETTERS}})
            b = methods.properties(BUILT["expression"]["G"], BUILT, {"T": T, "parameters": {n: upper[n] for n in LETTERS}})
            assert a["S"] == pytest.approx(b["S"], abs=0.1), name
            assert a["H"] == pytest.approx(b["H"], abs=200.0), name


def engine(name):
    pieces = DATA["species"][name]["pieces"]
    substance = {"formula": name, "charge": 0, "class": "gas_fluid", "aggregate_state": "gas",
                 "reference_conditions": {"T": 298.15, "P": 1e5},
                 "standard_state_properties": {"G": 0.0, "H": 0.0, "S": 0.0, "Cp": 0.0, "V": 0.0},
                 "convention": {"energy_reference": "benson_helgeson"},   # no conversion: the test has no elements
                 "methods": [{"method": "Shomate", "segments": [
                     {"t_min": p["t_min"], "t_max": p["t_max"], "parameters": {n: p[n] for n in LETTERS}} for p in pieces]}]}
    doc = {"thermohub": "2.0.0-draft.1", "methods": {"Shomate": BUILT}, "substances": {name: substance}}
    e = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    p = e.preferences
    p.enable_memoize = False
    e.setPreferences(p)
    return e


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
@pytest.mark.parametrize("name,row", CASES, ids=[f"{n}-{r['T']}" for n, r in CASES])
def test_the_engine_uses_the_piece_that_holds_the_temperature(name, row):
    p = engine(name).thermoPropertiesSubstance(row["T"], 1e5, name)
    piece = piece_for(name, row["T"])
    want = methods.properties(BUILT["expression"]["G"], BUILT, {"T": row["T"], "parameters": {n: piece[n] for n in LETTERS}})
    assert p.heat_capacity_cp.val == pytest.approx(want["Cp"], rel=1e-9)
    assert p.entropy.val == pytest.approx(want["S"], rel=1e-9)
    assert p.heat_capacity_cp.val == pytest.approx(row["Cp"], abs=0.03)


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
def test_a_temperature_outside_the_pieces_is_an_error():
    with pytest.raises(RuntimeError, match="outside the pieces"):
        engine("H2O").thermoPropertiesSubstance(300.0, 1e5, "H2O")


FORMULAS = {"CO2": "CO2", "H2O": "H2O", "N2": "N2"}


def engine_with_elements(name, energy_reference="assigned_enthalpy", elements=True):
    pieces = DATA["species"][name]["pieces"]
    substance = {"formula": FORMULAS[name], "charge": 0, "class": "gas_fluid", "aggregate_state": "gas",
                 "reference_conditions": {"T": 298.15, "P": 1e5},
                 "standard_state_properties": {"G": 0.0, "H": 0.0, "S": 0.0, "Cp": 0.0, "V": 0.0},
                 "convention": {"energy_reference": energy_reference},
                 "methods": [{"method": "Shomate", "segments": [
                     {"t_min": p["t_min"], "t_max": p["t_max"], "parameters": {n: p[n] for n in LETTERS}} for p in pieces]}]}
    doc = {"thermohub": "2.0.0-draft.1", "methods": {"Shomate": BUILT}, "substances": {name: substance}}
    if elements:
        doc["elements"] = DATA["elements"]
    e = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    p = e.preferences
    p.enable_memoize = False
    e.setPreferences(p)
    return e


ATOMS = {"CO2": {"C": 1, "O": 2}, "H2O": {"H": 2, "O": 1}, "N2": {"N": 2}}


def sum_of_element_entropies(name):
    return sum(n * DATA["elements"][e]["standard_entropy"] for e, n in ATOMS[name].items())


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
@pytest.mark.parametrize("name,row", CASES, ids=[f"{n}-{r['T']}" for n, r in CASES])
def test_the_assigned_enthalpy_scale_is_converted_by_hand_and_by_the_engine_alike(name, row):
    # by hand from the table: G = H - T S on the assigned scale, plus Tr times the entropies of the elements
    hand = (DATA["species"][name]["hf298"] + row["dH"]) * 1000.0 - row["T"] * row["S"] + 298.15 * sum_of_element_entropies(name)
    p = engine_with_elements(name).thermoPropertiesSubstance(row["T"], 1e5, name)
    assert p.gibbs_energy.val == pytest.approx(hand, abs=100.0 + 0.05 * row["T"])


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
@pytest.mark.parametrize("name", ["CO2", "N2"])
def test_at_the_reference_temperature_the_gibbs_energy_is_that_of_formation(name):
    p = engine_with_elements(name).thermoPropertiesSubstance(298.15, 1e5, name)
    assert p.gibbs_energy.val == pytest.approx(DATA["species"][name]["dfG298"] * 1000.0, abs=30.0)


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
def test_the_conversion_is_the_offset_of_the_elements_and_nothing_else():
    # canonical G - declared G = Tr * (sum of the entropies of the elements); S, H and Cp do not change
    raw = engine_with_elements("CO2", "benson_helgeson").thermoPropertiesSubstance(700.0, 1e5, "CO2")
    conv = engine_with_elements("CO2").thermoPropertiesSubstance(700.0, 1e5, "CO2")
    assert conv.gibbs_energy.val - raw.gibbs_energy.val == pytest.approx(298.15 * sum_of_element_entropies("CO2"), rel=1e-9)
    assert conv.entropy.val == pytest.approx(raw.entropy.val, rel=1e-12)
    assert conv.enthalpy.val == pytest.approx(raw.enthalpy.val, rel=1e-12)
    assert conv.heat_capacity_cp.val == pytest.approx(raw.heat_capacity_cp.val, rel=1e-12)


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
def test_without_element_records_the_built_in_element_data_is_used():
    with_records = engine_with_elements("CO2").thermoPropertiesSubstance(298.15, 1e5, "CO2").gibbs_energy.val
    built_in = engine_with_elements("CO2", elements=False).thermoPropertiesSubstance(298.15, 1e5, "CO2").gibbs_energy.val
    assert built_in == pytest.approx(with_records, abs=10.0)
