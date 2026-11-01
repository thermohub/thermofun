# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The SGTE polynomial and the magnetic term of Inden, Hillert and Jarl as symbolic methods, for bcc iron,
# compared with the Gibbs energies that pycalphad calculates from the same data.

import json
import pathlib

import pytest

sympy = pytest.importorskip("sympy")

import thermofun as tf  # noqa: E402
from thermofun import method_tool as methods  # noqa: E402

DATA = json.loads((pathlib.Path(__file__).resolve().parent / "data" / "sources" / "sgte-fe-bcc.json").read_text())
LETTERS = "abcdefgh"
MAG = DATA["magnetic"]

BASE = methods.build({"kind": "symbolic", "tier": 1, "operates_on": "substance",
                      "parameter_definitions": {n: {"unit": "1"} for n in LETTERS}, "source": {"form": "sgte"}})
MAGNETIC = methods.build({"kind": "symbolic", "tier": 1, "operates_on": "substance",
                          "parameter_definitions": {"Tc": {"unit": "K"}, "beta": {"unit": "1"}, "p": {"unit": "1"}},
                          "source": {"form": "magnetic_ihj"}})


def by_hand(T):
    """G of the table: polynomial plus magnetic term, from the formulas, with the gas constant of ThermoFun"""
    import math
    piece = DATA["base_pieces"][0] if T <= 1811.0 else DATA["base_pieces"][1]
    g = (piece["a"] + piece["b"] * T + piece["c"] * T * math.log(T) + piece["d"] * T**2 + piece["e"] * T**3
         + piece["f"] / T + piece["g"] * T**7 + piece["h"] / T**9)
    tau, p = T / MAG["Tc"], MAG["p"]
    A = 518 / 1125 + 11692 / 15975 * (1 / p - 1)
    if tau < 1:
        f = 1 - (79 / (140 * p * tau) + 474 / 497 * (1 / p - 1) * (tau**3 / 6 + tau**9 / 135 + tau**15 / 600)) / A
    else:
        f = -(tau**-5 / 10 + tau**-15 / 315 + tau**-25 / 1500) / A
    return g + 8.31451 * T * math.log(MAG["beta"] + 1) * f


def test_the_magnetic_form_gives_one_expression_for_each_side_of_tc():
    assert len(MAGNETIC["expression"]) == 2 and all("G_increment" in e for e in MAGNETIC["expression"])
    assert methods.check(MAGNETIC) == []


@pytest.mark.parametrize("row", DATA["rows"], ids=[str(r["T"]) for r in DATA["rows"]])
def test_the_formulas_agree_with_pycalphad(row):
    assert by_hand(row["T"]) == pytest.approx(row["G"], abs=0.2)


def engine():
    magnetic_pieces = [{"t_min": 0.0, "t_max": MAG["Tc"], "parameters": MAG}, {"t_min": MAG["Tc"], "t_max": 6000.0, "parameters": MAG}]
    substance = {"formula": "Fe", "charge": 0, "class": "component", "aggregate_state": "crystal",
                 "reference_conditions": {"T": 298.15, "P": 1e5},
                 "standard_state_properties": {"G": 0.0, "H": 0.0, "S": 0.0, "Cp": 0.0, "V": 7.09e-6},
                 "convention": {"energy_reference": "benson_helgeson"},
                 "methods": [{"method": "SGTE", "segments": [
                     {"t_min": p["t_min"], "t_max": p["t_max"], "parameters": {n: p[n] for n in LETTERS}} for p in DATA["base_pieces"]]},
                     {"method": "MagneticIHJ", "segments": magnetic_pieces}]}
    doc = {"thermohub": "2.0.0-draft.1", "methods": {"SGTE": BASE, "MagneticIHJ": MAGNETIC}, "substances": {"Fe": substance}}
    e = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    p = e.preferences
    p.enable_memoize = False
    e.setPreferences(p)
    return e


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
@pytest.mark.parametrize("row", DATA["rows"], ids=[str(r["T"]) for r in DATA["rows"]])
def test_the_engine_agrees_with_pycalphad(row):
    p = engine().thermoPropertiesSubstance(row["T"], 1e5, "Fe")
    assert p.gibbs_energy.val == pytest.approx(row["G"], abs=0.2)


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
def test_the_derivatives_are_consistent_and_the_heat_capacity_peaks_at_tc():
    e = engine()
    p = e.thermoPropertiesSubstance(1000.0, 1e5, "Fe")
    assert p.gibbs_energy.ddt == pytest.approx(-p.entropy.val, rel=1e-9)
    assert p.entropy.ddt == pytest.approx(p.heat_capacity_cp.val / 1000.0, rel=1e-8)
    cp = {T: e.thermoPropertiesSubstance(T, 1e5, "Fe").heat_capacity_cp.val for T in (900.0, 1030.0, 1043.0, 1100.0, 1300.0)}
    assert cp[1043.0] > cp[900.0] and cp[1043.0] > cp[1300.0]   # the lambda anomaly
    assert cp[1030.0] > cp[1100.0] or cp[1043.0] > 60.0


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
def test_the_pieces_in_t_join_at_tc():
    e = engine()
    lo, hi = (e.thermoPropertiesSubstance(1043.0 + d, 1e5, "Fe") for d in (-1e-6, 1e-6))
    assert lo.gibbs_energy.val == pytest.approx(hi.gibbs_energy.val, abs=1e-3)
    assert lo.entropy.val == pytest.approx(hi.entropy.val, abs=1e-3)
