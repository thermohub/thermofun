# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# Symbolic (Tier-1) methods of Method records: a base and an increment, evaluated from their expression.
# These tests need a build with TFUN_USE_SYMENGINE=ON.

import json
import math

import pytest

import thermofun as tf

pytestmark = pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")

TR, PR = 298.15, 1e5

# Cp = a + b T + c / T**2 integrated from Tr: G = Gr - Sr (T - Tr) + int Cp dT - T int Cp/T dT
MAIER_KELLEY = ("Gr - Sr*(T - Tr) + a*(T - Tr - T*log(T/Tr)) - b*(T - Tr)**2/2 - c*(T - Tr)**2/(2*T*Tr**2)")

SYMBOLIC_CP = {
    "kind": "symbolic", "tier": 1, "operates_on": "substance", "contributes_to": ["G", "H", "S", "Cp"],
    "parameter_definitions": {"a": {"unit": "J/(mol.K)"}, "b": {"unit": "J/(mol.K2)"}, "c": {"unit": "J.K/mol"}},
    "expression": {"G": MAIER_KELLEY},
}
INCREMENT = {
    "kind": "symbolic", "tier": 1, "operates_on": "substance", "contributes_to": ["G", "H", "S", "Cp", "V"],
    "parameter_definitions": {"dv": {"unit": "m3/mol"}},
    "expression": {"G_increment": "dv*(P - Pr)"},
}
COEFFICIENTS = {"a": 80.0, "b": 0.012, "c": -1.1e6}


def record(methods):
    return {"formula": "CaCO3", "charge": 0, "class": "component", "aggregate_state": "crystal",
            "reference_conditions": {"T": TR, "P": PR},
            "standard_state_properties": {"G": -1.2e6, "H": -1.2e6 + TR * 90.0, "S": 90.0, "Cp": 80.0, "V": 3e-5},
            "methods": methods}


def engine(methods, definitions=None):
    doc = {"thermohub": "2.0.0-draft.1", "substances": {"X": record(methods)}}
    if definitions:
        doc["methods"] = definitions
    e = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    p = e.preferences
    p.enable_memoize = False
    e.setPreferences(p)
    return e


def props(e, T, P=PR):
    p = e.thermoPropertiesSubstance(T, P, "X")
    return {"G": p.gibbs_energy, "H": p.enthalpy, "S": p.entropy, "Cp": p.heat_capacity_cp, "V": p.volume}


CP = {"method": "CpIntegration", "segments": [{"t_min": 273.15, "t_max": 2000.0, "parameters": {"a0": 80.0, "a1": 0.012, "a2": -1.1e6}}]}


@pytest.mark.parametrize("T", [300.0, 450.0, 800.0, 1500.0])
def test_a_symbolic_cp_integral_agrees_with_the_coded_method(T):
    coded = props(engine([CP]), T)
    symbolic = props(engine([{"method": "MaierKelleyCp", "parameters": COEFFICIENTS}], {"MaierKelleyCp": SYMBOLIC_CP}), T)
    for name in ("G", "H", "S", "Cp"):
        assert symbolic[name].val == pytest.approx(coded[name].val, rel=1e-10), name
        assert symbolic[name].ddt == pytest.approx(coded[name].ddt, rel=1e-8, abs=1e-8), name


def test_the_derivatives_of_a_symbolic_method_are_consistent():
    p = props(engine([{"method": "MaierKelleyCp", "parameters": COEFFICIENTS}], {"MaierKelleyCp": SYMBOLIC_CP}), 600.0)
    assert p["G"].ddt == pytest.approx(-p["S"].val, rel=1e-12)
    assert p["S"].ddt == pytest.approx(p["Cp"].val / 600.0, rel=1e-9)


def test_a_symbolic_increment_adds_to_a_coded_method():
    base = props(engine([CP]), 500.0, 5e7)
    both = props(engine([CP, {"method": "VolumeStep", "parameters": {"dv": 1e-6}}], {"VolumeStep": INCREMENT}), 500.0, 5e7)
    assert both["G"].val - base["G"].val == pytest.approx(1e-6 * (5e7 - PR), rel=1e-9)
    assert both["H"].val - base["H"].val == pytest.approx(1e-6 * (5e7 - PR), rel=1e-9)
    assert both["S"].val == pytest.approx(base["S"].val, rel=1e-12)
    assert both["V"].val - base["V"].val == pytest.approx(1e-6 * 1e5, rel=1e-9)   # the volume is in J/bar


def test_the_order_of_the_methods_does_not_change_the_result():
    one = engine([CP, {"method": "VolumeStep", "parameters": {"dv": 1e-6}}], {"VolumeStep": INCREMENT})
    two = engine([{"method": "VolumeStep", "parameters": {"dv": 1e-6}}, CP], {"VolumeStep": INCREMENT})
    a, b = props(one, 700.0, 2e8), props(two, 700.0, 2e8)
    assert {k: v.val for k, v in a.items()} == {k: v.val for k, v in b.items()}


def test_a_bad_expression_is_reported():
    for text, code in [("a + nothing", "V2-SYM-001"), ("a +", "V2-SYM-002"), ("sin(T)", "V2-SYM-001")]:
        definition = dict(SYMBOLIC_CP, expression={"G": text})
        doc = {"thermohub": "2.0.0-draft.1", "methods": {"MaierKelleyCp": definition},
               "substances": {"X": record([{"method": "MaierKelleyCp", "parameters": COEFFICIENTS}])}}
        assert any(d.code == code for d in tf.Database(json.dumps(doc)).validate()), text


def validate_codes(definition):
    doc = {"thermohub": "2.0.0-draft.1", "methods": {"M": definition}, "substances": {"X": record([{"method": "M", "parameters": COEFFICIENTS}])}}
    return [d.code for d in tf.Database(json.dumps(doc)).validate()]


def test_a_method_in_order_has_no_problem():
    codes = validate_codes(dict(SYMBOLIC_CP, source={"form": "gibbs", "expression": MAIER_KELLEY}))
    assert not [c for c in codes if c.startswith("V2-SYM")]


def test_a_source_changed_after_the_build_is_reported():
    definition = dict(SYMBOLIC_CP, source={"form": "gibbs", "expression": MAIER_KELLEY}, source_hash="sha256:0000")
    assert "V2-SYM-003" in validate_codes(definition)


def test_a_failing_test_case_is_reported():
    case = {"T": 500.0, "P": 1e5, "parameters": COEFFICIENTS, "reference": {"Gr": -1.2e6, "Sr": 90.0}, "expected": {"G": 1.0}}
    assert "V2-SYM-004" in validate_codes(dict(SYMBOLIC_CP, tests=[case]))


def test_a_test_case_in_order_is_not_reported():
    def G(T):
        a, b, c = COEFFICIENTS["a"], COEFFICIENTS["b"], COEFFICIENTS["c"]
        return -1.2e6 - 90.0 * (T - TR) + a * (T - TR - T * math.log(T / TR)) - b * (T - TR) ** 2 / 2 - c * (T - TR) ** 2 / (2 * T * TR ** 2)
    case = {"T": 500.0, "P": 1e5, "parameters": COEFFICIENTS, "reference": {"Gr": -1.2e6, "Sr": 90.0}, "expected": {"G": G(500.0)}}
    assert "V2-SYM-004" not in validate_codes(dict(SYMBOLIC_CP, tests=[case]))


def test_an_expression_that_is_not_an_energy_is_reported():
    assert "V2-SYM-005" in validate_codes(dict(SYMBOLIC_CP, expression={"G": "a*b*(T - Tr)"}))


def test_the_derivative_by_a_parameter_is_exact():
    e = engine([{"method": "MaierKelleyCp", "parameters": COEFFICIENTS}], {"MaierKelleyCp": SYMBOLIC_CP})
    T = 500.0
    got = e.parameterSensitivities(T, 1e5, "X", [("X", "MaierKelleyCp", "a")])["G"]["X/MaierKelleyCp/a"]
    assert got == pytest.approx(T - TR - T * math.log(T / TR), rel=1e-12)
