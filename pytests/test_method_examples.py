# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The examples of docs/reference/method-examples.yaml, shown on the method pages: each record loads, calculates, and gives
# the published values of its checks. An example that stops working fails here, so the pages cannot go stale.

import pytest
import thermofun as tf
import yaml

EXAMPLES = "docs/reference/method-examples.yaml"
HEADER = "thermohub: 2.0.0-draft.1\n"
# the water of a reaction example that needs a solvent and does not give one
WATER = """substances:
  H2O@:
    formula: H2O@
    class: water_solvent
    aggregate_state: aqueous
    standard_state_properties: {G: -237183.0, H: -285881.0, S: 69.923, Cp: 75.36053, V: {value: 1.8068397, unit: J/bar}}
    methods:
      - method: WaterIAPWS95
      - method: JohnsonNorton91
"""
SUBSTANCE = {"G": "gibbs_energy", "H": "enthalpy", "S": "entropy", "Cp": "heat_capacity_cp", "V": "volume"}


def examples():
    with open(EXAMPLES, encoding="utf-8") as f:
        return yaml.safe_load(f)


def engine(example):
    """The record of the example, with a water if it has none: some methods read the density of water."""
    if "H2O@:" in example["record"]:
        return tf.ThermoEngine(tf.Database(HEADER + example["record"]))
    database = tf.Database(HEADER + WATER)
    database.appendData(HEADER + example["record"])
    return tf.ThermoEngine(database)


def value(e, check):
    T, P, symbol, name = check["T"], check["P"], check["symbol"], check["property"]
    if name == "epsilon":
        return e.electroPropertiesSolvent(T, P, symbol).epsilon.val
    if name == "logK":
        return e.thermoPropertiesReaction(T, P, symbol).log_equilibrium_constant.val
    return getattr(e.thermoPropertiesSubstance(T, P, symbol), SUBSTANCE[name]).val


@pytest.mark.parametrize("example", examples(), ids=lambda x: x["method"])
def test_the_example_gives_the_published_values(example):
    e = engine(example)
    assert example["method"] in example["record"]
    for check in example.get("checks", []):
        assert value(e, check) == pytest.approx(check["value"], abs=check["tolerance"]), check
    for point in example.get("calculates", []):
        reactions = "reactions:" in example["record"] and point["symbol"] not in tf.Database(HEADER + example["record"]).mapSubstances()
        p = e.thermoPropertiesReaction(point["T"], point["P"], point["symbol"]) if reactions else e.thermoPropertiesSubstance(point["T"], point["P"], point["symbol"])
        assert p is not None


def test_each_example_has_a_source_and_something_to_test():
    known = {m["id"] for m in tf.methods.catalog()}
    for example in examples():
        assert example["method"] in known
        assert example["source"]
        assert example.get("checks") or example.get("calculates"), example["method"]


def test_each_page_with_an_example_shows_the_calculation_and_its_output():
    import glob
    shown = 0
    for page in glob.glob("docs/reference/methods/*.md"):
        text = open(page, encoding="utf-8").read()
        if "```yaml" in text:
            assert "Calculation, with the record as above:" in text and "```text" in text, page
            shown += 1
    assert shown >= 30
