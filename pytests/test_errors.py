"""Propagation of the errors (uncertainties) of the properties of substances and reactions."""
import copy
import json
import math
import os

import pytest
import thermofun as tf

HERE = os.path.dirname(os.path.abspath(__file__))
DB = os.path.join(HERE, "test-thermoengine-thermofun.json")
R = 8.31451  # R_CONSTANT of ThermoFun


def load():
    with open(DB) as f:
        return json.load(f)


def substance(data, symbol):
    return next(s for s in data["substances"] if s["symbol"] == symbol)


@pytest.fixture
def data(tmp_path):
    d = load()
    cal = substance(d, "Cal")
    cal["TPMethods"][0]["m_heat_capacity_ft_coeffs"]["errors"] = [0.5, 1e-4, 2e4, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    cal["sm_gibbs_energy"]["errors"] = [100.0]
    cal["sm_entropy_abs"]["errors"] = [0.3]
    return d


def make_engine(d, propagate, tmp_path):
    path = tmp_path / "db.json"
    path.write_text(json.dumps(d))
    eng = tf.ThermoEngine(str(path))
    eng.preferences.propagate_parameter_errors = propagate
    return eng


def calcite_G(T, a, G298, S298, Tr=298.15):
    """G(T) of the Cp = a0 + a1 T + a2 / T^2 equation."""
    a0, a1, a2 = a
    cp_int = a0 * (T - Tr) + a1 / 2 * (T**2 - Tr**2) - a2 * (1 / T - 1 / Tr)
    cp_t_int = a0 * math.log(T / Tr) + a1 * (T - Tr) - a2 / 2 * (1 / T**2 - 1 / Tr**2)
    return G298 - S298 * (T - Tr) + cp_int - T * cp_t_int


def test_default_keeps_the_errors_of_the_reference_properties(data, tmp_path):
    eng = make_engine(data, False, tmp_path)
    tps = eng.thermoPropertiesSubstance(298.15, 1e5, "Cal")
    assert tps.gibbs_energy.err == pytest.approx(100.0)
    assert tps.entropy.err == pytest.approx(0.3)


def test_parameter_errors_match_the_analytic_propagation(data, tmp_path):
    eng = make_engine(data, True, tmp_path)
    T = 450.0
    tps = eng.thermoPropertiesSubstance(T, 1e5, "Cal")

    a = [104.5163192749, 0.02192415855825, -2594080.0]
    sa = [0.5, 1e-4, 2e4]
    G298, S298 = -1129178.0, 92.675598144531
    sG, sS = 100.0, 0.3

    # first-order propagation of the independent errors, with partial derivatives of the equation
    h = 1e-6
    var = (sG * 1.0) ** 2 + (sS * (T - 298.15)) ** 2   # dG/dG298 = 1, dG/dS298 = -(T - Tr)
    for i in range(3):
        step = max(abs(a[i]), 1.0) * h
        up = [x + (step if j == i else 0.0) for j, x in enumerate(a)]
        dn = [x - (step if j == i else 0.0) for j, x in enumerate(a)]
        d = (calcite_G(T, up, G298, S298) - calcite_G(T, dn, G298, S298)) / (2 * step)
        var += (d * sa[i]) ** 2
    assert tps.gibbs_energy.err == pytest.approx(math.sqrt(var), rel=1e-3)

    # the values and derivatives do not change
    ref = make_engine(data, False, tmp_path).thermoPropertiesSubstance(T, 1e5, "Cal")
    assert tps.gibbs_energy.val == pytest.approx(ref.gibbs_energy.val)
    assert tps.gibbs_energy.ddt == pytest.approx(ref.gibbs_energy.ddt)
    assert tps.gibbs_energy.sta == ref.gibbs_energy.sta


def test_errors_grow_away_from_the_reference_state(data, tmp_path):
    eng = make_engine(data, True, tmp_path)
    e = [eng.thermoPropertiesSubstance(T, 1e5, "Cal").gibbs_energy.err for T in (298.15, 400.0, 600.0)]
    assert e[0] == pytest.approx(100.0, rel=1e-6)
    assert e[0] < e[1] < e[2]


def test_internal_energy_and_helmholtz_errors_are_weighted(data, tmp_path):
    eng = make_engine(data, False, tmp_path)
    T, P = 400.0, 2e7
    tps = eng.thermoPropertiesSubstance(T, P, "Cal")
    assert tps.helmholtz_energy.err >= tps.internal_energy.err


def reaction_data(data, nus):
    r = copy.deepcopy(data["reactions"][0])
    r["symbol"] = "test-reaction"
    r["equation"] = "test"
    r["reactants"] = [{"symbol": s, "coefficient": c} for s, c in nus.items()]
    for k in list(r.keys()):
        if k.startswith("drsm_") or k == "logKr":
            del r[k]
    data["reactions"].append(r)
    return r


def test_reaction_from_reactants_errors_are_weighted_by_the_coefficients(data, tmp_path):
    nus = {"Cal": -1.0, "Ca+2": 2.0, "CO3-2": 1.0}
    reaction_data(data, nus)
    sigma = {"Cal": 100.0, "Ca+2": 50.0, "CO3-2": 20.0}
    for s, e in sigma.items():
        if s != "Cal":
            substance(data, s)["sm_gibbs_energy"]["errors"] = [e]
    eng = make_engine(data, False, tmp_path)
    T = 298.15
    tpr = eng.thermoPropertiesReactionFromReactants(T, 1e5, "test-reaction")
    expected = math.sqrt(sum((nus[s] * sigma[s]) ** 2 for s in nus))
    assert tpr.reaction_gibbs_energy.err == pytest.approx(expected, rel=1e-9)
    assert tpr.ln_equilibrium_constant.err == pytest.approx(expected / (R * T), rel=1e-6)
    assert tpr.log_equilibrium_constant.err == pytest.approx(expected / (R * T) / math.log(10), rel=1e-6)

    # the perturbation of the parameters gives the same errors (the reaction is linear in G298)
    eng_on = make_engine(data, True, tmp_path)
    tpr_on = eng_on.thermoPropertiesReactionFromReactants(T, 1e5, "test-reaction")
    assert tpr_on.reaction_gibbs_energy.err == pytest.approx(expected, rel=1e-6)
    assert tpr_on.reaction_gibbs_energy.val == pytest.approx(tpr.reaction_gibbs_energy.val)


def test_nea_tdb_examples(data, tmp_path):
    """Examples 7 of the NEA TDB-3 guidelines (Wanner 1999): the propagation of the uncertainties in reactions and in
    ln K, log10 K (the uncertainties are those of the 95 % confidence level and are propagated linearly)."""
    # Delta_r G = 2 G(Ca+2) - G(Cal) with (-277.4 +- 4.9) and (-467.3 +- 6.2) kJ/mol -> -87.5 +- 11.6 kJ/mol
    reaction_data(data, {"Ca+2": 2.0, "Cal": -1.0})
    substance(data, "Ca+2")["sm_gibbs_energy"]["errors"] = [4900.0]
    substance(data, "Cal")["sm_gibbs_energy"]["errors"] = [6200.0]
    eng = make_engine(data, False, tmp_path)
    tpr = eng.thermoPropertiesReactionFromReactants(298.15, 1e5, "test-reaction")
    assert tpr.reaction_gibbs_energy.err == pytest.approx(math.sqrt((2 * 4900.0) ** 2 + 6200.0 ** 2), rel=1e-9)
    assert tpr.reaction_gibbs_energy.err / 1000 == pytest.approx(11.6, abs=0.05)

    # Delta_r G = -(39.46 +- 0.36) kJ/mol -> ln K = 15.92 +- 0.15, log10 K = 6.91 +- 0.06
    data2 = load()
    reaction_data(data2, {"Cal": 1.0})
    substance(data2, "Cal")["sm_gibbs_energy"]["errors"] = [360.0]
    tpr = make_engine(data2, False, tmp_path).thermoPropertiesReactionFromReactants(298.15, 1e5, "test-reaction")
    assert tpr.ln_equilibrium_constant.err == pytest.approx(0.15, abs=0.005)
    assert tpr.log_equilibrium_constant.err == pytest.approx(0.06, abs=0.005)
    assert tpr.log_equilibrium_constant.err == pytest.approx(tpr.ln_equilibrium_constant.err / math.log(10), rel=1e-5)


def test_rounding_functions_follow_the_nea_rules():
    assert tf.round_half_even(2.5, 0) == 2.0 and tf.round_half_even(3.5, 0) == 4.0   # 5 and no digits beyond: to even
    assert tf.round_half_even(2.45, 1) == pytest.approx(2.4) and tf.round_half_even(2.55, 1) == pytest.approx(2.6)
    assert tf.round_half_even(0.1251, 2) == pytest.approx(0.13)                      # other non-zero digits follow the 5
    assert tf.round_half_even(1234.0, -1) == 1230.0
    # examples of the guidelines: (25.45 +- 1.05) -> (25.4 +- 1.1); 3.478 +- 0.008; 2.8 +- 0.4; 4.85 +- 0.26
    v, e = tf.round_to_uncertainty(25.45, 1.05, 2)
    assert (v, e) == (pytest.approx(25.4), pytest.approx(1.1))
    v, e = tf.round_to_uncertainty(3.478, 0.008, 1)
    assert (v, e) == (pytest.approx(3.478), pytest.approx(0.008))
    v, e = tf.round_to_uncertainty(2.8123, 0.4, 1)
    assert (v, e) == (pytest.approx(2.8), pytest.approx(0.4))
    v, e = tf.round_to_uncertainty(4.8512, 0.26, 1)
    assert (v, e) == (pytest.approx(4.9), pytest.approx(0.3))                          # the uncertainty is rounded up
    v, e = tf.round_to_uncertainty(10.0, 0.96, 1)                                        # 0.96 -> 1
    assert (v, e) == (pytest.approx(10.0), pytest.approx(1.0))
    v, e = tf.round_to_uncertainty(1234.567, 0.0, 2)                                     # without uncertainty: unchanged
    assert (v, e) == (1234.567, 0.0)
    v, e = tf.round_to_uncertainty(-1129178.3, 100.0, 2)
    assert (v, e) == (pytest.approx(-1129180.0), pytest.approx(100.0))


def test_rounding_preference_of_the_engine(data, tmp_path):
    eng = make_engine(data, False, tmp_path)
    prefs = eng.preferences
    assert prefs.round_to_uncertainty is False and prefs.uncertainty_significant_digits == 2
    T, P = 400.0, 1e5
    plain = eng.thermoPropertiesSubstance(T, P, "Cal")

    prefs.round_to_uncertainty = True
    rounded = eng.thermoPropertiesSubstance(T, P, "Cal")
    g, e = rounded.gibbs_energy.val, rounded.gibbs_energy.err
    expected = tf.round_to_uncertainty(plain.gibbs_energy.val, plain.gibbs_energy.err, 2)
    assert (g, e) == pytest.approx(expected)
    assert g != plain.gibbs_energy.val
    # the derivatives and the properties without an error are not rounded
    assert rounded.gibbs_energy.ddt == plain.gibbs_energy.ddt
    assert rounded.volume.val == plain.volume.val

    prefs.uncertainty_significant_digits = 1
    assert eng.thermoPropertiesSubstance(T, P, "Cal").gibbs_energy.err >= e

    # reactions
    reaction_data(data, {"Ca+2": 2.0, "Cal": -1.0})
    substance(data, "Ca+2")["sm_gibbs_energy"]["errors"] = [4900.0]
    eng2 = make_engine(data, False, tmp_path)
    eng2.preferences.round_to_uncertainty = True
    tpr = eng2.thermoPropertiesReactionFromReactants(298.15, 1e5, "test-reaction")
    assert tpr.reaction_gibbs_energy.err > 0


def test_python_interface_of_the_uncertainty_options():
    prefs = tf.EnginePreferences()
    for name in ("propagate_parameter_errors", "round_to_uncertainty", "uncertainty_significant_digits"):
        assert hasattr(prefs, name)
    ps = tf.ThermoParametersSubstance()
    ps.coefficient_errors = {"eos_hkf_coeffs": [[0.1, 0.2, 0.0]]}
    assert ps.coefficient_errors["eos_hkf_coeffs"][0][1] == 0.2
    pr = tf.ThermoParametersReaction()
    pr.coefficient_errors = {"logk_ft_coeffs": [[0.01]]}
    assert pr.coefficient_errors["logk_ft_coeffs"][0][0] == 0.01


def test_coefficient_errors_are_read_from_the_records(data, tmp_path):
    make_engine(data, False, tmp_path)
    cal = tf.Database(str(tmp_path / "db.json")).getSubstance("Cal")
    errors = cal.thermoParameters().coefficient_errors
    assert errors["m_heat_capacity_ft_coeffs"][0][:3] == pytest.approx([0.5, 1e-4, 2e4])
