# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""A cached result does not depend on the calculations made before it.

Every record is calculated in sorted order and in reverse order by engines with memoization, and once by an
engine without memoization. All three must give the same values. Before the fix, the reactants of a
reaction-defined substance (calculated with the gas pressure correction on) shared the cache with the plain
calculations: for example H2(g) at 273.15 K and 100 bar gave 5484.36 J/mol instead of 3235.96 J/mol after
Fe4(OH)8Clwn(s).
"""
import os

import pytest
import thermofun as tf

HERE = os.path.dirname(os.path.abspath(__file__))
DATABASES = ["PsiTDB2020-subset-thermofun.json", "test-thermoengine-thermofun.json"]
POINTS = [(273.15, 0.0), (273.15, 1e7), (373.15, 1e5), (473.15, 1e8)]
PROPS = ["gibbs_energy", "enthalpy", "entropy", "volume", "heat_capacity_cp"]
REACTION_PROPS = ["reaction_gibbs_energy", "reaction_enthalpy", "reaction_entropy", "reaction_volume"]


def engine(path, memoize):
    e = tf.ThermoEngine(path)
    prefs = tf.EnginePreferences()
    prefs.enable_memoize = memoize
    e.setPreferences(prefs)
    return e


def calculate(e, kind, symbols):
    call = e.thermoPropertiesSubstance if kind == "substances" else e.thermoPropertiesReaction
    props = PROPS if kind == "substances" else REACTION_PROPS
    out = {}
    for sym in symbols:
        for T, P in POINTS:
            try:
                res = call(T, P, sym)
                out[sym, T, P] = tuple(getattr(res, p).val for p in props)
            except Exception as exc:
                out[sym, T, P] = str(exc).splitlines()[0]
    return out


@pytest.mark.parametrize("name", DATABASES)
@pytest.mark.parametrize("kind", ["substances", "reactions"])
def test_results_do_not_depend_on_the_order(name, kind):
    path = os.path.join(HERE, name)
    db = tf.Database(path)
    symbols = sorted(db.getSubstancesList() if kind == "substances" else db.getReactionsList())
    if not symbols:
        pytest.skip("no " + kind)
    expected = calculate(engine(path, False), kind, symbols)
    forward = calculate(engine(path, True), kind, symbols)
    backward = calculate(engine(path, True), kind, symbols[::-1])
    for key in expected:
        assert forward[key] == expected[key], key
        assert backward[key] == expected[key], key
