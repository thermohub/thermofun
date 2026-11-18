# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# WaterLVS83: the equation of Levelt Sengers et al. (1983) near the critical point of water, used in its region before
# WaterHGK84 (decision 0028), as in SUPCRT92 and GEMS.

import math
import pathlib

import pytest
import thermofun as tf

HERE = pathlib.Path(__file__).resolve().parent
CAL = 4.184


def engine(methods):
    water = {"formula": "H2O@", "class": "water_solvent", "aggregate_state": "aqueous", "standard_state_properties": {"G": -237181},
             "methods": methods + [{"method": "JohnsonNorton91", "implementation": "gems"}]}
    na = {"formula": "Na+", "charge": 1, "class": "aqueous_solute", "aggregate_state": "aqueous",
          "standard_state_properties": {"G": -62591 * CAL, "H": -57433 * CAL, "S": 13.96 * CAL},
          "methods": [{"method": "HKF88", "implementation": "gems", "units": {"energy": "cal", "pressure": "bar"},
                       "parameters": {"a1": 0.1839, "a2": -228.5, "a3": 3.256, "a4": -27260, "c1": 18.18, "c2": -29810, "omega": 33060}}]}
    return tf.ThermoEngine(tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"H2O@": water, "Na+": na}}))


GEMS = [{"method": "WaterLVS83"}, {"method": "WaterHGK84", "implementation": "gems"}]
HGK = [{"method": "WaterHGK84", "implementation": "gems"}]


def test_the_water_of_gems_in_the_old_format_is_lvs_and_hgk():
    db = tf.Database(str(HERE / "Substances" / "Solvent" / "water-thermofun.json"))
    methods = [m["method"] for m in db.get("/substances/H2O@/methods")]
    assert methods[:2] == ["WaterLVS83", "WaterHGK84"]


@pytest.mark.parametrize("T", [643.5, 645.0, 646.0, 647.0])
def test_the_saturation_of_lvs_is_that_of_water(T):
    """LVS of SUPCRT92 has Tc = 647.067 K (IPTS-68), IAPWS-95 647.096 K (ITS-90): its Psat is about 5e-4 lower."""
    iapws = engine([{"method": "WaterIAPWS95"}]).saturationPressure(T)
    assert engine(GEMS).saturationPressure(T) == pytest.approx(iapws, rel=1e-3)


@pytest.mark.parametrize("T", [643.2, 644.0, 645.0, 646.0, 646.6, 647.0])
@pytest.mark.parametrize("f", [0.0, 0.98, 0.999, 0.9999999, 1.0000001, 1.001, 1.02])
def test_lvs_gives_a_density_in_its_window(T, f):
    e = engine(GEMS)
    P = f * e.saturationPressure(T)
    for state in (0, 1):
        rho = e.propertiesSolvent(T, P, "H2O@", state).density.val
        assert math.isfinite(rho) and rho > 50.0, (T, f, state)
    assert math.isfinite(e.thermoPropertiesSubstance(T, P, "Na+").gibbs_energy.val)


@pytest.mark.parametrize("T", [645.5, 646.0, 646.6, 647.0])
def test_above_tmin2_the_liquid_is_continuous_at_the_saturation_pressure(T):
    """From 645.3 K the region of LVS holds both sides of the saturation curve."""
    e = engine(GEMS)
    Psat = e.saturationPressure(T)
    at_sat = e.propertiesSolvent(T, 0.0, "H2O@", 0).density.val
    above = e.propertiesSolvent(T, Psat * (1 + 1e-9), "H2O@", 0).density.val
    assert above == pytest.approx(at_sat, rel=1e-5)


def test_lvs_is_used_in_its_region_and_hgk_outside():
    gems, hgk = engine(GEMS), engine(HGK)
    inside = (670.0, 2.8e7)
    outside = (670.0, 1.0e7)
    assert gems.propertiesSolvent(*inside, "H2O@").density.val != pytest.approx(hgk.propertiesSolvent(*inside, "H2O@").density.val, rel=1e-6)
    assert gems.propertiesSolvent(*outside, "H2O@").density.val == hgk.propertiesSolvent(*outside, "H2O@").density.val
    assert gems.saturationPressure(600.0) == hgk.saturationPressure(600.0)


def test_lvs_needs_a_base_method_after_it():
    with pytest.raises(RuntimeError, match="WaterLVS83 is used only in its region"):
        engine([{"method": "WaterLVS83"}])


def test_hgk_alone_says_where_it_has_no_saturation():
    """HGK alone has no usable saturation in the last kelvin below its critical point; the error names WaterLVS83."""
    e = engine(HGK)
    for T in [646.0 + 0.0007 * i for i in range(1500)]:
        try:
            value = e.saturationPressure(T)
        except Exception as error:
            assert "WaterLVS83" in str(error)
            return
        assert math.isfinite(value)


def test_transport_properties_in_si():
    """Water at 25 degC and 1 bar: 0.890 mPa s, 0.607 W/(m K), 1497 m/s (IAPWS)."""
    s = engine(GEMS).propertiesSolvent(298.15, 1e5, "H2O@")
    assert s.dynamic_viscosity == pytest.approx(0.890e-3, rel=5e-3)
    assert s.thermal_conductivity == pytest.approx(0.607, rel=5e-3)
    assert s.speed_of_sound == pytest.approx(1497.0, rel=2e-3)
    assert engine(GEMS).propertiesSolvent(298.15, 0.0, "H2O@").surface_tension == pytest.approx(0.0720, rel=5e-3)


@pytest.mark.parametrize("T,P", [(650.0, 2.3e7), (670.0, 2.8e7), (646.5, 2.25e7), (644.0, 2.1e7)])
def test_lvs_is_consistent(T, P):
    e = engine(GEMS)
    t = e.thermoPropertiesSubstance(T, P, "H2O@")
    h = 1e-4
    dGdT = (e.thermoPropertiesSubstance(T + h, P, "H2O@").gibbs_energy.val - e.thermoPropertiesSubstance(T - h, P, "H2O@").gibbs_energy.val) / (2 * h)
    assert t.gibbs_energy.ddt == pytest.approx(-t.entropy.val, rel=1e-6)
    assert t.gibbs_energy.ddt == pytest.approx(dGdT, rel=1e-6)
    dP = P * 1e-6
    dGdP = (e.thermoPropertiesSubstance(T, P + dP, "H2O@").gibbs_energy.val - e.thermoPropertiesSubstance(T, P - dP, "H2O@").gibbs_energy.val) / (2 * dP)
    assert t.gibbs_energy.ddp == pytest.approx(dGdP, rel=1e-5)
