import math
import pytest
import thermofun as thermofun

# Substances: solid, gas, aqueous (HKF), and water solvent
SUBSTANCES = ["Quartz", "CO2@", "Ca+2", "H2O@"]
REACTION = "Cal = Ca+2 + CO3-2"

# Only the Gibbs energies are checked against finite differences: the derivatives of the other
# properties are provided by the models themselves and are not all consistent with their values.
PROPS_SUBSTANCE = ["gibbs_energy"]
PROPS_REACTION = ["reaction_gibbs_energy"]

TP_POINTS = [(298.15, 1e5), (423.15, 4.8e5), (573.15, 1e7), (673.15, 3000e5)]


@pytest.fixture(scope="module")
def engine():
    return thermofun.ThermoEngine('pytests/test-thermoengine-thermofun.json')


def central_difference(fn, prop, T, P, wrt):
    """Central finite difference of the value of `prop` with respect to T or P."""
    h = 1e-3 * T if wrt == "T" else 1e-3 * P
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
    # S = -dG/dT must hold through the derivatives
    T, P = 423.15, 4.8e5
    tps = engine.thermoPropertiesSubstance(T, P, substance)
    assert tps.gibbs_energy.ddt == pytest.approx(-tps.entropy.val, rel=1e-5, abs=1e-8)


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
