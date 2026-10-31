# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# Parameter handles and fits with known answers.

import json
import math
import random
import re

import pytest
import thermofun as tf

fitting = pytest.importorskip("thermofun.fitting")
pytest.importorskip("scipy")

Tr = 298.15
TRUTH = {"G": -856433.0, "S": 41.5, "a0": 110.7, "a1": -0.00519}


def dataset():
    doc = json.loads(tf.Database("pytests/data/v2/hkf.json").toJson(readable=False))
    doc["substances"]["Q"] = {"class": "component", "aggregate_state": "crystal", "formula": "SiO2", "charge": 0, "reference_conditions": {"T": Tr, "P": 1e5},
                              "standard_state_properties": {"G": TRUTH["G"], "H": -910830.0, "S": TRUTH["S"], "Cp": 44.9, "V": 2.27e-5},
                              "methods": [{"method": "CpPolynomial", "segments": [{"t_min": 273.15, "t_max": 2273.15, "parameters": {"a0": TRUTH["a0"], "a1": TRUTH["a1"], "a2": 0.0, "a3": -1128.3, "a4": 0.0, "a5": 0.0, "a6": 0.0, "a7": 0.0, "a8": 0.0, "a9": 0.0}}]},
                                          {"method": "ConstantVolume"}]}
    return doc


def engine_of(doc=None):
    return tf.ThermoEngine(tf.Database(json.dumps(doc or dataset())))


def test_a_handle_reads_and_changes_a_value():
    engine = engine_of()
    a0 = engine.parameter("Q", "CpPolynomial", "a0")
    assert a0.name() == "Q/CpPolynomial/a0"
    assert a0.value() == pytest.approx(TRUTH["a0"])
    before = engine.thermoPropertiesSubstance(500.0, 1e5, "Q").heat_capacity_cp.val
    a0.setValue(TRUTH["a0"] + 1.0)
    assert a0.value() == TRUTH["a0"] + 1.0
    assert engine.thermoPropertiesSubstance(500.0, 1e5, "Q").heat_capacity_cp.val == pytest.approx(before + 1.0)
    # the engine has its own copy of the database
    assert engine.commitParameters().getSubstance("Q").record().methods[0].segments[0].parameters["a0"].value == TRUTH["a0"] + 1.0


def test_a_handle_on_a_reference_value_and_on_a_reaction():
    engine = engine_of()
    G = engine.parameter("Q", "", "G")
    G.setValue(-856000.0)
    assert engine.thermoPropertiesSubstance(Tr, 1e5, "Q").gibbs_energy.val == pytest.approx(-856000.0)
    reaction = "H4SiO4@"  # LogKConstant: it reads the value of log K. The substance with this symbol has handles too
    logK = next(h for h in engine.parameters(reaction) if h.key()[2] == "logK")
    logK.setValue(3.25)
    assert engine.thermoPropertiesReaction(Tr, 1e5, reaction).log_equilibrium_constant.val == pytest.approx(3.25)


def test_a_change_clears_only_the_results_that_depend_on_it():
    engine = engine_of()
    engine.thermoPropertiesSubstance(500.0, 1e5, "Q")
    engine.thermoPropertiesSubstance(500.0, 1e5, "Na+")
    engine.thermoPropertiesSubstance(500.0, 1e5, "Cl-")
    kept = engine.numberOfCachedResults()
    engine.parameter("Q", "", "G").setValue(-856000.0)
    assert engine.numberOfCachedResults() == kept - 1
    engine.parameter("Na+", "", "G").setValue(-256000.0)
    assert engine.numberOfCachedResults() == kept - 2
    # the others are still right, and the changed ones are calculated again
    assert engine.thermoPropertiesSubstance(Tr, 1e5, "Q").gibbs_energy.val == pytest.approx(-856000.0)


def test_a_change_reaches_a_substance_defined_by_a_reaction():
    doc = json.loads(tf.Database("pytests/data/v2/reactions.json").toJson(readable=False))
    engine = engine_of(doc)
    before = engine.thermoPropertiesSubstance(373.15, 1e6, "H4SiO4@").gibbs_energy.val
    species = engine.parameter("SiO2@", "", "G")
    species.setValue(species.value() + 100.0)
    # H4SiO4@ = SiO2@ + 2 H2O@
    assert engine.thermoPropertiesSubstance(373.15, 1e6, "H4SiO4@").gibbs_energy.val == pytest.approx(before + 100.0, rel=1e-9)


def test_the_list_of_parameters_leaves_out_what_cannot_be_fitted():
    engine = engine_of()
    names = [h.name() for h in engine.parameters("Q")]
    assert "Q/G" in names and "Q/CpPolynomial/a0" in names
    assert all("n_atoms" not in n for n in names)
    with pytest.raises(RuntimeError):
        engine.parameter("Q", "CpPolynomial", "zz")
    with pytest.raises(RuntimeError):
        engine.parameter("NoSuch", "", "G")


def observations(doc, noise=0.0):
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    rng = random.Random(7)
    out = []
    for T in (300.0, 400.0, 500.0, 600.0, 800.0, 1000.0):
        r = engine.thermoPropertiesSubstance(T, 1e5, "Q")
        out.append(fitting.Observation("Q", "Cp", T, 1e5, r.heat_capacity_cp.val + rng.gauss(0.0, noise), max(noise, 0.1)))
        out.append(fitting.Observation("Q", "G", T, 1e5, r.gibbs_energy.val + rng.gauss(0.0, 10.0 * noise), max(10.0 * noise, 1.0)))
    return out


KEYS = [("Q", "", "G"), ("Q", "", "S"), ("Q", "CpPolynomial", "a0"), ("Q", "CpPolynomial", "a1")]


def wrong_start():
    engine = engine_of()
    engine.parameter("Q", "", "G").setValue(TRUTH["G"] + 5000.0)
    engine.parameter("Q", "", "S").setValue(TRUTH["S"] * 1.3)
    engine.parameter("Q", "CpPolynomial", "a0").setValue(TRUTH["a0"] * 0.8)
    engine.parameter("Q", "CpPolynomial", "a1").setValue(TRUTH["a1"] * 2.0)
    return engine


def test_a_fit_recovers_known_parameters():
    result = fitting.Problem(wrong_start(), KEYS, observations(dataset())).solve()
    assert result.success
    assert result.values["Q/G"] == pytest.approx(TRUTH["G"], rel=1e-7)
    assert result.values["Q/S"] == pytest.approx(TRUTH["S"], rel=1e-6)
    assert result.values["Q/CpPolynomial/a0"] == pytest.approx(TRUTH["a0"], rel=1e-6)
    assert result.values["Q/CpPolynomial/a1"] == pytest.approx(TRUTH["a1"], rel=1e-5)
    assert result.chi2 < 1e-8


def test_the_covariance_of_a_fit_describes_the_scatter_of_noisy_data():
    noisy = fitting.Problem(wrong_start(), KEYS, observations(dataset(), noise=0.2)).solve()
    for name, value in noisy.values.items():
        truth = {"Q/G": TRUTH["G"], "Q/S": TRUTH["S"], "Q/CpPolynomial/a0": TRUTH["a0"], "Q/CpPolynomial/a1": TRUTH["a1"]}[name]
        assert abs(value - truth) < 4.0 * noisy.uncertainties[name]
    # the same covariance, from the linear theory: the problem is linear in the parameters
    import numpy
    problem = fitting.Problem(wrong_start(), KEYS, observations(dataset(), noise=0.2))
    J = numpy.array(problem.jacobian(list(noisy.values.values())))
    wanted = numpy.linalg.inv(J.T @ J)
    assert numpy.allclose(noisy.covariance, wanted, rtol=1e-5, atol=0.0)


def test_the_database_of_a_fit_keeps_the_values_and_the_covariance(tmp_path):
    result = fitting.Problem(wrong_start(), KEYS, observations(dataset(), noise=0.2)).solve()
    db = result.database()
    assert result.covariance_dropped == 0
    path = tmp_path / "fitted.json"
    db.save(str(path))
    again = tf.Database(str(path))
    record = again.getSubstance("Q").record()
    assert record.methods[0].segments[0].parameters["a0"].value == pytest.approx(result.values["Q/CpPolynomial/a0"], rel=1e-12)
    assert len(record.covariance) == 1 and len(record.covariance[0].paths) == 4
    # the propagation reads them: at the point of an observation, the error of G follows J S J^T
    engine = tf.ThermoEngine(again)
    engine.preferences.propagate_parameter_errors = True
    err = engine.thermoPropertiesSubstance(500.0, 1e5, "Q").gibbs_energy.err
    d = engine.parameterSensitivities(500.0, 1e5, "Q", KEYS)["G"]
    J = [d["Q/G"], d["Q/S"], d["Q/CpPolynomial/a0"], d["Q/CpPolynomial/a1"]]
    names = result.names
    order = [names.index(n) for n in ("Q/G", "Q/S", "Q/CpPolynomial/a0", "Q/CpPolynomial/a1")]
    wanted = sum(J[a] * result.covariance[order[a]][order[b]] * J[b] for a in range(4) for b in range(4))
    assert err == pytest.approx(math.sqrt(wanted), rel=1e-9)


def test_wrong_input_of_a_problem():
    with pytest.raises(ValueError, match="no parameters"):
        fitting.Problem(engine_of(), [], [fitting.Observation("Q", "G", 300.0, 1e5, 0.0)])
    with pytest.raises(ValueError, match="cannot fix"):
        fitting.Problem(engine_of(), KEYS, [fitting.Observation("Q", "G", 300.0, 1e5, 0.0)])
    with pytest.raises(ValueError, match="Unknown property"):
        fitting.Problem(engine_of(), KEYS[:1], [fitting.Observation("Q", "Gibbs", 300.0, 1e5, 0.0)])
    with pytest.raises(ValueError, match="above 0"):
        fitting.Problem(engine_of(), KEYS[:1], [fitting.Observation("Q", "G", 300.0, 1e5, 0.0, 0.0)])


def test_the_examples_of_the_guide_run():
    text = open("docs/how-to/fit-parameters.md").read()
    blocks = re.findall(r"```python\n(.*?)```", text, re.S)
    assert len(blocks) == 4
    scope = {}
    for block in blocks:
        exec(block, scope)
