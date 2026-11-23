# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The propagation of the uncertainties: u^2 = J S J^T, against an analytic result with a covariance, and against Monte Carlo.

import copy
import json
import math
import random

import pytest
import thermofun as tf

R = 8.31451
Tr = 298.15


def mineral(**more):
    return {"class": "component", "aggregate_state": "crystal", "formula": "SiO2", "charge": 0, "reference_conditions": {"T": Tr, "P": 1e5},
            "standard_state_properties": {"G": {"value": -856433.0, "uncertainty": 100.0}, "H": -910830.0, "S": {"value": 41.5, "uncertainty": 0.3}, "Cp": 44.9, "V": 2.27e-5},
            "methods": [{"method": "CpPolynomial", "segments": [{"t_min": 273.15, "t_max": 2273.15, "parameters": {"a0": {"value": 110.7, "uncertainty": 0.5}, "a1": -0.00519, "a2": 0.0, "a3": -1128.3, "a4": 0.0, "a5": 0.0, "a6": 0.0, "a7": 0.0, "a8": 0.0, "a9": 0.0}}]},
                        {"method": "ConstantVolume"}], **more}


def engine_of(doc, propagate=True):
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    engine.preferences.propagate_parameter_errors = propagate
    return engine


def dataset(**substances):
    base = json.loads(tf.Database("pytests/data/v2/hkf.json").toJson(readable=False))
    base["substances"].update(substances)
    return base


def test_independent_values_add_in_quadrature():
    T = 600.0
    result = engine_of(dataset(Q=mineral())).thermoPropertiesSubstance(T, 1e5, "Q")
    dG_da0 = (T - Tr) - T * math.log(T / Tr)
    assert result.gibbs_energy.err == pytest.approx(math.sqrt(100.0 ** 2 + ((T - Tr) * 0.3) ** 2 + (dG_da0 * 0.5) ** 2), rel=1e-10)
    assert result.entropy.err == pytest.approx(math.sqrt(0.3 ** 2 + (math.log(T / Tr) * 0.5) ** 2), rel=1e-10)


@pytest.mark.parametrize("correlation", [-0.8, 0.0, 0.9])
def test_a_covariance_gives_the_analytic_result(correlation):
    T = 600.0
    sG, sS = 100.0, 0.3
    record = mineral()
    record["standard_state_properties"]["S"]["uncertainty"] = sS
    record["methods"][0]["segments"][0]["parameters"]["a0"] = 110.7
    cov = correlation * sG * sS
    record["covariances"] = [{"paths": ["/standard_state_properties/G", "/standard_state_properties/S"], "matrix": [[sG * sG, cov], [cov, sS * sS]]}]
    result = engine_of(dataset(Q=record)).thermoPropertiesSubstance(T, 1e5, "Q")
    dT = T - Tr
    wanted = math.sqrt(sG ** 2 + (dT * sS) ** 2 - 2.0 * dT * cov)  # G = G0 - S0 dT + ...
    assert result.gibbs_energy.err == pytest.approx(wanted, rel=1e-10)


def monte_carlo(doc, kind, symbol, T, P, field, inputs, samples=3000):
    """The standard deviation of a result over samples of the inputs. inputs: functions that give the holder and key in a document, with u."""
    rng = random.Random(20261031)
    values = []
    for _ in range(samples):
        sample = copy.deepcopy(doc)
        for holder_of, u in inputs:
            holder, key = holder_of(sample)
            if isinstance(holder[key], dict):
                holder[key]["value"] += rng.gauss(0.0, u)
            else:
                holder[key] += rng.gauss(0.0, u)
        engine = tf.ThermoEngine(tf.Database(json.dumps(sample)))
        r = engine.thermoPropertiesSubstance(T, P, symbol) if kind == "substance" else engine.thermoPropertiesReaction(T, P, symbol)
        values.append(getattr(r, field).val)
    mean = sum(values) / len(values)
    return math.sqrt(sum((v - mean) ** 2 for v in values) / (len(values) - 1))


def small(symbols):
    """Only the records that a calculation needs: each sample loads the file."""
    base = json.loads(tf.Database("pytests/data/v2/hkf.json").toJson(readable=False))
    base["substances"] = {k: v for k, v in base["substances"].items() if k in symbols}
    base["reactions"] = {}
    return base


def test_monte_carlo_for_a_mineral():
    doc = small({"H2O@"})
    doc["substances"]["Q"] = mineral()
    props = lambda d: d["substances"]["Q"]["standard_state_properties"]
    a0 = lambda d: (d["substances"]["Q"]["methods"][0]["segments"][0]["parameters"], "a0")
    inputs = [(lambda d: (props(d), "G"), 100.0), (lambda d: (props(d), "S"), 0.3), (a0, 0.5)]
    exact = engine_of(doc).thermoPropertiesSubstance(700.0, 1e7, "Q").gibbs_energy.err
    assert monte_carlo(doc, "substance", "Q", 700.0, 1e7, "gibbs_energy", inputs) == pytest.approx(exact, rel=0.04)


def test_monte_carlo_for_a_solute():
    doc = small({"H2O@", "Na+"})
    record = doc["substances"]["Na+"]
    method = next(m for m in record["methods"] if m["method"] == "HKF88")
    # 1 % of the values
    relative = {"a1": 0.01, "omega": 0.01, "c1": 0.01}
    for name, share in relative.items():
        value = method["parameters"][name]
        number = value["value"] if isinstance(value, dict) else value
        method["parameters"][name] = {"value": number, "uncertainty": abs(number) * share}
    index = record["methods"].index(method)
    inputs = [(lambda d, n=name: (d["substances"]["Na+"]["methods"][index]["parameters"], n), method["parameters"][name]["uncertainty"]) for name in relative]
    exact = engine_of(doc).thermoPropertiesSubstance(473.15, 5e7, "Na+").gibbs_energy.err
    assert exact > 0.0
    assert monte_carlo(doc, "substance", "Na+", 473.15, 5e7, "gibbs_energy", inputs) == pytest.approx(exact, rel=0.04)


def test_monte_carlo_for_a_substance_defined_by_a_reaction():
    doc = json.loads(tf.Database("pytests/data/v2/reactions.json").toJson(readable=False))
    symbol = "H4SiO4@"
    reaction = doc["reactions"][symbol]
    reaction["reaction_properties"]["logK"] = {"value": 2.0, "uncertainty": 0.05}
    for name in ("dG", "dH", "dS", "dCp", "dV"):
        reaction["reaction_properties"].pop(name, None)
    reaction["methods"] = [{"method": "LogKConstant"}]
    species = doc["substances"]["SiO2@"]["standard_state_properties"]
    species["S"] = {"value": species["S"]["value"] if isinstance(species["S"], dict) else species["S"], "uncertainty": 2.0}
    species["G"] = {"value": species["G"]["value"] if isinstance(species["G"], dict) else species["G"], "uncertainty": 300.0}
    doc["substances"] = {k: v for k, v in doc["substances"].items() if k in (symbol, "SiO2@", "H2O@")}
    doc["reactions"] = {symbol: reaction}
    for holder in (doc["substances"][symbol]["standard_state_properties"],):
        for name, value in holder.items():
            if isinstance(value, dict):
                value["uncertainty"] = 0.0
    inputs = [(lambda d: (d["reactions"][symbol]["reaction_properties"], "logK"), 0.05),
              (lambda d: (d["substances"]["SiO2@"]["standard_state_properties"], "G"), 300.0),
              (lambda d: (d["substances"]["SiO2@"]["standard_state_properties"], "S"), 2.0)]
    exact = engine_of(doc).thermoPropertiesSubstance(373.15, 1e6, symbol).gibbs_energy.err
    assert exact > 0.0
    assert monte_carlo(doc, "substance", symbol, 373.15, 1e6, "gibbs_energy", inputs) == pytest.approx(exact, rel=0.04)
