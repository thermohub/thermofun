# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The properties of a solute method are derivatives of its G: S = -dG/dT, H = G + T S + a constant (the convention of the
# reference G), Cp = dH/dT. Checked against Dolejs (2013), Reviews in Mineralogy and Geochemistry 76, Eqs. 106-107.

import pytest
import thermofun as tf

DB = """
thermohub: 2.0.0-draft.1
substances:
  H2O@:
    formula: H2O@
    class: water_solvent
    aggregate_state: aqueous
    standard_state_properties: {G: -237183.0, H: -285881.0, S: 69.923, Cp: 75.36053, V: {value: 1.8068397, unit: J/bar}}
    methods: [{method: WaterHGK84}, {method: JohnsonNorton91}]
  anderson:
    formula: Na+
    class: aqueous_solute
    aggregate_state: aqueous
    standard_state_properties: {G: -261881, H: -240300, S: 58.4, Cp: 38.1, V: {value: -0.111, unit: J/bar}}
    methods: [{method: AndersonEtAl91}]
  holland-powell:
    formula: Na+
    class: aqueous_solute
    aggregate_state: aqueous
    standard_state_properties: {G: -261881, H: -240300, S: 58.4, Cp: 38.1, V: {value: -0.111, unit: J/bar}}
    methods: [{method: SoluteHP98, parameters: {b: 0.0}}]
"""
OFFSET = -240300 + 261881 - 298.15 * 58.4  # H - G - Tr S of the reference values


@pytest.mark.parametrize("symbol", ["anderson", "holland-powell"])
@pytest.mark.parametrize("T", [298.15, 373.15, 473.15, 573.15])
def test_the_properties_of_a_solute_are_derivatives_of_its_gibbs_energy(symbol, T):
    engine = tf.ThermoEngine(DB)
    P, h = 1e7, 1e-2
    at = lambda t: engine.thermoPropertiesSubstance(t, P, symbol)
    r, up, down = at(T), at(T + h), at(T - h)
    assert r.entropy.val == pytest.approx(-(up.gibbs_energy.val - down.gibbs_energy.val) / (2 * h), abs=1e-5)
    assert r.enthalpy.val - (r.gibbs_energy.val + T * r.entropy.val) == pytest.approx(OFFSET, abs=1e-6)
    assert r.heat_capacity_cp.val == pytest.approx((up.enthalpy.val - down.enthalpy.val) / (2 * h), rel=1e-5, abs=1e-4)


@pytest.mark.parametrize("T", [373.15, 473.15, 573.15])
def test_the_published_formulas_of_holland_and_powell_are_an_implementation(T):
    """SoluteHP98: implementation consistent (the default) and published (the paper). G and V are the same; S, H, Cp
    of the paper are not the derivatives of G (decision 4 of the review of v2)."""
    consistent = tf.ThermoEngine(DB)
    published = tf.ThermoEngine(DB)
    published.preferences.method_implementation = {"SoluteHP98": "published"}
    P, h = 1e7, 1e-2
    c, p = (e.thermoPropertiesSubstance(T, P, "holland-powell") for e in (consistent, published))
    assert p.gibbs_energy.val == pytest.approx(c.gibbs_energy.val, rel=1e-12)
    assert p.volume.val == pytest.approx(c.volume.val, rel=1e-12)
    dGdT = (published.thermoPropertiesSubstance(T + h, P, "holland-powell").gibbs_energy.val
            - published.thermoPropertiesSubstance(T - h, P, "holland-powell").gibbs_energy.val) / (2 * h)
    assert c.entropy.val == pytest.approx(-dGdT, abs=1e-5)
    # S and H of the paper differ at every T; Cp only above 500 K, where T' = 500 K and the paper lacks 2 alpha/T'
    for name in ("entropy", "enthalpy") + (("heat_capacity_cp",) if T > 500 else ()):
        assert abs(getattr(p, name).val - getattr(c, name).val) > 1e-3, name
    if T < 500:
        assert p.heat_capacity_cp.val == pytest.approx(c.heat_capacity_cp.val, rel=1e-12)


def test_the_default_implementation_of_holland_and_powell_is_consistent():
    default, named = tf.ThermoEngine(DB), tf.ThermoEngine(DB)
    named.preferences.method_implementation = {"SoluteHP98": "consistent"}
    a, b = (e.thermoPropertiesSubstance(473.15, 1e7, "holland-powell") for e in (default, named))
    assert (a.entropy.val, a.enthalpy.val) == (b.entropy.val, b.enthalpy.val)
