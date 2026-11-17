# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# A fit of HKF parameters with the constraints of OptimB (Shvarov 2015, Appl. Geochem. 55, 17), checked against the
# results of that program. The example of the article (supplement mmc1, sheet UO2++): G of UO2+2 along Psat at
# 10-300 C (Hovey et al. 1989, GCA 53, 1503), a1, a4, c1, c2, omega fitted, G and S fixed, with three exact
# constraints: Cp and V at 25 C equal to the given values, and a4 = -4.134 a2 - 27790 (cal; Shock and Helgeson 1988).
# OptimB searches omega in one dimension and solves the other parameters for each omega; so does the fit here.

import json

import pytest
import thermofun as tf

fitting = pytest.importorskip("thermofun.fitting")
pytest.importorskip("numpy")

CAL = 4.184
Tr, Pr = 298.15, 1e5
# Born functions of water at 25 C, 1 bar used by OptimB in its Cp and V constraints (Shock and Helgeson 1988)
Q_SH88, X_SH88 = 5.903e-7 / 1e5, -3.09e-7   # 1/Pa, 1/K2

# T (C), weight (1/kJ, 1/standard uncertainty), G (kJ/mol); P = Psat
DATA = [(10, 2, -954.45), (25, 2, -952.95), (40, 2, -951.48), (50, 1, -950.52), (55, 2, -950.04),
        (100, 1, -945.78), (150, 1, -941.02), (200, 1, -936.02), (250, 0.5, -930.6), (300, 0.5, -924.6)]
# the start values of the sheet, in the units of the sheet: a1*10, a2*1e-2, a3, a4*1e-4, c1, c2*1e-4, omega*1e-5
START = {"a1": 3.08848947286606 / 10, "a2": -0.240292987823486 * 1e2, "a3": 5.84447574615479, "a4": -2.7690662109375e4,
         "c1": 25.3741035461426, "c2": -0.98945205078125e4, "omega": 1.4461825e5}
# OptimB's result, as its port (bit-identical to OptimB.exe) reproduces it from the same input
OPTIMB = {"a1": 0.280370473861694, "a4": -27690.662109375, "c1": 11.3825359344482, "c2": 37086.10546875, "omega": 96374.203125}
OPTIMB_RMS = 0.0128404529931369   # kJ/mol: sqrt(sum (w e)^2 / sum w^2)
GIVEN_CP, GIVEN_V = 10.04 * CAL, 6.1e-6   # J/(mol K), m3/mol
SCALE = {"a1": CAL / 1e5, "a2": CAL, "a3": CAL / 1e5, "a4": CAL, "c1": CAL, "c2": CAL, "omega": CAL}   # cal units -> handle (SI)


def engine():
    uo2 = {"formula": "UO2+2", "charge": 2, "class": "aqueous_solute", "aggregate_state": "aqueous",
           "standard_state_properties": {"G": -227761 * CAL, "S": -23.64 * CAL},
           "methods": [{"method": "HKF88", "units": {"energy": "cal", "pressure": "bar"}, "parameters": START}]}
    water = {"formula": "H2O@", "class": "water_solvent", "aggregate_state": "aqueous",
             "standard_state_properties": {"G": -237181.0, "H": -285881.0, "S": 69.92},
             "methods": [{"method": "WaterHGK84"}, {"method": "JohnsonNorton91"}]}
    return tf.ThermoEngine(tf.Database(json.dumps({"thermohub": "2.0.0-draft.1", "substances": {"H2O@": water, "UO2+2": uo2}})))


def key(name):
    return ("UO2+2", "HKF88", name)


def problem(e):
    observations = [fitting.Observation("UO2+2", "G", t + 273.15, 0.0, g * 1000.0, 1000.0 / w) for t, w, g in DATA]
    # OptimB's Cp and V at 25 C use the Born functions of Shock and Helgeson (1988); the solvent's differ a little:
    # Cp_OptimB = Cp - omega Tr (X - X_SH88), V_OptimB = V + omega (Q - Q_SH88)
    born = e.electroPropertiesSolvent(Tr, Pr, "H2O@")
    constraints = [
        fitting.PropertyConstraint("UO2+2", "Cp", Tr, Pr, GIVEN_CP, {key("omega"): -Tr * (born.bornX.val - X_SH88)}),
        fitting.PropertyConstraint("UO2+2", "V", Tr, Pr, GIVEN_V, {key("omega"): born.bornQ.val - Q_SH88}),
        fitting.Relation({key("a4"): 1.0, key("a2"): 4.134}, -27790.0 * CAL),
    ]
    free = [key(n) for n in ("a1", "a4", "c1", "c2", "omega")]
    return fitting.Problem(e, free, observations, constraints=constraints, search=key("omega"))


def test_the_fit_of_uo2_gives_the_parameters_of_optimb():
    e = engine()
    result = problem(e).solve(search_step=1000 * CAL, search_tol=0.01)   # the first step of OptimB (1000 cal)
    assert result.success, result.message
    got = {n: result.values[f"UO2+2/HKF88/{n}"] / SCALE[n] for n in OPTIMB}
    # The rest is the water: OptimB has its own HGK in single precision and a fixed Y at 25 C in the Born term of G.
    # Found 2026-11-17: omega 96871 (implementation reaktoro, the default), 96619 (gems); c1, c2 within 1.1 %.
    assert got["omega"] == pytest.approx(OPTIMB["omega"], rel=1e-2)
    assert got["a1"] == pytest.approx(OPTIMB["a1"], rel=2e-3)
    assert got["a4"] == pytest.approx(OPTIMB["a4"], rel=1e-6)          # a2 is fixed: the relation sets a4
    assert got["c1"] == pytest.approx(OPTIMB["c1"], rel=2e-2)
    assert got["c2"] == pytest.approx(OPTIMB["c2"], rel=2e-2)
    assert result.weighted_rms / 1000.0 == pytest.approx(OPTIMB_RMS, rel=5e-2)
    # the constraints hold exactly
    assert max(abs(r) for r in result.constraint_residuals[:1]) < 1e-9 * GIVEN_CP
    assert abs(result.constraint_residuals[1]) < 1e-9 * GIVEN_V
    assert abs(result.constraint_residuals[2]) < 1e-9 * 27790.0 * CAL


def test_g_of_optimb_parameters_agrees_along_psat():
    # the same parameters give the same G within 10 J/mol (7 J/mol at 300 C): the models agree to the uncertainty of the data
    e = engine()
    for n, v in OPTIMB.items():
        e.parameter(*key(n)).setValue(v * SCALE[n])
    sheet_opt = [-954.453864765579, -952.952024000001, -951.482715221891, -950.51593050358, -950.035468969482,
                 -945.758841231732, -940.987985892713, -936.031109585453, -930.670881759234, -924.564116729871]
    for (t, _, _), g in zip(DATA, sheet_opt):
        assert e.thermoPropertiesSubstance(t + 273.15, 0.0, "UO2+2").gibbs_energy.val == pytest.approx(g * 1000.0, abs=10.0)
