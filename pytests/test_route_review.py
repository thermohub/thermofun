# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The route after the review of 2026-11-23: result.route, the words of a reaction with log K, the solvent with its values at T, P.

import math

import pytest
import thermofun as tf

AQ17 = "Resources/databases/aq17-thermofun.json"


def test_a_result_has_its_route():
    e = tf.ThermoEngine(AQ17)
    r = e.thermoPropertiesSubstance(473.15, 1e7, "Ca+2")
    assert r.route is not None and str(r.route) == str(r.info.route)
    x = e.thermoPropertiesReaction(298.15, 1e5, "Calcite = Ca+2 + CO3-2")
    assert x.route.kind == "reaction" and str(x.route) == str(x.info.route)


def test_a_reaction_route_has_log_k_and_the_words_of_a_reaction():
    e = tf.ThermoEngine(AQ17)
    for T in (298.15, 473.15):
        r = e.thermoPropertiesReaction(T, 1e7, "Calcite = Ca+2 + CO3-2")
        total = r.route.total()
        assert sorted(total) == ["dCp", "dG", "dH", "dS", "dV", "logK"]
        assert total["logK"] == pytest.approx(r.log_equilibrium_constant.val, rel=1e-12)
        assert total["dG"] == pytest.approx(r.reaction_gibbs_energy.val, rel=1e-12)
        own = [line for line in str(r.route).split("\n") if line.startswith("contribution:")]   # the reaction's own line, not its reactants
        assert own and own[0].startswith("contribution: {logK: ")
    rows = e.routeTable([298.15, 373.15], 1e5, "Calcite = Ca+2 + CO3-2")
    assert "logK" in rows[0] and "dG" in rows[0] and "G" not in rows[0]


def test_the_solvent_of_a_result_has_the_values_its_methods_used():
    e = tf.ThermoEngine(AQ17)
    T, P = 473.15, 1e7
    need = e.thermoPropertiesSubstance(T, P, "Ca+2").route.needs[0]
    assert need.role == "solvent" and need.route.T == T and need.route.P == P
    eos, dielectric = need.route.methods
    assert eos.variables["density"] == pytest.approx(e.propertiesSolvent(T, P, "H2O@").density.val, rel=1e-12)
    assert dielectric.variables["epsilon"] == pytest.approx(e.electroPropertiesSolvent(T, P, "H2O@").epsilon.val, rel=1e-12)
    assert eos.contribution is None                           # the water's G is not calculated for a solute
    assert "contribution" not in str(need.route)
    # at the saturation pressure the solvent has the pressure used
    sat = e.thermoPropertiesSubstance(T, 0.0, "Ca+2").route.needs[0].route
    assert sat.P == pytest.approx(e.saturationPressure(T), rel=1e-9)
    # before calculating, no numbers
    static = e.route("Ca+2").needs[0].route
    assert static.T is None and not static.methods[0].variables
