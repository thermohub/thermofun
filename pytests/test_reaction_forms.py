# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The source forms of a method of a reaction: the tool makes the expression of dG, and the engine runs it.

import json
import math
import pathlib

import pytest

pytest.importorskip("sympy")

import thermofun as tf  # noqa: E402
from thermofun import method_tool as tool  # noqa: E402

pytestmark = pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")

R, Tr, ln10 = 8.31451, 298.15, math.log(10)
T = 400.0
LOGK, DH = 4.0, -20000.0
DG = -R * Tr * ln10 * LOGK
DS = (DH - DG) / Tr


def method(form, expression=None, parameters=None):
    source = {"form": form}
    if expression:
        source["expression"] = expression
    return tool.build({"kind": "symbolic", "tier": 1, "operates_on": "reaction", "parameter_definitions": {k: {"unit": v} for k, v in (parameters or {}).items()}, "source": source})


def reaction(definition, values=None, properties=None):
    doc = {"thermohub": "2.0.0-draft.1", "methods": {"M": definition},
           "substances": {"Y": {"formula": "Ca", "class": "component", "standard_state_properties": {"G": -1000.0, "H": -900.0, "S": 10.0, "Cp": 10.0, "V": 1e-5}},
                          "Z": {"formula": "Ca", "class": "component", "defining_reaction": "R"}},
           "reactions": {"R": {"equation": "Y = Z", "reaction_properties": properties or {}, "methods": [{"method": "M", "parameters": values or {}}]}}}
    return tf.Database(json.dumps(doc))


def at(database, P=1e5):
    return tf.ThermoEngine(database).thermoPropertiesReaction(T, P, "R")


def test_log_k_as_a_function_of_t():
    built = method("logk", "a + b/T", {"a": "1", "b": "K"})
    assert at(reaction(built, {"a": -3.0, "b": 1500.0})).log_equilibrium_constant.val == pytest.approx(-3.0 + 1500.0 / T, rel=1e-12)


def test_a_polynomial_of_log_k_gives_its_entropy_and_heat_capacity():
    built = method("logk", tool._TEMPLATES["logk"][0], tool._TEMPLATES["logk"][1])
    c = {"A0": 2.0, "A1": 0.001, "A2": -500.0, "A3": 0.5, "A4": 1e4}
    logK = lambda t: c["A0"] + c["A1"] * t + c["A2"] / t + c["A3"] * math.log10(t) + c["A4"] / t ** 2
    r = at(reaction(built, c))
    assert r.log_equilibrium_constant.val == pytest.approx(logK(T), rel=1e-12)
    h = 1e-2
    dG = lambda t: -R * t * ln10 * logK(t)
    assert r.reaction_entropy.val == pytest.approx(-(dG(T + h) - dG(T - h)) / (2 * h), rel=1e-7)
    assert r.reaction_heat_capacity_cp.val == pytest.approx(-T * (dG(T + h) - 2 * dG(T) + dG(T - h)) / h ** 2, rel=1e-4)


def test_constant_enthalpy_of_reaction():
    r = at(reaction(method("vant_hoff"), properties={"logK": LOGK, "dH": DH}))
    assert r.log_equilibrium_constant.val == pytest.approx(-(DH - T * DS) / (R * T * ln10), rel=1e-12)
    assert r.reaction_enthalpy.val == pytest.approx(DH, rel=1e-12)
    assert r.reaction_heat_capacity_cp.val == pytest.approx(0.0, abs=1e-9)


def test_a_heat_capacity_of_reaction():
    a, b = 120.0, 0.3
    built = method("drcp", "a + b*T", {"a": "J/(mol.K)", "b": "J/(mol.K2)"})
    expected = DG - DS * (T - Tr) + a * (T - Tr - T * math.log(T / Tr)) - b * (T - Tr) ** 2 / 2
    r = at(reaction(built, {"a": a, "b": b}, {"logK": LOGK, "dH": DH}))
    assert r.reaction_gibbs_energy.val == pytest.approx(expected, rel=1e-10)
    assert r.reaction_heat_capacity_cp.val == pytest.approx(a + b * T, rel=1e-10)


def test_the_entropy_comes_from_the_enthalpy_if_the_record_has_no_entropy():
    built = method("dg", "dGr - dSr*(T - Tr)")
    r = at(reaction(built, properties={"logK": LOGK, "dH": DH}))
    assert r.reaction_gibbs_energy.val == pytest.approx(DG - DS * (T - Tr), rel=1e-12)


def test_a_value_that_the_record_cannot_give_is_an_error():
    built = method("dg", "dGr - dSr*(T - Tr)")
    with pytest.raises(RuntimeError, match="uses dSr"):
        at(reaction(built, properties={"logK": LOGK}))


def test_a_form_of_a_reaction_on_a_substance_and_the_other_way_are_errors():
    with pytest.raises(tool.BuildError):
        tool.build({"kind": "symbolic", "operates_on": "reaction", "source": {"form": "cp", "expression": "a"}, "parameter_definitions": {"a": {"unit": "1"}}})
    with pytest.raises(tool.BuildError):
        tool.build({"kind": "symbolic", "operates_on": "substance", "source": {"form": "logk", "expression": "a"}, "parameter_definitions": {"a": {"unit": "1"}}})


@pytest.mark.parametrize("form", tool.REACTION_FORMS)
def test_the_template_of_each_form_builds_and_checks(form):
    built = tool.build(tool.new(form))
    assert "dG" in built["expression"]
    assert tool.check(built) == []


def test_check_runs_the_test_cases_of_a_reaction():
    built = method("logk", "a", {"a": "1"})
    built["tests"] = [{"T": 300.0, "P": 1e5, "parameters": {"a": 2.0}, "expected": {"logK": 2.0}}]
    assert tool.check(built) == []
    built["tests"][0]["expected"]["logK"] = 3.0
    assert any("V2-SYM-004" in p for p in tool.check(built))


@pytest.mark.parametrize("file", ["logk-polynomial.yaml", "drcp-linear.yaml", "vant-hoff.yaml"])
def test_the_examples_are_in_order(file):
    pytest.importorskip("yaml")
    document, _ = tool._load(str(pathlib.Path(__file__).resolve().parent.parent / "examples" / "v2" / "methods" / file))
    assert tool.check(document) == []


def test_a_large_coefficient_does_not_overflow():
    built = tool.build({"kind": "symbolic", "operates_on": "substance", "parameter_definitions": {"a": {"unit": "J/(mol.K)"}}, "source": {"form": "cp", "expression": "a"}})
    assert "**a" not in built["expression"]["G"]
    a, T2 = 500.0, 1500.0
    point = {"T": T2, "P": 1e5, "parameters": {"a": a}, "reference": {"Gr": 0.0, "Sr": 0.0}}
    assert tool.properties(built["expression"]["G"], built, point)["G"] == pytest.approx(a * (T2 - Tr - T2 * math.log(T2 / Tr)), rel=1e-10)
