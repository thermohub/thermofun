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
