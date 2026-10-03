# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

import math
import pytest
import thermofun as thermofun

# the derivatives are 0 if built with -DTFUN_USE_AUTODIFF=OFF
pytestmark = pytest.mark.skipif(not getattr(thermofun, "with_autodiff", True), reason="built with -DTFUN_USE_AUTODIFF=OFF")

# Substances: solid, gas, aqueous (HKF), and water solvent
SUBSTANCES = ["Quartz", "CO2@", "Ca+2", "H2O@"]
REACTION = "Cal = Ca+2 + CO3-2"

PROPS_SUBSTANCE = ["gibbs_energy", "enthalpy", "entropy", "heat_capacity_cp"]
PROPS_REACTION = ["reaction_gibbs_energy", "log_equilibrium_constant"]

# the first point is not the reference state (298.15 K, 1 bar), where some models switch branch
TP_POINTS = [(300.15, 1e5), (423.15, 4.8e5), (573.15, 1e7), (673.15, 3000e5)]


@pytest.fixture(scope="module")
def engine():
    return thermofun.ThermoEngine('pytests/test-thermoengine-thermofun.json')


def central_difference(fn, prop, T, P, wrt, step=1e-3):
    """Central finite difference of the value of `prop` with respect to T or P."""
    h = step * T if wrt == "T" else step * P
    if wrt == "T":
        up, dn = getattr(fn(T + h, P), prop).val, getattr(fn(T - h, P), prop).val
    else:
        up, dn = getattr(fn(T, P + h), prop).val, getattr(fn(T, P - h), prop).val
    return (up - dn) / (2 * h)


@pytest.mark.parametrize("T,P", TP_POINTS)
@pytest.mark.parametrize("substance", SUBSTANCES)
def test_substance_derivatives_match_finite_differences(engine, substance, T, P):
    fn = lambda t, p: engine.thermoPropertiesSubstance(t, p, substance)
    tps = fn(T, P)
    for prop in PROPS_SUBSTANCE:
        scale = max(abs(getattr(tps, prop).val), 1.0)
        for wrt, ad in (("T", getattr(tps, prop).ddt), ("P", getattr(tps, prop).ddp)):
            fd = central_difference(fn, prop, T, P, wrt)
            h = 1e-3 * (T if wrt == "T" else P)
            assert ad == pytest.approx(fd, rel=1e-3, abs=1e-6 * scale / h * 1e-3), f"{substance} d{prop}/d{wrt}"


@pytest.mark.parametrize("T,P", TP_POINTS)
def test_reaction_derivatives_match_finite_differences(engine, T, P):
    fn = lambda t, p: engine.thermoPropertiesReaction(t, p, REACTION)
    tpr = fn(T, P)
    for prop in PROPS_REACTION:
        scale = max(abs(getattr(tpr, prop).val), 1.0)
        for wrt, ad in (("T", getattr(tpr, prop).ddt), ("P", getattr(tpr, prop).ddp)):
            fd = central_difference(fn, prop, T, P, wrt)
            h = 1e-3 * (T if wrt == "T" else P)
            assert ad == pytest.approx(fd, rel=1e-3, abs=1e-6 * scale / h * 1e-3), f"{prop} d/d{wrt}"


@pytest.mark.parametrize("substance", ["Quartz", "CO2@", "Ca+2", "H2O@"])
def test_thermodynamic_consistency_of_derivatives(engine, substance):
    # S = -dG/dT, Cp = dH/dT and Cp = T dS/dT must hold through the derivatives
    T, P = 423.15, 4.8e5
    tps = engine.thermoPropertiesSubstance(T, P, substance)
    assert tps.gibbs_energy.ddt == pytest.approx(-tps.entropy.val, rel=1e-5, abs=1e-8)
    assert tps.enthalpy.ddt == pytest.approx(tps.heat_capacity_cp.val, rel=1e-5, abs=1e-8)
    assert T * tps.entropy.ddt == pytest.approx(tps.heat_capacity_cp.val, rel=1e-5, abs=1e-8)


def test_scalar_api():
    s = thermofun.ThermoScalar(2.0, 3.0, 4.0, 0.1, (thermofun.Status.assigned, ""))
    assert (s.val, s.ddt, s.ddp) == (2.0, 3.0, 4.0)
    s.val, s.ddt, s.ddp = 5.0, 6.0, 7.0
    assert (s.val, s.ddt, s.ddp) == (5.0, 6.0, 7.0)


def test_status_notdefined_is_preserved(engine):
    # a default ThermoScalar is not defined; defined values reported by a model are not
    assert thermofun.ThermoScalar().sta[0] == thermofun.Status.notdefined
    tps = engine.thermoPropertiesSubstance(298.15, 1e5, "Quartz")
    assert tps.gibbs_energy.sta[0] != thermofun.Status.notdefined


@pytest.mark.parametrize("T,P", [(423.15, 4.8e5), (600.0, 4.8e5), (873.15, 5000e5), (1000.0, 5000e5)])
def test_landau_volume_is_pressure_derivative_of_gibbs_energy(engine, T, P):
    # Quartz (Holland-Powell Landau model): V = dG/dP, below and above the critical temperature
    tps = engine.thermoPropertiesSubstance(T, P, "Quartz")
    assert tps.gibbs_energy.ddp * 1e5 == pytest.approx(tps.volume.val, rel=1e-9)


def test_multi_interval_cp_integration_derivatives():
    # Pyrrhotite has several Cp temperature intervals (phase transitions): dG/dT = -S, dH/dT = Cp, dS/dT = Cp/T
    engine = thermofun.ThermoEngine('pytests/mines16-thermofun.json')
    for T, P in [(423.15, 4.8e5), (573.15, 1e7)]:
        tps = engine.thermoPropertiesSubstance(T, P, "Pyrrhotite")
        assert tps.gibbs_energy.ddt == pytest.approx(-tps.entropy.val, rel=1e-6)
        assert tps.enthalpy.ddt == pytest.approx(tps.heat_capacity_cp.val, rel=1e-6)
        assert T * tps.entropy.ddt == pytest.approx(tps.heat_capacity_cp.val, rel=1e-6)


def test_reaction_entropy_derivative():
    # dS/dT = Cp/T for a reaction whose entropy is calculated from its enthalpy and Gibbs energy
    engine = thermofun.ThermoEngine('pytests/mines16-thermofun.json')
    T, P = 298.15, 1e5
    tpr = engine.thermoPropertiesReaction(T, P, "Sn(Cl)+")
    assert T * tpr.reaction_entropy.ddt == pytest.approx(tpr.reaction_heat_capacity_cp.val, rel=1e-6)


GEMS_WATER_POINTS = [(298.15, 1e5), (473.15, 1e7), (573.15, 5e7), (623.15, 2.5e7), (800.0, 1e8), (900.0, 5e8)]


@pytest.fixture(scope="module")
def gems_engine():
    # water with the HGK/LVS equation of state and the Johnson-Norton dielectric constant (GEMS implementation)
    return thermofun.ThermoEngine('pytests/PsiTDB2020-subset-thermofun.json')


@pytest.mark.parametrize("T,P", GEMS_WATER_POINTS)
def test_gems_water_derivatives_match_finite_differences(gems_engine, T, P):
    fn = lambda t, p: gems_engine.propertiesSolvent(t, p, "H2O(l)")
    ps = fn(T, P)
    for prop in ("density", "Alpha", "Beta", "dAldT"):
        for wrt, ad in (("T", getattr(ps, prop).ddt), ("P", getattr(ps, prop).ddp)):
            fd = central_difference(fn, prop, T, P, wrt, 1e-4)
            assert ad == pytest.approx(fd, rel=3e-3, abs=1e-12), f"{prop} d/d{wrt}"

    ft = lambda t, p: gems_engine.thermoPropertiesSubstance(t, p, "H2O(l)")
    tps = ft(T, P)
    for prop in ("gibbs_energy", "entropy", "heat_capacity_cp", "volume"):
        for wrt, ad in (("T", getattr(tps, prop).ddt), ("P", getattr(tps, prop).ddp)):
            fd = central_difference(ft, prop, T, P, wrt, 1e-4)
            assert ad == pytest.approx(fd, rel=3e-3, abs=1e-12), f"{prop} d/d{wrt}"
    # the consistency of the thermodynamic derivatives
    assert tps.gibbs_energy.ddt == pytest.approx(-tps.entropy.val, rel=1e-6)
    assert tps.enthalpy.ddt == pytest.approx(tps.heat_capacity_cp.val, rel=1e-6)
    assert T * tps.entropy.ddt == pytest.approx(tps.heat_capacity_cp.val, rel=1e-6)
    # Maxwell relation: dS/dP = -dV/dT (V in J/bar, P in Pa)
    assert tps.entropy.ddp == pytest.approx(-tps.volume.ddt * 1e-5, rel=1e-4)


@pytest.mark.parametrize("T,P", GEMS_WATER_POINTS)
def test_gems_water_dielectric_constant_derivatives(gems_engine, T, P):
    eps = gems_engine.electroPropertiesSolvent(T, P, "H2O(l)")
    # d(epsilon)/dT is the model's epsilonT; d(epsilon)/dP (per Pa) is epsilonP (per bar)
    assert eps.epsilon.ddt == pytest.approx(eps.epsilonT.val, rel=1e-4)
    assert eps.epsilon.ddp * 1e5 == pytest.approx(eps.epsilonP.val, rel=1e-4)
    assert eps.bornZ.ddt == pytest.approx(eps.bornY.val, rel=1e-4)


def test_gems_water_saturation_line(gems_engine):
    # at the saturation pressure (P = 0) the derivatives are those along the saturation line
    for T in (373.15, 473.15, 573.15):
        fn = lambda t: gems_engine.thermoPropertiesSubstance(t, 0, "H2O(l)")
        h = 1e-4 * T
        for prop in ("gibbs_energy", "entropy", "volume"):
            fd = (getattr(fn(T + h), prop).val - getattr(fn(T - h), prop).val) / (2 * h)
            assert getattr(fn(T), prop).ddt == pytest.approx(fd, rel=1e-4)


def test_substance_from_reaction_pressure_derivative(gems_engine):
    # H2PO4- is calculated from a reaction with a constant reaction volume: dG/dP = V
    for T, P in [(299.15, 1e5), (423.15, 4.8e5), (573.15, 1e7)]:
        tpr = gems_engine.thermoPropertiesReaction(T, P, "H2PO4-")
        assert tpr.reaction_gibbs_energy.ddp * 1e5 == pytest.approx(tpr.reaction_volume.val, rel=1e-9)
        tps = gems_engine.thermoPropertiesSubstance(T, P, "H2PO4-")
        assert tps.gibbs_energy.ddp * 1e5 == pytest.approx(tps.volume.val, rel=1e-3)


def test_gas_derivatives_match_finite_differences():
    # fluids calculated with equations of state (PRSV and CORK) in the mines16 and aq17 databases
    for database, symbol in (("pytests/mines16-thermofun.json", "CO"), ("pytests/test-aq17-gem-lma-thermofun.json", "CO2")):
        engine = thermofun.ThermoEngine(database)
        fn = lambda t, p: engine.thermoPropertiesSubstance(t, p, symbol)
        for T, P in [(400.0, 1e5), (600.0, 1e7), (800.0, 1e8)]:
            tps = fn(T, P)
            for prop in ("gibbs_energy", "enthalpy", "entropy", "volume"):
                for wrt, ad in (("T", getattr(tps, prop).ddt), ("P", getattr(tps, prop).ddp)):
                    fd = central_difference(fn, prop, T, P, wrt, 1e-4)
                    assert ad == pytest.approx(fd, rel=2e-3, abs=1e-12), f"{symbol} {prop} d/d{wrt}"



def test_gems_hgk_density_tp_has_exact_third_derivatives():
    """densityTP = d(density)/dT dP of the GEMS HGK water: its ddt and ddp are the mixed third derivatives."""
    f = 'pytests/PsiTDB2020-subset-thermofun.json'
    e = thermofun.ThermoEngine(f)
    e.setSolventSymbol("H2O(l)")
    for T, P in [(373.15, 1e7), (573.15, 5e7)]:
        ps = e.propertiesSolvent(T, P, "H2O(l)")
        hP, hT = 1e-3 * P, 1e-2
        fd_p = (e.propertiesSolvent(T, P + hP, "H2O(l)").densityTP.val - e.propertiesSolvent(T, P - hP, "H2O(l)").densityTP.val) / (2 * hP)
        fd_t = (e.propertiesSolvent(T + hT, P, "H2O(l)").densityTP.val - e.propertiesSolvent(T - hT, P, "H2O(l)").densityTP.val) / (2 * hT)
        assert ps.densityTP.ddp == pytest.approx(fd_p, rel=1e-4)
        assert ps.densityTP.ddt == pytest.approx(fd_t, rel=1e-4)
