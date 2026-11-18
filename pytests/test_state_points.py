# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# A state point without a meaning is refused at every entry point: T or P not finite, T <= 0, P < 0, and P = 0 (the
# saturation pressure of water) at or above the critical temperature of water (review of v2, S3 and Q3).

import math
import pathlib

import pytest
import thermofun as tf

HERE = pathlib.Path(__file__).resolve().parent
DB = str(HERE / "test-aq17-gem-lma-thermofun.json")
TCR = 647.096


def calls(engine):
    return {
        "mineral": lambda T, P: engine.thermoPropertiesSubstance(T, P, "Calcite"),
        "solute": lambda T, P: engine.thermoPropertiesSubstance(T, P, "Ca+2"),
        "solvent": lambda T, P: engine.propertiesSolvent(T, P, "H2O@"),
        "dielectric": lambda T, P: engine.electroPropertiesSolvent(T, P, "H2O@"),
        "reaction": lambda T, P: engine.thermoPropertiesReaction(T, P, "Calcite = Ca+2 + CO3-2"),
    }


@pytest.mark.parametrize("what", ["mineral", "solute", "solvent", "dielectric", "reaction"])
@pytest.mark.parametrize("T", [TCR, 700.0, 1273.15])
def test_zero_pressure_above_the_critical_temperature_is_refused(what, T):
    with pytest.raises(ValueError, match="saturation pressure of water"):
        calls(tf.ThermoEngine(DB))[what](T, 0.0)


@pytest.mark.parametrize("what", ["mineral", "solute", "solvent", "dielectric", "reaction"])
def test_zero_pressure_below_the_critical_temperature_is_the_saturation_pressure(what):
    calls(tf.ThermoEngine(DB))[what](473.15, 0.0)


@pytest.mark.parametrize("T,P", [(math.nan, 1e5), (298.15, math.nan), (math.inf, 1e5), (0.0, 1e5), (-10.0, 1e5), (298.15, -1e5)])
def test_a_state_point_without_a_meaning_is_refused(T, P):
    with pytest.raises(ValueError):
        tf.ThermoEngine(DB).thermoPropertiesSubstance(T, P, "Ca+2")


def test_nan_is_not_served_from_the_cache():
    engine = tf.ThermoEngine(DB)
    engine.thermoPropertiesSubstance(298.15, 1e5, "Ca+2")
    with pytest.raises(ValueError):
        engine.thermoPropertiesSubstance(math.nan, 1e5, "Ca+2")


def test_zero_pressure_is_the_same_saturation_pressure_for_every_record():
    """A mineral took P = 0 as 0 Pa, a solute as Psat; in a reaction the order of the reactants decided (review Q3)."""
    engine = tf.ThermoEngine(DB)
    T = 473.15
    Psat = engine.saturationPressure(T)
    assert Psat == pytest.approx(15.5e5, rel=0.01)
    for symbol in ("Calcite", "Ca+2", "CO3-2", "H2O@"):
        at_zero = tf.ThermoEngine(DB).thermoPropertiesSubstance(T, 0.0, symbol).gibbs_energy.val
        at_psat = tf.ThermoEngine(DB).thermoPropertiesSubstance(T, Psat, symbol).gibbs_energy.val
        assert at_zero == pytest.approx(at_psat, rel=1e-9, abs=1e-6), symbol


def test_a_reaction_at_zero_pressure_is_at_the_saturation_pressure_whatever_the_order_of_its_reactants():
    """A reaction from reactants passes one P to its species in order: Calcite first used to run at 0 Pa (review Q3)."""
    T = 473.15
    Psat = tf.ThermoEngine(DB).saturationPressure(T)
    # the reactants are taken in the order of their symbols: Brucite and Aragonite come before the aqueous species
    for equation in ("Brucite = Mg+2 + 2OH-", "Aragonite = Ca+2 + CO3-2", "Calcite = Ca+2 + CO3-2"):
        at_zero = tf.ThermoEngine(DB).thermoPropertiesReaction(T, 0.0, equation).reaction_gibbs_energy.val
        at_psat = tf.ThermoEngine(DB).thermoPropertiesReaction(T, Psat, equation).reaction_gibbs_energy.val
        assert at_zero == pytest.approx(at_psat, rel=1e-9, abs=1e-6), equation


def test_saturation_pressure_without_a_water_record_and_above_the_critical_temperature():
    engine = tf.ThermoEngine(str(HERE / "Substances" / "Solids" / "Mo(s).json"))
    assert engine.saturationPressure(373.15) == pytest.approx(101418.0, rel=1e-3)  # Wagner-Pruss: 1.01418 bar at 100 degC
    with pytest.raises(ValueError, match="saturation pressure of water"):
        engine.saturationPressure(TCR)


def test_a_substance_defined_by_a_reaction_with_a_volume_is_at_the_saturation_pressure():
    """NpO2SiO3@ of PSI/Nagra: its reaction has a volume and no water, and was taken at 0 Pa (293 J/mol at 573 K)."""
    db = str(HERE.parent / "Resources" / "databases" / "psinagra-12-07-thermofun.json")
    T = 573.15
    Psat = tf.ThermoEngine(db).saturationPressure(T)
    for symbol in ("NpO2SiO3@", "PuO2SiO3@"):
        at_zero = tf.ThermoEngine(db).thermoPropertiesSubstance(T, 0.0, symbol)
        at_psat = tf.ThermoEngine(db).thermoPropertiesSubstance(T, Psat, symbol)
        assert at_zero.gibbs_energy.val == pytest.approx(at_psat.gibbs_energy.val, rel=1e-9), symbol
        assert at_zero.enthalpy.val == pytest.approx(at_psat.enthalpy.val, rel=1e-9), symbol


def _na_engine(implementation):
    cal = 4.184
    water = {"formula": "H2O@", "class": "water_solvent", "aggregate_state": "aqueous", "standard_state_properties": {"G": -237181},
             "methods": [{"method": "WaterHGK84", "implementation": implementation},
                         {"method": "JohnsonNorton91", "implementation": implementation}]}
    na = {"formula": "Na+", "charge": 1, "class": "aqueous_solute", "aggregate_state": "aqueous",
          "standard_state_properties": {"G": -62591 * cal, "H": -57433 * cal, "S": 13.96 * cal},
          "methods": [{"method": "HKF88", "implementation": implementation, "units": {"energy": "cal", "pressure": "bar"},
                       "parameters": {"a1": 0.1839, "a2": -228.5, "a3": 3.256, "a4": -27260, "c1": 18.18, "c2": -29810, "omega": 33060}}]}
    return tf.ThermoEngine(tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"H2O@": water, "Na+": na}}))


# The water of the implementation gems took the saturation branch only at P == Psat to 1e-14 bar. The engine writes Psat in Pa to
# P and calls the water again with it (for the dielectric constant): back in bar it missed that test at these T, and gave vapour.
@pytest.mark.parametrize("tc", [299.0, 299.3, 300.9, 301.0, 301.6, 312.7, 320.8, 327.5, 371.9])
def test_water_gems_is_liquid_at_the_saturation_pressure(tc):
    T = tc + 273.15
    gems, reaktoro = _na_engine("gems"), _na_engine("reaktoro")
    expected = reaktoro.thermoPropertiesSubstance(T, 0.0, "Na+").gibbs_energy.val
    Psat = gems.saturationPressure(T)
    for P in (0.0, Psat):
        G = _na_engine("gems").thermoPropertiesSubstance(T, P, "Na+").gibbs_energy.val
        assert G == pytest.approx(expected, abs=200.0), (tc, P)  # vapour was 74-130 kJ/mol off


# One saturation pressure in the water gems: that of its equation of state. Before, the correlation PsHGK (up to 7e-5 lower)
# was written back, and a P between the two was vapour. The values at P = 0 follow the saturation curve of the equation of state,
# and their T derivatives are those of that curve.
@pytest.mark.parametrize("tc", [100.0, 181.6, 250.0, 300.0, 360.0])
def test_water_gems_is_the_same_liquid_at_and_next_to_its_saturation_pressure(tc):
    T = tc + 273.15
    engine = _na_engine("gems")
    Psat = engine.saturationPressure(T)
    at_sat = engine.thermoPropertiesSubstance(T, 0.0, "Na+").gibbs_energy.val
    for f in (1 - 1e-6, 1 + 1e-6):
        G = engine.thermoPropertiesSubstance(T, Psat * f, "Na+").gibbs_energy.val
        assert G == pytest.approx(at_sat, abs=1.0), (tc, f)


@pytest.mark.parametrize("T", [373.15, 473.15, 573.15, 623.15])
def test_water_gems_temperature_derivative_at_the_saturation_pressure(T):
    engine = tf.ThermoEngine(str(HERE / "Substances" / "Solvent" / "water-thermofun.json"))
    h = 1e-3
    exact = engine.thermoPropertiesSubstance(T, 0.0, "H2O@").enthalpy.ddt
    fd = (engine.thermoPropertiesSubstance(T + h, 0.0, "H2O@").enthalpy.val - engine.thermoPropertiesSubstance(T - h, 0.0, "H2O@").enthalpy.val) / (2 * h)
    assert exact == pytest.approx(fd, rel=1e-6)
