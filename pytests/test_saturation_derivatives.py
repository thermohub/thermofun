# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The water of GEMS at saturation (P = 0 or P = Psat): P is a function of T alone there, but the derivatives with respect to P of
# the properties of the phase are those of the phase at constant T (dG/dP = V, dS/dP = -V alpha, ...). They were 0.

import pytest
import thermofun as tf

HGK = [{"method": "WaterHGK84", "implementation": "gems"}]
LVS_HGK = [{"method": "WaterLVS83"}] + HGK


def engine(methods):
    water = {"formula": "H2O@", "class": "water_solvent", "aggregate_state": "aqueous", "standard_state_properties": {"G": -237181},
             "methods": methods + [{"method": "JohnsonNorton91", "implementation": "gems"}]}
    return tf.ThermoEngine(tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"H2O@": water}}))


@pytest.mark.parametrize("methods,T", [(HGK, 373.15), (HGK, 473.15), (HGK, 573.15), (LVS_HGK, 646.5)])
@pytest.mark.parametrize("P_is_zero", [True, False])
def test_dG_dP_at_saturation_is_the_volume(methods, T, P_is_zero):
    e = engine(methods)
    P = 0.0 if P_is_zero else e.saturationPressure(T)
    r = e.thermoPropertiesSubstance(T, P, "H2O@")
    assert r.gibbs_energy.ddp == pytest.approx(r.volume.val, rel=1e-12)


@pytest.mark.parametrize("methods,T,offset", [(HGK, 473.15, -1e-4), (HGK, 573.15, 1e-4), (LVS_HGK, 646.5, 1e-6)])
def test_the_derivatives_at_saturation_are_those_of_the_phase_next_to_it(methods, T, offset):
    """The same phase just off the saturation curve, by finite differences (HGK has the metastable phase on both sides, LVS the liquid)."""
    e = engine(methods)
    P = e.saturationPressure(T)
    at = e.thermoPropertiesSubstance(T, 0.0, "H2O@")
    sign = 1 if offset > 0 else -1
    a = e.thermoPropertiesSubstance(T, P * (1 + sign * 2 * abs(offset)), "H2O@")
    b = e.thermoPropertiesSubstance(T, P * (1 + sign * abs(offset)), "H2O@")
    for name in ("gibbs_energy", "enthalpy", "entropy", "heat_capacity_cp", "volume"):
        fd = (getattr(a, name).val - getattr(b, name).val) / (sign * abs(offset) * P)
        assert getattr(at, name).ddp == pytest.approx(fd, rel=1e-2), name
