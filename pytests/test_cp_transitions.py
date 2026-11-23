# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# CpPolynomial with fewer phase transitions than segment boundaries read past the end of the list of transitions
# (review of v2, S5: valgrind, and G at 600 K changed). A boundary without a transition has none, so the record gives the
# same values as one with an explicit zero transition there.

import pytest
import thermofun as tf


def database(transitions):
    segment = lambda lo, hi, a0: "{t_min: %s, t_max: %s, parameters: {a0: %s, a1: 0.01}}" % (lo, hi, a0)
    return "\n".join([
        "thermohub: 2.0.0-draft.1",
        "substances:",
        "  X:",
        "    formula: SiO2",
        "    class: component",
        "    aggregate_state: crystal",
        "    standard_state_properties: {G: -856288, H: -910700, S: 41.46, Cp: 44.6, V: 2.269e-5}",
        "    methods:",
        "      - method: CpPolynomial",
        "        segments: [%s, %s, %s]" % (segment(298.15, 500, 44.6), segment(500, 700, 60.0), segment(700, 1200, 70.0)),
        "        transitions: [%s]" % ", ".join(transitions),
    ]) + "\n"


ONE = ["{T: 500, dS: 1.5, dH: 750, dV: 0}"]
TWO = ONE + ["{T: 700, dS: 0, dH: 0, dV: 0}"]


@pytest.mark.parametrize("T", [400.0, 600.0, 800.0, 1100.0])
def test_a_boundary_without_a_transition_has_none(T):
    one, two = (tf.ThermoEngine(database(t)).thermoPropertiesSubstance(T, 1e5, "X") for t in (ONE, TWO))
    for name in ("gibbs_energy", "enthalpy", "entropy", "heat_capacity_cp"):
        assert getattr(one, name).val == getattr(two, name).val, (name, T)


def test_a_transition_away_from_a_boundary_is_reported():
    db = tf.Database(database(["{T: 550, dS: 1.5, dH: 750, dV: 0}"]))
    assert "V2-MTH-025" in {d.code for d in db.validate().all()}
    assert "V2-MTH-025" not in {d.code for d in tf.Database(database(ONE)).validate().all()}
    too_many = tf.Database(database(TWO + ["{T: 900, dS: 0, dH: 0, dV: 0}"]))
    assert "V2-MTH-025" in {d.code for d in too_many.validate().all()}
