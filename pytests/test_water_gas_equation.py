# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# WaterIAPWS95Gas: the gas equation of Wagner and Pruss (2002), Eq. 3.2, used for the metastable vapour (above the saturation
# pressure, below 10 MPa) of a vapour record, before WaterIAPWS95 (decision 0028). IAPWS-95 itself is not reliable there away
# from the saturated vapour line (Sects. 6.3.1 and 7.3.2.3 of the paper).

import pytest
import thermofun as tf

GAS_THEN_IAPWS = [{"method": "WaterIAPWS95Gas"}, {"method": "WaterIAPWS95"}]
IAPWS = [{"method": "WaterIAPWS95"}]


def engine(methods, state="gas"):
    water = {"formula": "H2O", "class": "water_solvent", "aggregate_state": state, "standard_state_properties": {"G": -228572}, "methods": methods}
    return tf.ThermoEngine(tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"W": water}}))


def V(e, T, P):
    return e.thermoPropertiesSubstance(T, P, "W").volume.val


@pytest.mark.parametrize("T", [300.0, 350.0, 373.15, 400.0, 450.0, 500.0, 550.0])
def test_gas_equation_agrees_with_iapws95_on_the_saturated_vapour_line(T):
    """The gas equation was fitted to the same data: close to the saturated vapour line the two agree to 4e-4 in V."""
    gas, iapws = engine(GAS_THEN_IAPWS), engine(IAPWS)
    P = iapws.saturationPressure(T) * (1 + 1e-6)
    assert V(gas, T, P) == pytest.approx(V(iapws, T, P), rel=4e-4)


def test_gas_equation_gives_the_vapour_where_iapws95_has_none():
    """400 K, 3 times the saturation pressure: beyond the vapour spinodal of IAPWS-95, which gives the liquid root."""
    gas, iapws = engine(GAS_THEN_IAPWS), engine(IAPWS)
    P = 3 * iapws.saturationPressure(400.0)
    assert V(iapws, 400.0, P) < 1e-4
    assert V(gas, 400.0, P) == pytest.approx(4.04e-3, rel=1e-2)


def test_metastable_vapour_of_a_vapour_record_below_the_saturation_pressure_of_the_next_method():
    """298.15 K, 1 bar: IAPWS-95 alone gave an error (now the liquid root); the gas equation gives a vapour."""
    assert V(engine(GAS_THEN_IAPWS), 298.15, 1e5) == pytest.approx(2.26e-2, rel=2e-2)


@pytest.mark.parametrize("T,P", [(373.15, 101325.0 * 0.5), (473.15, 10e5), (300.0, 0.0), (473.15, 0.0), (550.0, 4e6)])
def test_stable_vapour_and_saturation_stay_with_iapws95(T, P):
    gas, iapws = engine(GAS_THEN_IAPWS), engine(IAPWS)
    a, b = gas.thermoPropertiesSubstance(T, P, "W"), iapws.thermoPropertiesSubstance(T, P, "W")
    assert a.gibbs_energy.val == b.gibbs_energy.val
    assert a.volume.val == b.volume.val


def test_a_liquid_record_does_not_use_the_gas_equation():
    gas, iapws = engine(GAS_THEN_IAPWS, "aqueous"), engine(IAPWS, "aqueous")
    for T, P in ((298.15, 1e5), (400.0, 7.4e5), (473.15, 50e5)):
        assert V(gas, T, P) == V(iapws, T, P)


def test_above_10_mpa_and_above_the_critical_temperature_it_is_not_used():
    gas, iapws = engine(GAS_THEN_IAPWS), engine(IAPWS)
    for T, P in ((500.0, 1.5e7), (700.0, 5e6)):
        assert V(gas, T, P) == V(iapws, T, P)


def test_the_gas_equation_alone_is_refused():
    with pytest.raises(RuntimeError, match="WaterIAPWS95Gas is used only in its region"):
        engine([{"method": "WaterIAPWS95Gas"}])


def test_derivatives_of_the_gas_equation_agree_with_finite_differences():
    """dG/dP = V and dG/dT = -S in the region of the gas equation."""
    e = engine(GAS_THEN_IAPWS)
    T, P = 400.0, 7.4e5
    r = e.thermoPropertiesSubstance(T, P, "W")
    h = 1e-3
    dGdP = (e.thermoPropertiesSubstance(T, P * (1 + h), "W").gibbs_energy.val - e.thermoPropertiesSubstance(T, P * (1 - h), "W").gibbs_energy.val) / (2 * P * h)
    assert dGdP == pytest.approx(r.volume.val, rel=1e-4)
