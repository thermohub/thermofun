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
