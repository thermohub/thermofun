# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Writes the v2 fixtures of pytests/data/v2 from records of the bundled legacy databases.

The values are the SI values of the records, so a v2 fixture must give the same properties as the legacy record
(to the rounding of the unit conversion). Run it with a ThermoFun build that reads the legacy format:
    python pytests/data/v2/make_v2_fixtures.py
"""

import json
import os

import thermofun as tf

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
OUT = os.path.join(ROOT, "pytests", "data", "v2")


def quantity(q):
    if q.uncertainty is None:
        return q.value
    return {"value": q.value, "uncertainty": q.uncertainty.standard}


def number_map(parameters):
    return {name: quantity(q) for name, q in parameters.items()}


def applicability(a):
    out = {}
    for key in ("t_min", "t_max", "p_min", "p_max"):
        if getattr(a, key) is not None:
            out[key] = getattr(a, key)
    return out


def method(app):
    out = {"method": app.method}
    if app.implementation:
        out["implementation"] = app.implementation
    if app.parameters:
        out["parameters"] = number_map(app.parameters)
    if app.segments:
        out["segments"] = [{"t_min": s.t_min, "t_max": s.t_max, "parameters": number_map(s.parameters)} for s in app.segments]
    if app.transitions:
        out["transitions"] = [{k: v.value for k, v in row.items()} for row in app.transitions]
    if app.table is not None:
        t = app.table
        table = {"temperatures": list(t.temperatures), "pressures": list(t.pressures)}
        table.update({k: list(v) for k, v in t.values.items()})
        if t.reference_pressure is not None:
            table["reference_pressure"] = t.reference_pressure
        out["table"] = table
    a = applicability(app.applicability)
    if a:
        out["applicability"] = a
    return out


def substance(db, symbol):
    r = db.getSubstance(symbol).record()
    out = {"name": r.name, "formula": r.formula}
    if r.charge is not None:
        out["charge"] = r.charge
    out["class"] = r.class_
    out["aggregate_state"] = r.aggregate_state
    out["reference_conditions"] = {"T": r.reference_conditions.T, "P": r.reference_conditions.P}
    props = {}
    for name, q in r.standard_state_properties.items():
        props[str(name).split(".")[-1]] = quantity(q)
    out["standard_state_properties"] = props
    if r.molar_mass is not None:
        out["molar_mass"] = {"value": r.molar_mass.value, "unit": "kg/mol"}
    if r.methods:
        out["methods"] = [method(m) for m in r.methods]
    if r.defining_reaction:
        out["defining_reaction"] = r.defining_reaction
    if r.preferred_property_source is not None:
        out["preferred_property_source"] = str(r.preferred_property_source).split(".")[-1].lower()
    a = applicability(r.applicability)
    if a:
        out["applicability"] = a
    if r.structural:
        out["structural"] = number_map(r.structural)
    return out


def reaction(db, symbol):
    r = db.getReaction(symbol).record()
    # the equation of a legacy record may name the substances by their formulas; the stoichiometry has the symbols
    out = {"name": r.name, "stoichiometry": {k: v for k, v in r.stoichiometry}}
    out["reference_conditions"] = {"T": r.reference_conditions.T, "P": r.reference_conditions.P}
    props = {}
    for name, q in r.reaction_properties.items():
        props[str(name).split(".")[-1]] = quantity(q)
    out["reaction_properties"] = props
    if r.methods:
        out["methods"] = [method(m) for m in r.methods]
    a = applicability(r.applicability)
    if a:
        out["applicability"] = a
    return out


def element(db, symbol):
    r = db.getElement(symbol).record()
    out = {"name": r.name, "class": r.class_}
    if r.number is not None:
        out["number"] = r.number
    for key in ("molar_mass", "standard_entropy", "heat_capacity", "volume"):
        q = getattr(r, key)
        if q is not None:
            out[key] = q.value
    if r.molar_mass is not None:
        out["molar_mass"] = {"value": r.molar_mass.value, "unit": "kg/mol"}
    return out


def document(path, substances, reactions, elements, dataset):
    db = tf.Database(os.path.join(ROOT, path))
    doc = {"thermohub": "2.0.0-draft.1", "generator": "make_v2_fixtures.py", "thermodataset": dataset}
    if elements:
        doc["elements"] = {s: element(db, s) for s in elements if db.containsElement(s)}
    # the closure: the reactions that define the substances, and the substances of those reactions
    wanted, todo, reactions = [], list(substances), list(reactions)
    while todo:
        symbol = todo.pop()
        if symbol in wanted or not db.containsSubstance(symbol):
            continue
        wanted.append(symbol)
        defining = db.getSubstance(symbol).record().defining_reaction
        if defining and defining not in reactions:
            reactions.append(defining)
    for symbol in reactions:
        if db.containsReaction(symbol):
            for item, _ in db.getReaction(symbol).record().stoichiometry:
                if item not in wanted:
                    todo.append(item)
        while todo:
            item = todo.pop()
            if item in wanted or not db.containsSubstance(item):
                continue
            wanted.append(item)
            defining = db.getSubstance(item).record().defining_reaction
            if defining and defining not in reactions:
                reactions.append(defining)
    doc["substances"] = {s: substance(db, s) for s in wanted}
    if reactions:
        doc["reactions"] = {s: reaction(db, s) for s in reactions if db.containsReaction(s)}
    return doc


def write(name, doc):
    with open(os.path.join(OUT, name), "w") as f:
        json.dump(doc, f, indent=1)
        f.write("\n")


def main():
    data = tf.Database(os.path.join(ROOT, "pytests/test-database-thermofun.json"))
    write("minimal.json", document("pytests/test-database-thermofun.json", data.getSubstancesList(), data.getReactionsList(),
                                   data.getElementsList(), {"symbol": "minimal", "name": "Fixture from the test database"}))
    write("hkf.json", document("pytests/test-aq17-gem-lma-thermofun.json",
                               ["H2O@", "H+", "Na+", "Cl-", "Ca+2", "HCO3-", "Al(OH)2+", "Al(OH)4-", "H4SiO4@"],
                               [], [], {"symbol": "hkf", "name": "HKF solutes from aq17"}))
    write("minerals.json", document("pytests/test-aq17-gem-lma-thermofun.json",
                                    ["Quartz", "Akermanite", "Calcite", "Portlandite", "Gibbsite"], [], [],
                                    {"symbol": "minerals", "name": "Minerals from aq17"}))
    mines = tf.Database(os.path.join(ROOT, "Resources/databases/mines16-thermofun.json"))
    gases = [x for x in mines.getSubstancesList() if mines.getSubstance(x).substanceClass().name == "GASFLUID"][:6]
    write("gases.json", document("Resources/databases/mines16-thermofun.json", gases, [], [], {"symbol": "gases"}))
    write("reactions.json", document("pytests/test-aq17-gem-lma-thermofun.json", [], ["Meionite-Ca"], [],
                                     {"symbol": "reactions"}))


def invalid():
    """One small file for each error that the reader reports. The file name starts with the code."""
    def doc(**kw):
        d = {"thermohub": "2.0.0-draft.1"}
        d.update(kw)
        return d

    def sub(**kw):
        d = {"name": "x", "formula": "Fe", "charge": 0, "class": "component", "aggregate_state": "crystal"}
        d.update(kw)
        return d

    cases = {
        "V2-VER-001-major-version": {"thermohub": "3.0.0", "substances": {}},
        "V2-ID-002-key-and-symbol": doc(substances={"A": sub(symbol="B")}),
        "V2-ID-001-no-symbol": doc(substances=[sub()]),
        "V2-QTY-001-string-quantity": doc(substances={"A": sub(standard_state_properties={"G": "abc kJ/mol"})}),
        "V2-QTY-002-no-value": doc(substances={"A": sub(standard_state_properties={"G": {"unit": "kJ/mol"}})}),
        "V2-UNIT-001-unknown-unit": doc(substances={"A": sub(standard_state_properties={"G": {"value": 1, "unit": "furlong"}})}),
        "V2-UNIT-002-wrong-dimension": doc(substances={"A": sub(standard_state_properties={"G": {"value": 1, "unit": "kg"}})}),
        "V2-UNC-001-no-coverage-factor": doc(substances={"A": sub(standard_state_properties={"G": {"value": 1, "uncertainty": {"expanded": 2}}})}),
        "V2-UNC-002-standard-and-expanded": doc(substances={"A": sub(standard_state_properties={"G": {"value": 1, "uncertainty": {"standard": 1, "expanded": 2, "coverage_factor": 2}}})}),
        "V2-UNC-003-negative": doc(substances={"A": sub(standard_state_properties={"G": {"value": 1, "uncertainty": -1}})}),
        "V2-UNC-004-coverage-factor": doc(substances={"A": sub(standard_state_properties={"G": {"value": 1, "uncertainty": {"expanded": 2, "coverage_factor": 0}}})}),
        "V2-MTH-001-unknown-method": doc(substances={"A": sub(methods=[{"method": "NoSuchMethod"}])}),
        "V2-MTH-002-unknown-parameter": doc(substances={"A": sub(methods=[{"method": "ConMolVol", "parameters": {"zz": 1}}])}),
        "V2-MTH-003-parameters-and-segments": doc(substances={"A": sub(methods=[{"method": "CpIntegration", "parameters": {"a0": 1}, "segments": []}])}),
        "V2-MTH-004-missing-parameter": doc(substances={"A": sub(methods=[{"method": "HKFReaktoro", "parameters": {"a1": 1}}])}),
        "V2-MTH-018-table-size": doc(substances={"A": sub(methods=[{"method": "TPTableInterpolation", "table": {"temperatures": [300, 400], "pressures": [1e5], "G": [1]}}])}),
        "V2-RXN-001-equation-and-stoichiometry": doc(reactions={"R": {"equation": "1 Na+ + 1 Cl- = 1 NaCl@", "stoichiometry": {"NaCl@": 1, "Na+": -1, "Cl-": -2}}}),
        "V2-DS-001-published-without-version": doc(thermodataset={"symbol": "x", "status": "published"}, substances={}),
    }
    for name, d in cases.items():
        with open(os.path.join(OUT, "invalid", name + ".json"), "w") as f:
            json.dump(d, f, indent=1)
            f.write("\n")


if __name__ == "__main__":
    main()
    invalid()
