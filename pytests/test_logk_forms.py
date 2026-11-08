# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The forms of log K(T) against their closed formulas at three temperatures (review of v2, B2), for each combination of
# reference values that the docs allow. Before: log K and dH gave -1.75 instead of 2 at 25 degC (dG was not derived from
# log K), log K, dH and dCp stayed constant, and a missing dH or dS was filled at T instead of Tr.
# Closed forms: Dolejs (2013) Rev. Mineral. Geochem. 76, Eqs. 36, 38 and 40.

import math

import pytest
import thermofun as tf

R, LN10, TR = 8.31451, math.log(10.0), 298.15
LOGK, DH, DCP = 2.0, 10000.0, 50.0
DG = -R * TR * LN10 * LOGK
DS = (DH - DG) / TR

SPECIES = "{formula: Na+, charge: 1, class: aqueous_solute, aggregate_state: aqueous, standard_state_properties: {G: 0, H: 0, S: 0, Cp: 0, V: 0}, methods: [{method: ConstantCp}]}"
REACTIONS = {
    "vant_hoff_logk_dh": ("{logK: 2.0, dH: 10000}", "LogKVantHoff"),
    "vant_hoff_logk_ds": ("{logK: 2.0, dS: %r}" % DS, "LogKVantHoff"),
    "vant_hoff_dg_dh": ("{dG: %r, dH: 10000}" % DG, "LogKVantHoff"),
    "vant_hoff_all": ("{logK: 2.0, dH: 10000, dS: %r}" % DS, "LogKVantHoff"),
    "constant_cp_logk_dh_dcp": ("{logK: 2.0, dH: 10000, dCp: 50}", "LogKConstantCp"),
    "constant_cp_logk_ds_dcp": ("{logK: 2.0, dS: %r, dCp: 50}" % DS, "LogKConstantCp"),
    "zero_entropy_logk": ("{logK: 2.0, dS: 0}", "LogKZeroEntropy"),
    "zero_entropy_dg": ("{dG: %r, dS: 0}" % DG, "LogKZeroEntropy"),
    "constant_logk": ("{logK: 2.0}", "LogKConstant"),
}


def vant_hoff(T):        # Eq. 38
    return LOGK - DH / (R * LN10) * (1.0 / T - 1.0 / TR)


def constant_cp(T):      # Eq. 36
    return vant_hoff(T) + DCP / (R * LN10) * (TR / T - 1.0 + math.log(T / TR))


def zero_entropy(T):     # Eq. 40: dG constant
    return LOGK * TR / T


EXPECTED = {"vant_hoff_logk_dh": vant_hoff, "vant_hoff_logk_ds": vant_hoff, "vant_hoff_dg_dh": vant_hoff, "vant_hoff_all": vant_hoff,
            "constant_cp_logk_dh_dcp": constant_cp, "constant_cp_logk_ds_dcp": constant_cp,
            "zero_entropy_logk": zero_entropy, "zero_entropy_dg": zero_entropy, "constant_logk": lambda T: LOGK}


@pytest.fixture(scope="module")
def engine():
    lines = ["thermohub: 2.0.0-draft.1", "substances:", "  A: " + SPECIES, "  B: " + SPECIES, "reactions:"]
    for name, (properties, method) in REACTIONS.items():
        lines.append("  %s: {equation: A = B, reaction_properties: %s, methods: [{method: %s}]}" % (name, properties, method))
    return tf.ThermoEngine("\n".join(lines) + "\n")


@pytest.mark.parametrize("name", sorted(REACTIONS))
@pytest.mark.parametrize("T", [TR, 373.15, 473.15])
def test_log_k_follows_its_closed_form(engine, name, T):
    r = engine.thermoPropertiesReaction(T, 1e5, name)
    assert r.log_equilibrium_constant.val == pytest.approx(EXPECTED[name](T), rel=1e-10, abs=1e-12), name
    # and the reaction properties agree with log K: dG = -R T ln10 log K, dG = dH - T dS
    assert r.reaction_gibbs_energy.val == pytest.approx(-R * T * LN10 * r.log_equilibrium_constant.val, rel=1e-10)
    if name != "constant_logk":
        assert r.reaction_gibbs_energy.val == pytest.approx(r.reaction_enthalpy.val - T * r.reaction_entropy.val, rel=1e-9, abs=1e-6)


def test_the_derivative_of_log_k_by_t_is_the_enthalpy(engine):
    """d(log K)/dT = dH / (R T^2 ln10), here for the record with log K, dH and dCp."""
    T = 373.15
    r = engine.thermoPropertiesReaction(T, 1e5, "constant_cp_logk_dh_dcp")
    assert r.log_equilibrium_constant.ddt == pytest.approx(r.reaction_enthalpy.val / (R * T * T * LN10), rel=1e-9)
    assert r.reaction_enthalpy.val == pytest.approx(DH + DCP * (T - TR), rel=1e-12)


def test_log_k_from_the_heat_capacity_polynomial_of_a_legacy_record():
    """logk_fpt_function without log K coefficients, and dr_heat_capacity_ft: the coefficients of log K come from dCp(T).
    Their integration constants were taken at the T of the call, so the dCp terms cancelled (review of v2, B2); with a
    constant dCp the result is now the closed form of Eq. 36."""
    import json
    species = {"sm_gibbs_energy": {"values": [0]}, "sm_enthalpy": {"values": [0]}, "sm_entropy_abs": {"values": [0]},
               "sm_heat_capacity_p": {"values": [0]}, "sm_volume": {"values": [0]}, "formula": "Na+", "aggregate_state": {"4": "AS_AQUEOUS"},
               "class_": {"2": "SC_AQSOLUTE"}, "TPMethods": [{"method": {"0": "cp_ft_equation"}, "limitsTP": {"lowerT": 273.15, "upperT": 2000},
                                                                "m_heat_capacity_ft_coeffs": {"values": [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]}}]}
    reaction = {"symbol": "R", "equation": "A = B", "Tst": TR, "Pst": 1e5, "logKr": {"values": [LOGK]},
                "drsm_enthalpy": {"values": [DH]}, "drsm_entropy": {"values": [DS]},
                "reactants": [{"coefficient": -1, "symbol": "A"}, {"coefficient": 1, "symbol": "B"}],
                "TPMethods": [{"method": {"0": "logk_fpt_function"}},
                              {"method": {"11": "dr_heat_capacity_ft"}, "dr_heat_capacity_ft_coeffs": {"values": [DCP, 0, 0, 0, 0]}}]}
    doc = {"substances": [dict(species, symbol="A"), dict(species, symbol="B")], "reactions": [reaction]}
    engine = tf.ThermoEngine(json.dumps(doc))
    for T in (TR, 373.15, 473.15):
        assert engine.thermoPropertiesReaction(T, 1e5, "R").log_equilibrium_constant.val == pytest.approx(constant_cp(T), rel=1e-9), T


def test_a_method_of_log_k_without_its_values_falls_back_and_is_reported():
    """LogKVantHoff with log K alone was log K = 0 at every T (dH = dS = 0); it is constant now, and validate says so."""
    text = "\n".join(["thermohub: 2.0.0-draft.1", "substances:", "  A: " + SPECIES, "  B: " + SPECIES, "reactions:",
                      "  vh: {equation: A = B, reaction_properties: {logK: 2.0}, methods: [{method: LogKVantHoff}]}",
                      "  cp: {equation: A = B, reaction_properties: {logK: 2.0, dH: 10000}, methods: [{method: LogKConstantCp}]}",
                      "  none: {equation: A = B, reaction_properties: {dH: 10000}, methods: [{method: LogKZeroEntropy}]}"]) + "\n"
    db = tf.Database(text)
    found = {(d.record, d.code) for d in db.validate().all()}
    assert {("vh", "V2-MTH-024"), ("cp", "V2-MTH-024"), ("none", "V2-MTH-024")} <= found
    engine = tf.ThermoEngine(db)
    for T in (TR, 373.15):
        assert engine.thermoPropertiesReaction(T, 1e5, "vh").log_equilibrium_constant.val == pytest.approx(LOGK)
        assert engine.thermoPropertiesReaction(T, 1e5, "cp").log_equilibrium_constant.val == pytest.approx(vant_hoff(T), rel=1e-10)
