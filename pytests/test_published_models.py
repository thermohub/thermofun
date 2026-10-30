# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Models against the values published with them.

Each database in pytests/data/published/ holds the parameters of one paper. The tests compare the calculated
values with the values that the paper gives. The tolerances cover the rounding of the published tables and the
small difference of the water equation of state.
"""
import json
import math
import os

import pytest
import thermofun as tf

HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data", "published")

# Dolejs and Manning (2010), Table 2: log K, dG [kJ/mol], dH [kJ/mol], dS [J/(K mol)], dCp [J/(K mol)], dV [cm3/mol]
DM10_TABLE2 = {
    "Apatite-F": (-11.31, 64.53, 61.24, -11.02, -90.13, -12.08),
    "Calcite": (-8.20, 46.78, 55.69, 29.90, -73.65, -9.87),
    "Corundum": (-12.54, 71.60, 79.46, 26.37, -37.35, -5.00),
    "Fluorite": (-8.59, 49.01, 55.01, 20.13, -60.27, -8.07),
    "Portlandite": (-2.82, 16.08, 10.84, -17.60, -95.60, -12.81),
    "Quartz": (-3.91, 22.32, 24.05, 5.80, -2.04, -2.50),
    "Rutile": (-16.45, 93.93, 103.98, 33.72, -13.43, -1.80),
}
# Rutile: Table 2 gives dS = -b of Table 1 (33.72), as if the e ln(rho) term were left out, which it is not for the
# other minerals; dH differs by the same term. Only log K, dG, dCp and dV are compared for it.
DM10_SKIP = {("Rutile", 2), ("Rutile", 3)}
DM10_TOL = (0.02, 0.15, 0.15, 0.1, 0.1, 0.05)


@pytest.mark.parametrize("mineral", sorted(DM10_TABLE2))
def test_dolejs_manning_2010_table2(mineral):
    engine = tf.ThermoEngine(os.path.join(HERE, "DolejsManning2010-thermofun.json"))
    p = engine.thermoPropertiesReaction(298.15, 1e5, mineral + "-dissolution")
    got = (p.log_equilibrium_constant.val, p.reaction_gibbs_energy.val / 1e3, p.reaction_enthalpy.val / 1e3,
           p.reaction_entropy.val, p.reaction_heat_capacity_cp.val, p.reaction_volume.val * 10)
    for i, (value, expected, tol) in enumerate(zip(got, DM10_TABLE2[mineral], DM10_TOL)):
        if (mineral, i) in DM10_SKIP:
            continue
        assert value == pytest.approx(expected, abs=tol), (mineral, i)
    # log K follows from the Gibbs energy
    assert got[0] == pytest.approx(-got[1] * 1e3 / (8.31451 * 298.15 * math.log(10)), rel=1e-6)


def test_marshall_franck_1981_water_ion_product():
    engine = tf.ThermoEngine(os.path.join(HERE, "MarshallFranck1981-thermofun.json"))
    p = engine.thermoPropertiesReaction(298.15, 1e5, "H2O-ionization")
    assert p.log_equilibrium_constant.val == pytest.approx(-13.995, abs=0.002)
    assert p.log_equilibrium_constant.val == pytest.approx(
        -p.reaction_gibbs_energy.val / (8.31451 * 298.15 * math.log(10)), rel=1e-6)


def test_published_databases_name_their_source():
    for name in os.listdir(HERE):
        with open(os.path.join(HERE, name)) as f:
            data = json.load(f)
        assert data["datasources"] and "(" in data["datasources"][0], name
