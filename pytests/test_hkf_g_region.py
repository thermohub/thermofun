# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The g function of HKF88 uses the difference function f of Shock et al. (1992) only in its region II, 155-355 C and
# P <= 1000 bar, as SUPCRT92 does (owner, 2026-11-17). From 2017 to 2026 ThermoFun used f also above 355 C below
# 1000 bar: g rose to +0.35 A at 450 C, 500 bar and +800 A at 600 C, 300 bar. The reference values are those of the
# program OptimB (Shvarov 2015; its C++ port, bit-identical to OptimB.exe), whose g follows the rule of SUPCRT92.
# Below 350 kg/m3, the lower density limit of HKF in SUPCRT92 (which refuses it), g is held at its value at 350 kg/m3:
# a function of T only, so that the effective radius of an ion stays positive and its G finite in steam (owner, 2026-11-17).

import pytest
import thermofun as tf

# T (C), P (bar), g (Angstrom) of OptimB
OPTIMB = [(300, 500, -0.001681),     # in region II: f is used
          (354.99, 300, -0.009327),   # just below 355 C: f is used
          (355.01, 300, -0.017195),   # just above: f is not used, g jumps
          (400, 500, -0.028716),
          (450, 500, -0.107972),
          (500, 800, -0.073979)]


def engine(implementation):
    water = {"formula": "H2O@", "class": "water_solvent", "aggregate_state": "aqueous", "standard_state_properties": {"G": -237181.0},
             "methods": [{"method": "WaterHGK84", "implementation": implementation}, {"method": "JohnsonNorton91", "implementation": implementation}]}
    na = {"formula": "Na+", "charge": 1, "class": "aqueous_solute", "aggregate_state": "aqueous", "standard_state_properties": {"G": -261881.0, "S": 58.4},
          "methods": [{"method": "HKF88", "implementation": implementation, "units": {"energy": "cal", "pressure": "bar"},
                       "parameters": {"a1": 0.1839, "a2": -228.5, "a3": 3.256, "a4": -27260, "c1": 18.18, "c2": -29810, "omega": 33060}}]}
    return tf.ThermoEngine(tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"H2O@": water, "Na+": na}}))


def g(e, t, p):
    return e.auxiliaryOutputs("Na+", t + 273.15, p * 1e5)[0]["HKF88:g"]


@pytest.mark.parametrize("implementation", ["reaktoro", "gems"])
def test_g_is_that_of_supcrt92_and_optimb(implementation):
    e = engine(implementation)
    for t, p, wanted in OPTIMB:
        # OptimB rounds g to single precision and has its own HGK water: 1e-5 A
        assert g(e, t, p) == pytest.approx(wanted, abs=1e-5), (t, p)


@pytest.mark.parametrize("implementation", ["reaktoro", "gems"])
def test_g_stays_negative_above_355_c(implementation):
    e = engine(implementation)
    for t in (400, 450, 500):   # not 380: the water of reaktoro fails at 653 K, 800 bar (handoff item 5)
        assert g(e, t, 800.0) < 0.0, t
    assert g(e, 450, 500.0) < 0.0


@pytest.mark.parametrize("implementation", ["reaktoro", "gems"])
def test_the_derivatives_of_g_follow_g_on_both_sides_of_355_c(implementation):
    e = engine(implementation)
    for t in (350.0, 360.0):
        T, P, h = t + 273.15, 300e5, 1e-3
        a = e.auxiliaryOutputs("Na+", T, P)[0]
        dG_dT = (e.thermoPropertiesSubstance(T + h, P, "Na+").gibbs_energy.val - e.thermoPropertiesSubstance(T - h, P, "Na+").gibbs_energy.val) / (2 * h)
        assert dG_dT == pytest.approx(-e.thermoPropertiesSubstance(T, P, "Na+").entropy.val, rel=1e-6), t
        assert a["HKF88:g"] < 0.0


def rho(e, T, P):
    return e.propertiesSolvent(T, P, "H2O@").density.val


@pytest.mark.parametrize("implementation", ["reaktoro", "gems"])
def test_g_is_held_below_350_kg_per_m3(implementation):
    e = engine(implementation)
    for T in (673.15, 873.15, 1073.15):
        steam = [e.auxiliaryOutputs("Na+", T, P)[0] for P in (1e5, 1e7)]
        assert rho(e, T, 1e7) < 350.0
        assert steam[0]["HKF88:g"] == pytest.approx(steam[1]["HKF88:g"], rel=1e-12)   # not a function of P
        assert -0.2 < steam[0]["HKF88:g"] < 0.0 and steam[0]["HKF88:re"] > 1.7
    # continuous at 350 kg/m3 (at 673.15 K about 298.1 bar)
    lo, hi = 100e5, 400e5
    for _ in range(60):
        mid = (lo + hi) / 2
        lo, hi = (mid, hi) if rho(e, 673.15, mid) < 350.0 else (lo, mid)
    below = e.auxiliaryOutputs("Na+", 673.15, lo * (1 - 1e-7))[0]["HKF88:g"]
    above = e.auxiliaryOutputs("Na+", 673.15, hi * (1 + 1e-7))[0]["HKF88:g"]
    assert below == pytest.approx(above, abs=1e-5)


@pytest.mark.parametrize("implementation", ["reaktoro", "gems"])
def test_the_properties_in_steam_are_consistent(implementation):
    e = engine(implementation)
    for T, P in ((673.15, 1e5), (873.15, 1e7), (1073.15, 1e7)):
        r = e.thermoPropertiesSubstance(T, P, "Na+")
        h, hp = 1e-3, P * 1e-6
        G = lambda T, P: e.thermoPropertiesSubstance(T, P, "Na+").gibbs_energy.val
        assert (G(T + h, P) - G(T - h, P)) / (2 * h) == pytest.approx(-r.entropy.val, rel=1e-6), (T, P)
        assert (G(T, P + hp) - G(T, P - hp)) / (2 * hp) == pytest.approx(r.volume.val, rel=1e-5), (T, P)


# V of Na+ (SUPCRT92 parameters) as dG/dP of the G of OptimB, cm3/mol: on both sides of 355 C and of 1000 bar
OPTIMB_V = [(400, 800, -6.1045), (400, 500, 2.7482), (400, 2000, -1.5959), (350, 800, -7.6168), (350, 500, -22.1134)]


@pytest.mark.parametrize("implementation", ["reaktoro", "gems"])
def test_the_volume_is_that_of_optimb(implementation):
    e = engine(implementation)
    for t, p, wanted in OPTIMB_V:
        # OptimB: G in single precision, its own HGK water, its V from a difference of G over 1 bar: 0.01 cm3/mol
        assert e.thermoPropertiesSubstance(t + 273.15, p * 1e5, "Na+").volume.val * 1e6 == pytest.approx(wanted, abs=1e-2), (t, p)
