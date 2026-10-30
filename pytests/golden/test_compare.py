# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The comparison of two results: the margin of P*V takes the rounding of P*V, and nothing more."""

import copy
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import compare_builds as cb  # noqa: E402
import golden_common as gc  # noqa: E402

P = 1e9  # Pa
V = 2478.9  # J/bar: a gas with a constant volume, for which U and P*V nearly cancel
U = -2478.9


def result(u, u_err=0.0, volume=True):
    record = {"internal_energy": [[u, 0.0, 0.0, u_err, 1]]}
    if volume:
        record["volume"] = [[V, 0.0, 0.0, 0.0, 1]]
    return {"grid": {"T": [298.15], "P": [P]}, "substances": {"Ar(g)": record}, "reactions": {}, "solvent": {}}


def diffs(expected, got):
    return gc.compare(expected, got, 1e-12)


def one_ulp_of_pv():
    return math.ulp(P / 1e5 * V)


def test_one_ulp_of_pv_is_accepted_in_the_value():
    assert one_ulp_of_pv() > 1e-12 * abs(U)
    assert not diffs(result(U), result(U + one_ulp_of_pv()))


def test_a_real_change_is_found():
    assert diffs(result(U), result(U + 8 * one_ulp_of_pv()))


def test_the_margin_needs_the_volume_and_is_only_for_the_value():
    assert diffs(result(U, volume=False), result(U + one_ulp_of_pv(), volume=False))
    assert diffs(result(U, u_err=2000.0), result(U, u_err=2000.0 + 1e-6))


def test_the_margin_is_only_for_the_energies():
    expected = result(U)
    expected["substances"]["Ar(g)"]["entropy"] = [[U, 0.0, 0.0, 0.0, 1]]
    got = copy.deepcopy(expected)
    got["substances"]["Ar(g)"]["entropy"][0][0] += one_ulp_of_pv()
    assert diffs(expected, got)


def test_an_expected_change_can_be_limited_to_one_property_and_a_size():
    key = ("db", "v", "substances", "H2O")
    limited = {key: ("heat_capacity_cv", 2e-8)}
    diff = lambda prop, rel: {"database": "db", "variant": "v", "kind": "substances", "record": "H2O", "property": prop, "rel": rel}
    assert cb.is_expected(diff("heat_capacity_cv", 1e-8), limited)
    assert not cb.is_expected(diff("heat_capacity_cv", 1e-6), limited)
    assert not cb.is_expected(diff("enthalpy", 1e-8), limited)
    assert not cb.is_expected(diff("heat_capacity_cv", None), limited)
    assert cb.is_expected(diff("enthalpy", 1.0), {key: None})
    assert not cb.is_expected(diff("enthalpy", 1e-8), {})


def test_the_count_of_rounding_differences_has_a_limit():
    assert cb.exit_code([], 45, 45) == 0
    assert cb.exit_code([], 46, 45) == 1
    assert cb.exit_code([{"record": "x"}], 0, 45) == 1
    assert cb.rounding_limit() >= 39
