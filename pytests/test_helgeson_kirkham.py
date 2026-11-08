# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The dielectric constant of Helgeson and Kirkham (1974), with the tables and the example of Tanger and Helgeson (1988).

import json

import pytest
import thermofun as tf

WATER = tf.methods.solventMethod("WaterHGK84", implementation="reaktoro")
METHOD = tf.methods.dielectricMethod("HelgesonKirkham74")


def born(T_C, P_kb, method=METHOD):
    T, P = T_C + 273.15, P_kb * 1e8
    return method.properties(T, P, WATER.properties(T, P, 0), 0)


@pytest.mark.parametrize("T_C,P_kb,table", [(25, 0.25, 5.73), (25, 1, 5.52), (200, 0.5, 12.07), (300, 0.5, 21.69), (350, 0.25, 67.94), (400, 1, 30.68), (450, 2, 22.40)])
def test_y_of_table_h4(T_C, P_kb, table):
    assert -born(T_C, P_kb).bornY.val * 1e5 == pytest.approx(table, abs=0.02)


@pytest.mark.parametrize("T_C,P_kb,table", [(25, 0.25, 2.87), (25, 1, 2.40), (100, 0.5, 3.33), (200, 1, 3.95), (300, 0.5, 17.00), (350, 0.25, 198.5), (400, 1, 22.84)])
def test_x_of_table_h6(T_C, P_kb, table):
    assert -born(T_C, P_kb).bornX.val * 1e7 == pytest.approx(table, rel=2e-3, abs=0.02)


def test_the_reference_values_of_issue_82():
    T, P = 298.15, 1e5
    x = METHOD.properties(T, P, WATER.properties(T, P, 0), 0)
    assert x.bornQ.val * 1e5 == pytest.approx(5.903e-7, rel=1e-3)
    assert x.bornX.val == pytest.approx(-3.09e-7, rel=1e-3)


def test_the_derivatives_agree_with_finite_differences():
    T, P = 473.15, 5e7
    values = lambda t, p: METHOD.properties(t, p, WATER.properties(t, p, 0), 0)
    x = values(T, P)
    assert x.epsilonT.val == pytest.approx((values(T + 1e-2, P).epsilon.val - values(T - 1e-2, P).epsilon.val) / 2e-2, rel=1e-5)
    assert x.epsilonP.val == pytest.approx((values(T, P + 1e3).epsilon.val - values(T, P - 1e3).epsilon.val) / 2e3 * 1e5, rel=1e-4)
    assert x.bornY.val == pytest.approx(x.epsilonT.val / x.epsilon.val ** 2, rel=1e-12)
    assert x.epsilon.ddt == pytest.approx(x.epsilonT.val, rel=1e-8)


def al3(dielectric):
    water = {"formula": "H2O@", "class": "water_solvent", "aggregate_state": "aqueous",
             "standard_state_properties": {"G": -237181.0, "H": -285837.0, "S": 69.92, "Cp": 75.36, "V": 1.8068e-5},
             "methods": [{"method": "WaterHGK84", "implementation": "reaktoro"}, dict(dielectric)]}
    parameters = {"a1": {"value": -0.334, "unit": "cal/(mol.bar)"}, "a2": {"value": -1711, "unit": "cal/mol"}, "a3": {"value": 14.99, "unit": "cal.K/(mol.bar)"},
                  "a4": {"value": -20720, "unit": "cal.K/mol"}, "c1": {"value": 10.7, "unit": "cal/(mol.K)"}, "c2": {"value": -80600, "unit": "cal.K/mol"},
                  "omega": {"value": 287100, "unit": "cal/mol"}}
    ion = {"formula": "Al+3", "charge": 3, "class": "aqueous_solute", "aggregate_state": "aqueous",
           "standard_state_properties": {"G": -481160.0, "H": -527184.0, "S": -74.9, "Cp": -134.5, "V": -4.44e-5},
           "methods": [{"method": "HKF88", "implementation": "reaktoro", "parameters": parameters}]}
    doc = {"thermohub": "2.0.0-draft.1", "substances": {"H2O@": water, "Al+3": ion}}
    return tf.ThermoEngine(tf.Database(json.dumps(doc))).thermoPropertiesSubstance(298.15, 1e5, "Al+3")


def test_published_hkf_coefficients_give_the_published_properties_with_this_dielectric_constant():
    result = al3({"method": "HelgesonKirkham74"})
    assert result.volume.val * 1e6 == pytest.approx(-44.4, abs=0.05)
    assert result.heat_capacity_cp.val == pytest.approx(-134.5, abs=0.15)


def test_and_they_do_not_with_johnson_norton():
    result = al3({"method": "JohnsonNorton91", "implementation": "reaktoro"})
    assert result.volume.val * 1e6 == pytest.approx(-45.28, abs=0.05)
    assert result.heat_capacity_cp.val == pytest.approx(-133.2, abs=0.15)
