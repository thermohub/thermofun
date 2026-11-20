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
