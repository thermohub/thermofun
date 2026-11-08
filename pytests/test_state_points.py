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
