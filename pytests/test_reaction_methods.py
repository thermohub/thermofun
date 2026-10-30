# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The reaction methods alone, with no database and no engine, and the rules of the reaction-defined substances.

import json
import math
import pathlib

import pytest
import thermofun as tf

HERE = pathlib.Path(__file__).resolve().parent
R = 8.31451
LN10 = 2.302585093
CELSIUS = [0.01, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0]
TEMPERATURES = [c + 273.15 for c in CELSIUS]


def function(T):  # a smooth log K(T) from which the table is taken
    return -8.0 + 0.05 * (T - 298.15) - 0.0002 * (T - 298.15) ** 2 + 3000.0 / T - 3000.0 / 298.15


TABLE = dict(temperatures=TEMPERATURES, logK=[function(T) for T in TEMPERATURES])
FORMS = [0, 3]
IDS = ["spline", "lagrange"]
GEMS = json.loads((HERE / "data" / "sources" / "gems-lagrange.json").read_text())


@pytest.mark.parametrize("order", FORMS, ids=IDS)
def test_the_table_values_are_reproduced_exactly(order):
    table = tf.methods.reactionMethod("LogKTable", lagrange_order=order, **TABLE)
    for T in TEMPERATURES:
        assert table.calculate(T, 1e6).log_equilibrium_constant.val == pytest.approx(function(T), abs=1e-9)


@pytest.mark.parametrize("order,tolerance", zip(FORMS, [5e-3, 5e-3]), ids=IDS)
def test_between_the_points_the_interpolation_is_close_to_the_function(order, tolerance):
    table = tf.methods.reactionMethod("LogKTable", lagrange_order=order, **TABLE)
    for T in (310.0, 400.0, 480.0):
        assert table.calculate(T, 1e6).log_equilibrium_constant.val == pytest.approx(function(T), abs=tolerance)


def test_the_lagrange_polynomial_reproduces_a_polynomial():
    points = [300.0, 350.0, 400.0, 450.0]
    cubic = lambda T: 1.0 + 0.1 * T - 1e-3 * T**2 + 2e-6 * T**3
    table = tf.methods.reactionMethod("LogKTable", lagrange_order=3, temperatures=points, logK=[cubic(T) for T in points])
    assert table.calculate(375.0, 1e5).log_equilibrium_constant.val == pytest.approx(cubic(375.0), rel=1e-11)


@pytest.mark.parametrize("row", GEMS["rows"], ids=lambda r: f"order{r['order']}-{r['T']}K")
def test_the_lagrange_polynomial_agrees_with_gems(row):
    y = [function(T) for T in GEMS["temperatures"]]
    table = tf.methods.reactionMethod("LogKTable", lagrange_order=row["order"], temperatures=GEMS["temperatures"], logK=y)
    assert table.calculate(row["T"], 1e5).log_equilibrium_constant.val == pytest.approx(row["logK"], rel=1e-12)


@pytest.mark.parametrize("order", FORMS, ids=IDS)
def test_the_other_properties_follow_from_the_derivatives(order):
    table = tf.methods.reactionMethod("LogKTable", lagrange_order=order, **TABLE)
    T, h = 400.0, 1e-3
    p = table.calculate(T, 1e6)
    dlogk = (table.calculate(T + h, 1e6).log_equilibrium_constant.val - table.calculate(T - h, 1e6).log_equilibrium_constant.val) / (2 * h)
    assert p.reaction_gibbs_energy.val == pytest.approx(-R * T * LN10 * p.log_equilibrium_constant.val, rel=1e-9)
    assert p.reaction_enthalpy.val == pytest.approx(R * LN10 * T * T * dlogk, rel=1e-6)
    assert p.reaction_entropy.val == pytest.approx((p.reaction_enthalpy.val - p.reaction_gibbs_energy.val) / T, rel=1e-8)
    assert p.reaction_gibbs_energy.ddt == pytest.approx(-p.reaction_entropy.val, rel=1e-8)
    assert p.reaction_enthalpy.ddt == pytest.approx(p.reaction_heat_capacity_cp.val, rel=1e-8)


def test_outside_the_table_is_an_error():
    with pytest.raises(RuntimeError, match="outside the table"):
        tf.methods.reactionMethod("LogKTable", **TABLE).calculate(700.0, 1e6)


def test_a_table_method_without_a_table_does_nothing():
    incoming = tf.ThermoPropertiesReaction()
    incoming.log_equilibrium_constant.val = 3.0
    assert tf.methods.reactionMethod("LogKTable").calculate(300.0, 1e5, incoming).log_equilibrium_constant.val == 3.0


LOGK = {"a0": 2.0, "a2": -500.0}
REFERENCE = {"logK": 0.5, "dG": -0.5 * R * 298.15 * LN10, "dH": 1000.0, "dS": 10.0, "dCp": 5.0, "dV": 1e-5}


def test_the_polynomial_of_log_k():
    p = tf.methods.reactionMethod("LogKPolynomial", parameters=LOGK, reference=REFERENCE).calculate(400.0, 1e5)
    assert p.log_equilibrium_constant.val == pytest.approx(2.0 - 500.0 / 400.0, rel=1e-12)


def test_the_constant_form_of_log_k():
    p = tf.methods.reactionMethod("LogKConstant", reference=REFERENCE).calculate(400.0, 1e5)
    assert p.log_equilibrium_constant.val == pytest.approx(0.5, rel=1e-12)


def test_the_constant_volume_of_reaction_adds_to_log_k():
    base = tf.methods.reactionMethod("LogKConstant", reference=REFERENCE).calculate(400.0, 1e7)
    p = tf.methods.reactionMethod("ReactionConstantVolume", reference=REFERENCE).calculate(400.0, 1e7, base)
    # V in J/bar, dP in bar
    assert p.log_equilibrium_constant.val == pytest.approx(0.5 - 1.0 * 99.0 / (R * 400.0) / LN10, rel=1e-9)
    assert p.reaction_gibbs_energy.val == pytest.approx(base.reaction_gibbs_energy.val + 1.0 * 99.0, rel=1e-9)


def test_the_density_models_say_what_is_missing():
    with pytest.raises(RuntimeError, match="needs the state of the solvent"):
        tf.methods.reactionMethod("MarshallFranck81", parameters={"a": 1.0}).calculate(400.0, 1e7)


def substance(symbol, reaction=None):
    record = {"symbol": symbol, "name": symbol, "formula": "Na+", "formula_charge": 1, "class_": {"2": "SC_AQSOLUTE"}, "aggregate_state": {"4": "AS_AQUEOUS"},
              "Tst": 298.15, "Pst": 100000, "TPMethods": [], "sm_gibbs_energy": {"values": [-1000.0], "units": ["J/mol"]},
              "sm_enthalpy": {"values": [-2000.0], "units": ["J/mol"]}, "sm_entropy_abs": {"values": [10.0], "units": ["J/(mol*K)"]},
              "sm_heat_capacity_p": {"values": [5.0], "units": ["J/(mol*K)"]}, "sm_volume": {"values": [1.0], "units": ["J/bar"]}}
    if reaction:
        record["reaction"] = reaction
    return record


def reaction(symbol, reactants):
    return {"symbol": symbol, "name": symbol, "equation": "", "reactants": [{"symbol": s, "coefficient": c} for s, c in reactants],
            "limitsTP": {"lowerT": 273.15, "upperT": 1000, "range": False}, "Tst": 298.15, "Pst": 100000,
            "TPMethods": [{"method": {"4": "logk_1_term_extrap0"}}], "logKr": {"values": [1.0], "errors": [0]}}


def test_a_cycle_of_defining_reactions_names_the_cycle():
    doc = {"elements": [], "substances": [substance("A", "rA"), substance("B", "rB")],
           "reactions": [reaction("rA", [("A", 1.0), ("B", -1.0)]), reaction("rB", [("B", 1.0), ("A", -1.0)])]}
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    with pytest.raises(RuntimeError, match="cycle of defining reactions: .*A -> B -> A|cycle of defining reactions: .*B -> A -> B"):
        engine.thermoPropertiesSubstance(300.0, 1e5, "A")


def test_after_a_cycle_the_engine_still_works():
    doc = {"elements": [], "substances": [substance("A", "rA"), substance("B", "rB"), substance("C")],
           "reactions": [reaction("rA", [("A", 1.0), ("B", -1.0)]), reaction("rB", [("B", 1.0), ("A", -1.0)])]}
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    with pytest.raises(RuntimeError):
        engine.thermoPropertiesSubstance(300.0, 1e5, "A")
    assert math.isfinite(engine.thermoPropertiesSubstance(300.0, 1e5, "C").gibbs_energy.val)
