# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The NASA 7-term polynomials as a symbolic method, checked against the NIST-JANAF tables.

import json
import pathlib

import pytest

sympy = pytest.importorskip("sympy")

import thermofun as tf  # noqa: E402
from thermofun import method_tool as methods  # noqa: E402

DATA = json.loads((pathlib.Path(__file__).resolve().parent / "data" / "sources" / "nasa7-gri30.json").read_text())
NAMES = [f"a{i}" for i in range(1, 8)]
BUILT = methods.build({"kind": "symbolic", "tier": 1, "operates_on": "substance", "contributes_to": ["G", "H", "S", "Cp"],
                       "parameter_definitions": {n: {"unit": "1"} for n in NAMES}, "source": {"form": "nasa7"}})


def piece_for(species, T):
    for piece in DATA["species"][species]["pieces"]:
        if piece["t_min"] <= T <= piece["t_max"]:
            return piece
    return None


def props(name, T):
    piece = piece_for(name, T) or DATA["species"][name]["pieces"][0]   # 298.15 K is 2 K below the first piece of N2
    return methods.properties(BUILT["expression"]["G"], BUILT, {"T": T, "parameters": {n: piece[n] for n in NAMES}})


CASES = [(n, r) for n, s in DATA["species"].items() for r in s["rows"] if piece_for(n, r["T"])]


R = 8.31451


def polynomial(name, T):
    """Cp, S and H straight from the polynomials (J/mol), with the gas constant of ThermoFun"""
    a = [piece_for(name, T)[f"a{i}"] for i in range(1, 8)]
    cp = R * (a[0] + a[1] * T + a[2] * T**2 + a[3] * T**3 + a[4] * T**4)
    h = R * T * (a[0] + a[1] * T / 2 + a[2] * T**2 / 3 + a[3] * T**3 / 4 + a[4] * T**4 / 5 + a[5] / T)
    import math
    s = R * (a[0] * math.log(T) + a[1] * T + a[2] * T**2 / 2 + a[3] * T**3 / 3 + a[4] * T**4 / 4 + a[6])
    return {"Cp": cp, "S": s, "H": h}


@pytest.mark.parametrize("name,row", CASES, ids=[f"{n}-{r['T']}" for n, r in CASES])
def test_the_form_equals_the_polynomials(name, row):
    got, want = props(name, row["T"]), polynomial(name, row["T"])
    for key in ("Cp", "S", "H"):
        assert got[key] == pytest.approx(want[key], rel=1e-10), key


@pytest.mark.parametrize("name,row", CASES, ids=[f"{n}-{r['T']}" for n, r in CASES])
def test_the_polynomials_are_close_to_the_janaf_table(name, row):
    # the fits are older than the tables: the water fit differs by up to 2 % in Cp at 2000 K and above
    got = props(name, row["T"])
    assert got["Cp"] == pytest.approx(row["Cp"], rel=0.025)
    assert got["S"] == pytest.approx(row["S"], abs=1.0)
    # H on the assigned scale: the enthalpy of formation at 298.15 K plus H - H(298.15)
    assert got["H"] - props(name, 298.15)["H"] == pytest.approx(row["dH"] * 1000.0, abs=1500.0)


@pytest.mark.parametrize("name", ["CO2", "H2O", "N2"])
def test_the_enthalpy_at_298_is_the_enthalpy_of_formation(name):
    assert props(name, 298.15)["H"] == pytest.approx(DATA["species"][name]["hf298"] * 1000.0, abs=1500.0)


def test_the_two_pieces_join_at_1000_k():
    for name in DATA["species"]:
        a, b = DATA["species"][name]["pieces"][:2]
        T = a["t_max"]
        pa = methods.properties(BUILT["expression"]["G"], BUILT, {"T": T, "parameters": {n: a[n] for n in NAMES}})
        pb = methods.properties(BUILT["expression"]["G"], BUILT, {"T": T, "parameters": {n: b[n] for n in NAMES}})
        assert pa["S"] == pytest.approx(pb["S"], abs=0.05), name
        assert pa["H"] == pytest.approx(pb["H"], abs=150.0), name


def engine(name):
    substance = {"formula": name, "charge": 0, "class": "gas_fluid", "aggregate_state": "gas",
                 "reference_conditions": {"T": 298.15, "P": 1e5},
                 "standard_state_properties": {"G": 0.0, "H": 0.0, "S": 0.0, "Cp": 0.0, "V": 0.0},
                 "convention": {"energy_reference": "benson_helgeson"},
                 "methods": [{"method": "NASA7", "segments": [
                     {"t_min": p["t_min"], "t_max": p["t_max"], "parameters": {n: p[n] for n in NAMES}}
                     for p in DATA["species"][name]["pieces"]]}]}
    doc = {"thermohub": "2.0.0-draft.1", "methods": {"NASA7": BUILT}, "substances": {name: substance}}
    e = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    p = e.preferences
    p.enable_memoize = False
    e.setPreferences(p)
    return e


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
@pytest.mark.parametrize("name,row", CASES, ids=[f"{n}-{r['T']}" for n, r in CASES])
def test_the_engine_uses_the_piece_of_the_temperature(name, row):
    p = engine(name).thermoPropertiesSubstance(row["T"], 1e5, name)
    want = props(name, row["T"])
    assert p.heat_capacity_cp.val == pytest.approx(want["Cp"], rel=1e-9)
    assert p.entropy.val == pytest.approx(want["S"], rel=1e-9)
