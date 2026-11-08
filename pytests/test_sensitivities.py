# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The derivatives by the parameters against central finite differences, for each parameter of each record of the test files, through
# reactions and substances defined by a reaction.

import copy
import glob
import json
import math
import os

import pytest
import thermofun as tf

FILES = sorted(glob.glob("pytests/data/v2/*.json")) + sorted(glob.glob("pytests/data/legacy-synthetic/*.json"))
POINTS = [(298.15, 1e5), (450.0, 2e7), (623.15, 1e8)]
STEP = 1e-4
SUBSTANCE = {"G": "gibbs_energy", "H": "enthalpy", "S": "entropy", "Cp": "heat_capacity_cp", "V": "volume"}
REACTION = {"logK": "log_equilibrium_constant", "dG": "reaction_gibbs_energy", "dH": "reaction_enthalpy", "dS": "reaction_entropy", "dCp": "reaction_heat_capacity_cp", "dV": "reaction_volume"}
# the engine gives V in J/bar, the derivatives are SI
FACTOR = {}  # the public results and the sensitivities both give V in m3/mol
# the size of the noise of S and H of Birch-Murnaghan: see test_the_entropy_of_birch_murnaghan_with_a_large_step
NOISE = {"S": 1e-7, "H": 5e-5}


def numeric(holder, key):
    """The number in a value of a file, with or without an uncertainty."""
    x = holder[key]
    return x["value"] if isinstance(x, dict) else x


def change(holder, key, factor):
    if isinstance(holder[key], dict):
        holder[key]["value"] *= factor
    else:
        holder[key] *= factor


def parameters_of(section, symbol, record):
    """Each parameter of a record that has a value: (reference, method, name, holder path)."""
    out = []
    values = record.get("standard_state_properties" if section == "substances" else "reaction_properties", {})
    for name in values:
        if numeric(values, name):
            out.append(("", name, lambda doc, n=name, key="standard_state_properties" if section == "substances" else "reaction_properties": (doc[section][symbol][key], n)))
    for i, method in enumerate(record.get("methods", [])):
        for name, value in method.get("parameters", {}).items():
            if not isinstance(value, (int, float, dict)) or not numeric(method["parameters"], name):
                continue
            out.append((method["method"], name, lambda doc, i=i, n=name: (doc[section][symbol]["methods"][i]["parameters"], n)))
        for k, segment in enumerate(method.get("segments", [])):
            for name, value in segment.get("parameters", {}).items():
                if numeric(segment["parameters"], name):
                    label = name if k == 0 else f"{name}[{k}]"
                    out.append((method["method"], label, lambda doc, i=i, k=k, n=name: (doc[section][symbol]["methods"][i]["segments"][k]["parameters"], n)))
        for k, row in enumerate(method.get("transitions", [])):
            for name in ("dS", "dH", "dV"):
                if name in row and numeric(row, name):
                    out.append((method["method"], f"{name}[{k}]", lambda doc, i=i, k=k, n=name: (doc[section][symbol]["methods"][i]["transitions"][k], n)))
    return out


def properties(doc, kind, T, P, symbol):
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    if kind == "substance":
        r = engine.thermoPropertiesSubstance(T, P, symbol)
        return {k: getattr(r, f).val * FACTOR.get(k, 1.0) for k, f in SUBSTANCE.items()}
    r = engine.thermoPropertiesReaction(T, P, symbol)
    return {k: getattr(r, f).val * FACTOR.get(k, 1.0) for k, f in REACTION.items()}


def si_value(db, section, symbol, owner, name):
    """The value of a parameter in SI, from the record."""
    record = db.getSubstance(symbol).record() if section == "substances" else db.getReaction(symbol).record()
    if not owner:
        values = record.standard_state_properties if section == "substances" else record.reaction_properties
        return next(q.value for key, q in values.items() if str(key).split(".")[-1] == name)
    method = next(m for m in record.methods if m.method == owner)
    if name[:2] in ("dS", "dH", "dV") and method.transitions:
        return method.transitions[int(name[3:-1])][name[:2]].value
    if method.segments:
        base, _, k = name.partition("[")
        return method.segments[int(k[:-1]) if k else 0].parameters[base].value
    return method.parameters[name].value


def check(doc, kind, T, P, symbol, ref, holder_of, exact, theta, noise={}):
    """exact: the derivatives by the parameter, for one (T, P). theta: its value in SI. The file can have other units: a relative step is the same in all."""
    up, down = copy.deepcopy(doc), copy.deepcopy(doc)
    change(*holder_of(up), 1.0 + STEP)
    change(*holder_of(down), 1.0 - STEP)
    a, b = properties(up, kind, T, P, symbol), properties(down, kind, T, P, symbol)
    problems = []
    for name in a:
        difference = (a[name] - b[name]) / (2.0 * STEP)  # dX / dln(theta)
        if not (math.isfinite(difference) and math.isfinite(exact[name])):
            continue
        wanted = exact[name] * theta
        if abs(difference - wanted) > 3e-5 * abs(difference) + 1e-11 * abs(a[name]) / STEP + noise.get(name, 0.0) / STEP:
            problems.append((symbol, ref, name, T, P, "difference", difference, "exact", wanted))
    return problems


def bad_points(path, only=None):
    """The differences between the derivatives and the finite differences. only: just the records with one of these methods."""
    db = tf.Database(path)
    return bad_points_of(db, json.loads(db.toJson(readable=False)), only)


def bad_points_of(db, doc, only=None, one=None):
    problems = []
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    for section, kind, method in (("substances", "substance", engine.parameterSensitivities), ("reactions", "reaction", engine.reactionParameterSensitivities)):
        for symbol, record in doc.get(section, {}).items():
            if record.get("class") == "water_solvent":
                continue
            if only and not any(m["method"] in only for m in record.get("methods", [])):
                continue
            if one and symbol != one:
                continue
            for owner, name, holder_of in parameters_of(section, symbol, record):
                ref = (symbol, owner, name)
                theta = si_value(db, section, symbol, owner, name)
                for T, P in POINTS:
                    try:
                        derivatives = method(T, P, symbol, [ref])
                    except RuntimeError as error:
                        if "Cannot derive" in str(error):
                            problems.append((symbol, ref, T, P, str(error)[:200]))
                        continue  # the model does not cover this point
                    exact = {k: v[label(ref)] for k, v in derivatives.items()}
                    noise = NOISE if any(m["method"] == "BirchMurnaghan47" for m in record.get("methods", [])) else {}
                    problems += check(doc, kind, T, P, symbol, ref, holder_of, exact, theta, noise)
    return problems


# The methods that no record of the files above has with parameters
SOLUTE_REFERENCE = {"G": -157220.0, "H": -230020.0, "S": -10.71, "Cp": -137.2, "V": -0.418e-5}
MINERAL = {"class": "component", "aggregate_state": "crystal", "formula": "SiO2", "charge": 0, "reference_conditions": {"T": 298.15, "P": 100000.0},
           "standard_state_properties": {"G": -856433.0, "H": -910830.0, "S": 41.5, "Cp": 44.9, "V": 2.27e-5}}
CP = {"method": "CpPolynomial", "segments": [{"t_min": 273.15, "t_max": 2273.15, "parameters": {"a0": 110.7, "a1": -0.00519, "a2": 0.0, "a3": -1128.3, "a4": 0.0, "a5": 0.0, "a6": 0.0, "a7": 0.0, "a8": 0.0, "a9": 0.0}}]}
HKF = {"a1": 7.69e-06, "a2": -956.0, "a3": 0.000136, "a4": -114055.0, "c1": 76.1, "c2": -124725.0, "omega": 138323.0}
ADDED_SUBSTANCES = [
    ("OH-hp", dict(formula="OH-", charge=-1, **{"class": "aqueous_solute"}), SOLUTE_REFERENCE, [{"method": "SoluteHP98", "parameters": {"b": 1.0}}]),
    ("OH-an", dict(formula="OH-", charge=-1, **{"class": "aqueous_solute"}), SOLUTE_REFERENCE, [{"method": "AndersonEtAl91"}]),
    ("OH-gems", dict(formula="OH-", charge=-1, **{"class": "aqueous_solute"}), SOLUTE_REFERENCE, [{"method": "HKF88", "implementation": "gems", "parameters": HKF}]),
    ("Q-l11", {}, MINERAL["standard_state_properties"], [CP, {"method": "LandauHP11", "parameters": {"Tc0": 847.0, "Smax": 4.95, "Vmax": 1.188e-6}}]),
    ("Q-tait", {}, MINERAL["standard_state_properties"], [CP, {"method": "TaitHP11", "parameters": {"kappa0p": 4.0, "kappa0pp": -5e-11, "n_atoms": 3, "alpha0": 6.5e-6, "kappa0": 7.5e10}}]),
    ("OH-uni", dict(formula="OH-", charge=-1, **{"class": "aqueous_solute"}), SOLUTE_REFERENCE, [{"method": "CpExtendedUNIQUAC", "parameters": {"a": -200.0, "b": 0.5, "c": 30000.0, "alpha": 1e-10, "beta": -1e-19, "Theta": 200.0}}]),
    ("Q-tr", {}, MINERAL["standard_state_properties"], [{"method": "CpPolynomial", "segments": [{"t_min": 273.15, "t_max": 400.0, "parameters": CP["segments"][0]["parameters"]}, {"t_min": 400.0, "t_max": 2273.15, "parameters": dict(CP["segments"][0]["parameters"])}],
                                                       "transitions": [{"T": 400.0, "dS": 2.0, "dH": 800.0, "dV": 1e-7}]}]),
    ("Q-cp", {}, MINERAL["standard_state_properties"], [{"method": "ConstantCp"}, {"method": "ConstantVolume"}]),
]
EQUATION = {"reference_conditions": {"T": 298.15, "P": 100000.0}, "stoichiometry": {"H4SiO4@": -1.0, "SiO2@": 1.0, "H2O@": 2.0}}
LOGK = {"logK": 2.7, "dG": -15411.0, "dH": 25000.0, "dS": 135.0, "dCp": 40.0, "dV": 2e-6}
ADDED_REACTIONS = [
    ("r-rb", [{"method": "RyzhenkoBryzgalin", "parameters": {"pK298": 2.7, "A": -500.0, "B": 3.0}}]),
    ("r-cp", [{"method": "LogKNordstromMunoz88", "parameters": {"a0": 1.8, "a1": 0.0, "a2": 0.0, "a3": 0.0, "a4": 0.0, "a5": 0.0, "a6": 0.0, "a": 20.0, "b": 0.05, "c": 1000.0, "d": 10.0, "e": 1e-5}}]),
    ("r-nm", [{"method": "LogKNordstromMunoz88", "parameters": {"a0": 1.8, "a1": -0.0026, "a2": 0.0, "a3": 0.0, "a4": 0.0, "a5": 4.4e-6, "a6": 0.0}}]),
    ("r-cp3", [{"method": "LogKConstantCp"}, {"method": "ReactionConstantVolume"}]),
    ("r-const", [{"method": "ConstantReactionProperties"}]),
]


def test_the_methods_that_the_files_do_not_have():
    doc = json.loads(tf.Database("pytests/data/v2/hkf.json").toJson(readable=False))
    for symbol, extra, reference, methods in ADDED_SUBSTANCES:
        record = {**{k: v for k, v in MINERAL.items() if k != "standard_state_properties"}, "name": symbol, "standard_state_properties": dict(reference), "methods": copy.deepcopy(methods), **extra}
        if extra:
            record["aggregate_state"] = "aqueous"
        doc["substances"][symbol] = record
    for symbol, methods in ADDED_REACTIONS:
        doc["reactions"][symbol] = {**copy.deepcopy(EQUATION), "name": symbol, "reaction_properties": dict(LOGK), "methods": copy.deepcopy(methods)}
    added = {m["method"] for _, _, _, methods in ADDED_SUBSTANCES for m in methods} | {m["method"] for _, methods in ADDED_REACTIONS for m in methods}
    problems = bad_points_of(tf.Database(json.dumps(doc)), doc, added)
    assert not problems, problems[:6]


@pytest.mark.parametrize("path,methods", [("pytests/test-aq17-gem-lma-thermofun.json", {"AkinfievDiamond03", "CorkHP91"}), ("Resources/databases/mines16-thermofun.json", {"LogKNordstromMunoz88"})],
                         ids=["aq17", "mines16"])
def test_the_methods_of_the_large_files(path, methods):
    """One record of each method, because each step loads the whole file."""
    db = tf.Database(path)
    doc = json.loads(db.toJson(readable=False))
    for method in methods:
        for section in ("substances", "reactions"):
            for symbol, record in doc.get(section, {}).items():
                if any(m["method"] == method for m in record.get("methods", [])):
                    problems = bad_points_of(db, doc, {method}, symbol)
                    assert not problems, problems[:6]
                    break


def test_the_derivatives_of_a_substance_through_its_defining_reaction():
    """Through the reaction, and through the other species of the reaction, with the parameters of each."""
    path = "pytests/data/v2/reactions.json"
    db = tf.Database(path)
    doc = json.loads(db.toJson(readable=False))
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    defined = [s for s, r in doc["substances"].items() if r.get("defining_reaction")]
    assert defined
    checked = 0
    problems = []
    for symbol in defined:
        reaction = doc["substances"][symbol]["defining_reaction"]
        others = [o for o in doc["reactions"][reaction]["stoichiometry"] if o != symbol]
        items = [("reactions", reaction, doc["reactions"][reaction])] + [("substances", o, doc["substances"][o]) for o in others if doc["substances"][o].get("class") != "water_solvent"]
        for section, owner_symbol, record in items:
            for owner, name, holder_of in parameters_of(section, owner_symbol, record):
                ref = (owner_symbol, owner, name)
                theta = si_value(db, section, owner_symbol, owner, name)
                for T, P in POINTS:
                    exact = {k: v[label(ref)] for k, v in engine.parameterSensitivities(T, P, symbol, [ref]).items()}
                    problems += check(doc, "substance", T, P, symbol, ref, holder_of, exact, theta)
                    checked += 1
    assert checked > 10
    assert not problems, problems[:6]


def test_the_derivatives_of_a_reaction_from_its_reactants():
    """A reaction without methods sums the derivatives of its species."""
    db = tf.Database("pytests/data/v2/reactions.json")
    doc = json.loads(db.toJson(readable=False))
    symbol, species = "H4SiO4@", "SiO2@"
    doc["reactions"][symbol].pop("methods", None)
    doc["reactions"][symbol].pop("reaction_properties", None)
    doc["substances"][symbol].pop("defining_reaction", None)  # else the reaction and the substance need each other
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    nu = doc["reactions"][symbol]["stoichiometry"][species]
    ref = (species, "", "S")
    reaction = engine.reactionParameterSensitivities(473.15, 1e7, symbol, [ref])
    substance = engine.parameterSensitivities(473.15, 1e7, species, [ref])
    assert reaction["dS"][label(ref)] == pytest.approx(nu * substance["S"][label(ref)], rel=1e-12)
    assert reaction["logK"][label(ref)] == pytest.approx(-nu * substance["G"][label(ref)] / (8.31451 * 473.15 * math.log(10.0)), rel=1e-4)


@pytest.mark.parametrize("ref,words", [
    (("Quartz", "CpPolynomial", "zz"), "a0, a1"),
    (("Quartz", "HKF88", "a1"), "does not use this method"),
    (("Quartz", "NoSuchMethod", "a1"), "no such method"),
    (("Quartz", "", "logK2"), "reference value is one of"),
    (("NoSuchSymbol", "", "G"), "no substance"),
    (("H2O@", "WaterHGK84", "state"), "method of water"),
])
def test_an_unknown_parameter_is_an_error_and_not_a_derivative_of_zero(ref, words):
    engine = tf.ThermoEngine("pytests/data/v2/minerals.json")
    with pytest.raises(RuntimeError, match=words):
        engine.parameterSensitivities(500.0, 1e7, "Quartz", [ref])


def test_the_derivative_is_zero_for_a_parameter_of_a_record_that_is_not_used():
    engine = tf.ThermoEngine("pytests/data/v2/minerals.json")
    other = next(s for s in engine.database().getSubstancesList() if s != "Quartz" and engine.database().getSubstance(s).record().standard_state_properties and s != "H2O@")
    ref = (other, "", "S")
    result = engine.parameterSensitivities(500.0, 1e7, "Quartz", [ref])
    assert all(v[label(ref)] == 0.0 for v in result.values())


def test_a_pressure_of_zero_is_the_saturation_pressure():
    engine = tf.ThermoEngine("pytests/data/v2/hkf.json")
    ref = ("Na+", "HKF88", "c1")
    saturated = engine.parameterSensitivities(473.15, 0.0, "Na+", [ref])
    psat = 1554939.22  # Pa at 200 degC
    assert saturated["Cp"][label(ref)] == pytest.approx(engine.parameterSensitivities(473.15, psat, "Na+", [ref])["Cp"][label(ref)], rel=1e-5)


def test_the_entropy_of_birch_murnaghan_with_a_large_step():
    """S of this model comes from a finite difference over a Newton iteration: a small step only shows that noise."""
    path = "pytests/data/legacy-synthetic/s37-mv_eos_birch_murnaghan_gott97.json"
    db = tf.Database(path)
    doc = json.loads(db.toJson(readable=False))
    symbol = next(iter(doc["substances"]))
    i = next(j for j, m in enumerate(doc["substances"][symbol]["methods"]) if m["method"] == "BirchMurnaghan47")
    ref = (symbol, "BirchMurnaghan47", "a1")
    theta = si_value(db, "substances", symbol, "BirchMurnaghan47", "a1")
    up, down = copy.deepcopy(doc), copy.deepcopy(doc)
    step = 0.03
    change(up["substances"][symbol]["methods"][i]["parameters"], "a1", 1.0 + step)
    change(down["substances"][symbol]["methods"][i]["parameters"], "a1", 1.0 - step)
    a, b = properties(up, "substance", 450.0, 2e7, symbol), properties(down, "substance", 450.0, 2e7, symbol)
    exact = tf.ThermoEngine(db).parameterSensitivities(450.0, 2e7, symbol, [ref])
    assert (a["S"] - b["S"]) / (2.0 * step) == pytest.approx(exact["S"][label(ref)] * theta, rel=2e-3)


def label(ref):
    symbol, method, parameter = ref
    return f"{symbol}/{method + '/' if method else ''}{parameter}"


@pytest.mark.parametrize("path", FILES, ids=os.path.basename)
def test_the_derivatives_of_a_record_by_its_parameters(path):
    problems = bad_points(path)
    assert not problems, problems[:6]


def both_ways(reaction_symbol="C"):
    """C has methods and a defining reaction. The reaction has methods and reactants, and the same symbol as C."""
    def substance(symbol, G, **more):
        return {"symbol": symbol, "formula": symbol, "class_": {"0": "SC_COMPONENT"}, "aggregate_state": {"3": "AS_CRYSTAL"}, "Tst": 298.15, "Pst": 100000,
                "sm_gibbs_energy": {"values": [G]}, "sm_enthalpy": {"values": [G]}, "sm_entropy_abs": {"values": [10.0]},
                "sm_heat_capacity_p": {"values": [20.0]}, "sm_volume": {"values": [1.0]}, **more}
    methods = [{"method": {"43": "standard_entropy_cp_integration"}}]
    return tf.Database(json.dumps({
        "substances": [substance("A", -1000.0, TPMethods=methods), substance("B", -2000.0, TPMethods=methods), substance("C", -3000.0, TPMethods=methods, reaction=reaction_symbol)],
        "reactions": [{"symbol": reaction_symbol, "equation": "A + B = C", "reactants": [{"symbol": "A", "coefficient": -1}, {"symbol": "B", "coefficient": -1}, {"symbol": "C", "coefficient": 1}],
                       "Tst": 298.15, "Pst": 100000, "logKr": {"values": [1.0]}, "TPMethods": [{"method": {"4": "logk_1_term_extrap0"}}]}]}))


def derivative(function, symbol, ref, name):
    return function(400.0, 1e7, symbol, [ref])[name][label(ref)]


def test_the_derivatives_follow_the_path_of_the_substance():
    engine = tf.ThermoEngine(both_ways())
    own, other = ("C", "", "G"), ("A", "", "G")
    assert derivative(engine.parameterSensitivities, "C", own, "G") == 1.0   # its own methods
    assert derivative(engine.parameterSensitivities, "C", other, "G") == 0.0
    engine.setPropertySource("C", "reaction")                                # its defining reaction: G(C) = dG + G(A) + G(B)
    assert derivative(engine.parameterSensitivities, "C", own, "G") == 0.0
    assert derivative(engine.parameterSensitivities, "C", other, "G") == pytest.approx(1.0)
    assert derivative(engine.parameterSensitivities, "C", ("C", "", "logK"), "G") == pytest.approx(-8.31451 * 400.0 * math.log(10.0), rel=1e-4)


def test_the_derivatives_follow_the_path_of_the_reaction():
    engine = tf.ThermoEngine(both_ways())
    logK, species = ("C", "", "logK"), ("C", "", "G")
    assert derivative(engine.reactionParameterSensitivities, "C", logK, "logK") == 1.0  # its own methods
    assert derivative(engine.reactionParameterSensitivities, "C", species, "dG") == 0.0
    engine.setPropertySource("C", "reactants")                                          # the sum over its species
    assert derivative(engine.reactionParameterSensitivities, "C", logK, "logK") == 0.0
    assert derivative(engine.reactionParameterSensitivities, "C", species, "dG") == pytest.approx(1.0)
    assert derivative(engine.reactionParameterSensitivities, "C", ("A", "", "G"), "dG") == pytest.approx(-1.0)


def test_a_substance_and_a_reaction_with_one_symbol_have_their_own_path():
    engine = tf.ThermoEngine(both_ways())
    engine.setPropertySource("C", "direct")
    engine.setPropertySource("C", "reactants")
    assert engine.thermoPropertiesSubstance(298.15, 1e5, "C").gibbs_energy.val == pytest.approx(-3000.0)   # still its methods
    assert engine.thermoPropertiesReaction(298.15, 1e5, "C").reaction_gibbs_energy.val == pytest.approx(0.0, abs=1e-6)   # G(C) - G(A) - G(B)
    assert engine.propertySource("C") == tf.PropertySource.Reactants
    engine.setPropertySource("C", "record")
    assert engine.propertySource("C") == tf.PropertySource.Record
    # a substance from a reaction that comes from that substance has no answer
    engine.setPropertySource("C", "reaction")
    engine.setPropertySource("C", "reactants")
    with pytest.raises(RuntimeError, match="C"):
        engine.thermoPropertiesSubstance(298.15, 1e5, "C")


def test_the_derivatives_of_a_fluid_on_a_substance_defined_by_a_reaction(tmp_path):
    source = json.loads(open("pytests/data/legacy-synthetic/s12-fluid_sterner_pitzer.json").read())
    template = source["substances"][0]
    fluid = {"method": {"13": "fluid_peng_robinson78"}, "eos_gas_crit_props": {"values": [304.2, 73.8, 0.225, 0.0427, 0, 0, 0]}}
    a, x = copy.deepcopy(template), copy.deepcopy(template)
    a.update(symbol="A", name="A", formula="CO2", TPMethods=[template["TPMethods"][0]])
    x.update(symbol="X", name="X", formula="CO2", TPMethods=[fluid], reaction="R")
    source["substances"] = [a, x]
    source["reactions"] = [{"symbol": "R", "equation": "A = X", "reactants": [{"symbol": "A", "coefficient": -1}, {"symbol": "X", "coefficient": 1}], "Tst": 298.15, "Pst": 100000,
                            "logKr": {"values": [1.0]}, "TPMethods": [{"method": {"4": "logk_1_term_extrap0"}}]}]
    path = tmp_path / "fluid.json"
    path.write_text(json.dumps(source))
    db = tf.Database(str(path))
    doc = json.loads(db.toJson(readable=False))
    engine = tf.ThermoEngine(db)
    problems = []
    volumes = {}
    for owner, name, holder_of in parameters_of("substances", "X", doc["substances"]["X"]):
        if owner != "PengRobinson78":
            continue
        ref = ("X", owner, name)
        theta = si_value(db, "substances", "X", owner, name)
        exact = {k: v[label(ref)] for k, v in engine.parameterSensitivities(600.0, 1e8, "X", [ref]).items()}
        volumes[name] = exact["V"]
        problems += check(doc, "substance", 600.0, 1e8, "X", ref, holder_of, exact, theta)
    assert not problems, problems[:4]
    assert volumes["Tc"] != 0.0 and volumes["Pc"] != 0.0


# --- the constants of the methods, also through the water ------------------------------------------------------------

AQ = "pytests/data/v2/hkf.json"


def solvent_method(engine_file=AQ):
    doc = json.loads(tf.Database(engine_file).toJson(readable=False))
    solvent = next(s for s, r in doc["substances"].items() if r.get("class") == "water_solvent")
    return solvent, doc["substances"][solvent]["methods"]


def gibbs_with(ref, value, symbol, T, P):
    engine = tf.ThermoEngine(AQ)
    engine.parameter(*ref).setValue(value)
    return engine.thermoPropertiesSubstance(T, P, symbol).gibbs_energy.val


def test_a_constant_of_the_dielectric_method_reaches_a_solute_through_the_water():
    solvent, methods = solvent_method()
    dielectric = next(m["method"] for m in methods if m["method"] in ("JohnsonNorton91", "SverjenskyEtAl14", "FernandezEtAl97"))
    name = {"JohnsonNorton91": "a2", "SverjenskyEtAl14": "b3", "FernandezEtAl97": "n1"}[dielectric]
    ref = (solvent, dielectric, name)
    T, P = 473.15, 5e7
    engine = tf.ThermoEngine(AQ)
    value = engine.parameter(*ref).value()
    got = engine.parameterSensitivities(T, P, "Na+", [ref])["G"]["%s/%s/%s" % ref]
    step = abs(value) * 1e-3
    hand = (gibbs_with(ref, value + step, "Na+", T, P) - gibbs_with(ref, value - step, "Na+", T, P)) / (2 * step)
    assert got != 0.0
    assert got == pytest.approx(hand, rel=1e-3)


def test_a_constant_of_the_water_reaches_the_water_itself():
    solvent, methods = solvent_method()
    water = next(m for m in methods if m["method"] in ("WaterHGK84", "WaterIAPWS95"))
    name = "M" if water.get("implementation", "reaktoro") == "reaktoro" else "ac_tz"
    ref = (solvent, water["method"], name)
    T, P = 473.15, 5e7
    engine = tf.ThermoEngine(AQ)
    assert engine.parameterSensitivities(T, P, solvent, [ref])["V"]["%s/%s/%s" % ref] != 0.0


def test_a_constant_of_the_method_of_a_solute():
    T, P = 473.15, 5e7
    ref = ("Na+", "HKF88", "eta")
    engine = tf.ThermoEngine(AQ)
    assert engine.parameterSensitivities(T, P, "Na+", [ref])["G"]["Na+/HKF88/eta"] != 0.0
    assert engine.parameter(*ref).value() > 0.0
    assert "eta" not in [h.key()[2] for h in engine.parameters("Na+")]


def test_a_constant_of_the_method_of_a_reaction():
    file = "pytests/Reactions/Co_Cu_aq-thermofun.json"
    ref = ("CoCl4-2", "RyzhenkoBryzgalin", "zzH2O")
    got = tf.ThermoEngine(file).reactionParameterSensitivities(673.15, 800e5, "CoCl4-2", [ref])["logK"]["CoCl4-2/RyzhenkoBryzgalin/zzH2O"]
    def logK(value):
        engine = tf.ThermoEngine(file)
        engine.parameter(*ref).setValue(value)
        return engine.thermoPropertiesReaction(673.15, 800e5, "CoCl4-2").log_equilibrium_constant.val
    assert got == pytest.approx((logK(1.0107 * 1.001) - logK(1.0107 * 0.999)) / (2 * 1.0107e-3), rel=1e-3)


def test_a_method_that_the_record_does_not_use_is_an_error():
    with pytest.raises(RuntimeError, match="does not use"):
        tf.ThermoEngine(AQ).parameterSensitivities(473.15, 5e7, "Na+", [("Na+", "CorkHP91", "cs_a0")])
