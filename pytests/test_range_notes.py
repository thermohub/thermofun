# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# Range notes (decision 0029). A value outside the range of its model is calculated and marked; a value of a method inside its
# own range that uses another outside it inherits the note, with the path.

import pytest
import thermofun as tf

CAL = 4.184


def engine(implementation):
    water = {"formula": "H2O@", "class": "water_solvent", "aggregate_state": "aqueous", "standard_state_properties": {"G": -237181},
             "methods": ([{"method": "WaterLVS83"}] if implementation == "gems" else [])
             + [{"method": "WaterHGK84", "implementation": implementation}, {"method": "JohnsonNorton91", "implementation": implementation}]}
    na = {"formula": "Na+", "charge": 1, "class": "aqueous_solute", "aggregate_state": "aqueous",
          "standard_state_properties": {"G": -62591 * CAL, "H": -57433 * CAL, "S": 13.96 * CAL},
          "methods": [{"method": "HKF88", "implementation": implementation, "units": {"energy": "cal", "pressure": "bar"},
                       "parameters": {"a1": 0.1839, "a2": -228.5, "a3": 3.256, "a4": -27260, "c1": 18.18, "c2": -29810, "omega": 33060}}]}
    return tf.ThermoEngine(tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"H2O@": water, "Na+": na}}))


@pytest.mark.parametrize("T,P", [(1274.0, 5e8), (1273.15, 5.1e8)])
def test_johnson_norton_gems_beyond_its_range_is_calculated_and_marked(T, P):
    """SUPCRT92 and GEMS returned 0 for epsilon and the Born functions above 1000 degC or 5 kbar: Na+ jumped by 20 kJ/mol."""
    gems, reaktoro = engine("gems"), engine("reaktoro")
    eps = gems.electroPropertiesSolvent(T, P, "H2O@").epsilon
    assert eps.val == pytest.approx(reaktoro.electroPropertiesSolvent(T, P, "H2O@").epsilon.val, rel=1e-3)
    assert eps.outside_range and "JohnsonNorton91" in eps.sta[1]
    G = gems.thermoPropertiesSubstance(T, P, "Na+").gibbs_energy
    assert G.val == pytest.approx(reaktoro.thermoPropertiesSubstance(T, P, "Na+").gibbs_energy.val, abs=50.0)


def test_a_solute_inherits_the_note_of_the_water_with_the_path():
    G = engine("gems").thermoPropertiesSubstance(1274.0, 5e8, "Na+").gibbs_energy
    assert G.outside_range
    assert "via the water: JohnsonNorton91" in G.sta[1]


def test_inside_the_range_there_is_no_note():
    e = engine("gems")
    assert not e.electroPropertiesSolvent(1273.15, 5e8, "H2O@").epsilon.outside_range
    assert not e.thermoPropertiesSubstance(1273.15, 5e8, "Na+").gibbs_energy.outside_range


def test_the_notes_are_data_with_the_method_and_the_path():
    e = engine("gems")
    water = e.electroPropertiesSolvent(1274.0, 5e8, "H2O@").notes
    assert len(water) == 1 and water[0].method == "JohnsonNorton91" and water[0].via == []
    assert water[0].severity == tf.Note.Severity.Soft and water[0].kind == tf.Note.Kind.Range
    solute = e.thermoPropertiesSubstance(1274.0, 5e8, "Na+").notes
    assert len(solute) == 1 and solute[0].method == "JohnsonNorton91" and solute[0].via == ["the water"]


def test_the_status_message_is_made_from_the_notes():
    r = engine("gems").thermoPropertiesSubstance(1274.0, 5e8, "Na+")
    assert r.gibbs_energy.sta[1] == "".join(repr(n)[len("<Note "):-1] for n in r.notes)
    assert str(r.notes[0]).startswith("Outside the range of the model: via the water: JohnsonNorton91: T = 1000.85 degC")


def test_inside_the_range_the_result_has_no_notes():
    e = engine("gems")
    assert e.thermoPropertiesSubstance(1273.15, 5e8, "Na+").notes == []
    assert e.electroPropertiesSolvent(1273.15, 5e8, "H2O@").notes == []


def engine_with_reactions():
    """Na+ on the GEMS water; a reaction without methods (from its species), and a record defined by a reaction with Na+."""
    import json
    water = {"formula": "H2O@", "class": "water_solvent", "aggregate_state": "aqueous", "standard_state_properties": {"G": -237181},
             "methods": [{"method": "WaterLVS83"}, {"method": "WaterHGK84", "implementation": "gems"}, {"method": "JohnsonNorton91", "implementation": "gems"}]}
    na = {"formula": "Na+", "charge": 1, "class": "aqueous_solute", "aggregate_state": "aqueous",
          "standard_state_properties": {"G": -62591 * CAL, "H": -57433 * CAL, "S": 13.96 * CAL},
          "methods": [{"method": "HKF88", "implementation": "gems", "units": {"energy": "cal", "pressure": "bar"},
                       "parameters": {"a1": 0.1839, "a2": -228.5, "a3": 3.256, "a4": -27260, "c1": 18.18, "c2": -29810, "omega": 33060}}]}
    b = {"formula": "Na+", "charge": 1, "class": "component", "aggregate_state": "crystal",
         "standard_state_properties": {"G": -260000.0, "H": -240000.0, "S": 60.0, "Cp": 40.0, "V": 2e-5}, "methods": [{"method": "ConstantCp"}]}
    z = {"formula": "Na+", "charge": 1, "class": "component", "aggregate_state": "crystal", "defining_reaction": "RZ"}
    doc = {"thermohub": "2.0.0-draft.1", "substances": {"H2O@": water, "Na+": na, "B": b, "Z": z},
           "reactions": {"RB": {"equation": "B = Na+"},
                         "RZ": {"equation": "Z = Na+", "reaction_properties": {"logK": 1.0}, "methods": [{"method": "LogKConstant"}]}}}
    return tf.ThermoEngine(tf.Database(json.dumps(doc)))


def test_a_reaction_from_its_species_gets_their_notes():
    r = engine_with_reactions().thermoPropertiesReaction(1274.0, 5e8, "RB")
    assert [(n.method, n.via) for n in r.notes] == [("JohnsonNorton91", ["Na+", "the water"])]
    assert "via Na+: via the water: JohnsonNorton91" in r.log_equilibrium_constant.sta[1]
    assert r.log_equilibrium_constant.outside_range


def test_a_record_defined_by_a_reaction_gets_the_notes_of_the_other_species():
    z = engine_with_reactions().thermoPropertiesSubstance(1274.0, 5e8, "Z")
    assert ("JohnsonNorton91", ["Na+", "the water"]) in [(n.method, n.via) for n in z.notes]
    assert z.gibbs_energy.outside_range


def test_inside_the_range_reactions_and_derived_records_have_no_notes():
    e = engine_with_reactions()
    assert e.thermoPropertiesReaction(1273.15, 5e8, "RB").notes == []
    assert e.thermoPropertiesSubstance(1273.15, 5e8, "Z").notes == []
