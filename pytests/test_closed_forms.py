# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# Sweeps in T (review of v2, S10): each family of methods against its closed formula, and every record of the bundled
# databases against the relations that hold for any method (dG/dT = -S, dH/dT = Cp, dG/dP = V; for a reaction also
# dG = -RT ln10 log K and dG = dH - T dS). A sweep of this kind would have found B2 (log K) and S5 (transitions).

import glob
import math

import pytest
import thermofun as tf

R, LN10, TR, PR = 8.31451, math.log(10.0), 298.15, 1e5
TS = [298.15, 350.0, 450.0, 600.0, 800.0, 1000.0, 1400.0]
G0, H0, S0, CP0, V0 = -856288.0, -910700.0, 41.46, 44.6, 2.269e-5


def engine(methods, extra=""):
    text = "\n".join(["thermohub: 2.0.0-draft.1", "substances:", "  X:", "    formula: SiO2", "    class: component",
                      "    aggregate_state: " + ("gas" if "IdealGas" in methods else "crystal"),
                      "    standard_state_properties: {G: %r, H: %r, S: %r, Cp: %r, V: %r}" % (G0, H0, S0, CP0, V0),
                      "    methods:"] + ["      - " + m for m in methods]) + "\n" + extra
    return tf.ThermoEngine(text)


def check(p, G, H, S, Cp, T):
    assert p.gibbs_energy.val == pytest.approx(G, rel=1e-11, abs=1e-6), T
    assert p.enthalpy.val == pytest.approx(H, rel=1e-11, abs=1e-6), T
    assert p.entropy.val == pytest.approx(S, rel=1e-11, abs=1e-9), T
    assert p.heat_capacity_cp.val == pytest.approx(Cp, rel=1e-11, abs=1e-9), T


def from_integrals(T, dH, dS):
    """G, H, S from the integrals of Cp from Tr to T: dH = int Cp dT, dS = int Cp/T dT."""
    return G0 + dH - T * dS - S0 * (T - TR), H0 + dH, S0 + dS


@pytest.mark.parametrize("T", TS)
def test_constant_cp(T):
    G, H, S = from_integrals(T, CP0 * (T - TR), CP0 * math.log(T / TR))
    check(engine(["{method: ConstantCp}"]).thermoPropertiesSubstance(T, PR, "X"), G, H, S, CP0, T)


A, B, C = 46.94, 0.03431, -1.131e6          # Maier-Kelley: Cp = a + b T + c / T^2
A2 = 60.29                                   # the second segment: constant Cp
TB, DH, DS = 846.0, 290.0, 290.0 / 846.0     # the transition at the boundary


def mk_h(T1, T2):
    return A * (T2 - T1) + B / 2 * (T2 ** 2 - T1 ** 2) - C * (1 / T2 - 1 / T1)


def mk_s(T1, T2):
    return A * math.log(T2 / T1) + B * (T2 - T1) - C / 2 * (1 / T2 ** 2 - 1 / T1 ** 2)


@pytest.mark.parametrize("T", TS)
def test_cp_polynomial_of_maier_kelley_with_a_transition(T):
    segments = "[{t_min: 298.15, t_max: %r, parameters: {a0: %r, a1: %r, a2: %r}}, {t_min: %r, t_max: 2000, parameters: {a0: %r}}]" % (TB, A, B, C, TB, A2)
    e = engine(["{method: CpPolynomial, segments: %s, transitions: [{T: %r, dS: %r, dH: %r, dV: 0}]}" % (segments, TB, DS, DH)])
    if T < TB:
        dH, dS, Cp = mk_h(TR, T), mk_s(TR, T), A + B * T + C / T ** 2
    else:
        dH = mk_h(TR, TB) + DH + A2 * (T - TB)
        dS = mk_s(TR, TB) + DS + A2 * math.log(T / TB)
        Cp = A2
    G, H, S = from_integrals(T, dH, dS)
    check(e.thermoPropertiesSubstance(T, PR, "X"), G, H, S, Cp, T)


@pytest.mark.parametrize("T", TS)
@pytest.mark.parametrize("P", [PR, 1e8])
def test_constant_volume(T, P):
    p = engine(["{method: ConstantCp}", "{method: ConstantVolume}"]).thermoPropertiesSubstance(T, P, "X")
    G, H, S = from_integrals(T, CP0 * (T - TR), CP0 * math.log(T / TR))
    check(p, G + V0 * (P - PR), H + V0 * (P - PR), S, CP0, T)
    assert p.volume.val == pytest.approx(V0, rel=1e-12)


@pytest.mark.parametrize("T", TS)
@pytest.mark.parametrize("P", [PR, 1e7])
def test_ideal_gas(T, P):
    """G has RT ln(P/Pr) by default (apply_pressure_correction_to_gas_props); V = RT/P."""
    p = engine(["{method: ConstantCp}", "{method: IdealGas}"]).thermoPropertiesSubstance(T, P, "X")
    G, H, S = from_integrals(T, CP0 * (T - TR), CP0 * math.log(T / TR))
    check(p, G + R * T * math.log(P / PR), H, S - R * math.log(P / PR), CP0, T)
    assert p.volume.val == pytest.approx(R * T / P, rel=1e-12)


# The relations of every record of the bundled databases. The state points are in liquid water or above its critical
# point, where every aqueous model is defined.
POINTS = [(298.15, 1e5), (373.15, 5e7), (573.15, 5e7), (773.15, 1e8)]
DATABASES = sorted(glob.glob("Resources/databases/*-thermofun.json"))
FLUIDS = {"PRSV86", "CorkHP91", "ChurakovGottschalk03", "SoaveRedlichKwong72", "SternerPitzer94", "PengRobinson78"}

# The fluid methods, LandauHP98 and MurnaghanHP98 are consistent since 2026-11-08 (implementation consistent, the default);
# the implementation gems keeps the formulas of GEMS, which were not (tests at the end). The consistency is now checked
# with a tighter dH/dT, and at more points of the fluids: below and above the critical points, and above p0 of CORK.
# A substance from a reaction takes V from its record or from the reaction, and G changes with P as the reaction does:
# for these, dG/dP and V agree to 1e-9 m3/mol (0.001 cm3/mol), not better.
# HKF88 converts V with 41.84004 cm3 bar/cal, the constant of SUPCRT92, and G with 4.184 J/cal: dG/dP and V differ by
# 9.6e-7 relative in every HKF species and in every species made from one. The tolerance of dG/dP covers it.


def close(a, b, rel, scale):
    return abs(a - b) <= rel * max(abs(a), abs(b), scale)


def from_reaction(record):
    return bool(record.defining_reaction) and (not record.methods or str(record.preferred_property_source).endswith("Reaction"))


def substance_problems(engine, symbol, T, P, v_abs=0.0):
    try:
        p = engine.thermoPropertiesSubstance(T, P, symbol)
    except Exception:
        return []        # outside the range of the record, or no data: other tests check the errors
    G, H, S, Cp, V = p.gibbs_energy, p.enthalpy, p.entropy, p.heat_capacity_cp, p.volume
    out = []
    if not close(G.ddt, -S.val, 1e-6, 1.0):
        out.append("dG/dT %.12g, -S %.12g" % (G.ddt, -S.val))
    if not close(H.ddt, Cp.val, 1e-6, 1.0):
        out.append("dH/dT %.12g, Cp %.12g" % (H.ddt, Cp.val))
    if not close(G.ddp, V.val, 2e-6, 1e-6) and abs(G.ddp - V.val) > v_abs:
        out.append("dG/dP %.12g, V %.12g" % (G.ddp, V.val))
    return out


def reaction_problems(engine, symbol, T, P):
    try:
        r = engine.thermoPropertiesReaction(T, P, symbol)
    except Exception:
        return []
    G, H, S, logK = r.reaction_gibbs_energy, r.reaction_enthalpy, r.reaction_entropy, r.log_equilibrium_constant
    out = []
    if not close(G.val, -R * T * LN10 * logK.val, 1e-9, 1e-3):
        out.append("dG %.12g, -RT ln10 log K %.12g" % (G.val, -R * T * LN10 * logK.val))
    if not close(G.val, H.val - T * S.val, 1e-9, 1e-3):
        out.append("dG %.12g, dH - T dS %.12g" % (G.val, H.val - T * S.val))
    if not close(G.ddt, -S.val, 1e-6, 1.0):
        out.append("d(dG)/dT %.12g, -dS %.12g" % (G.ddt, -S.val))
    return out


@pytest.mark.parametrize("path", DATABASES, ids=lambda p: p.split("/")[-1])
def test_every_record_is_consistent(path):
    db = tf.Database(path)
    engine = tf.ThermoEngine(db)
    problems = []
    for symbol in db.mapSubstances():
        record = db.getSubstance(symbol).record()
        for T, P in POINTS:
            problems += ["substance %s at %s K %s Pa: %s" % (symbol, T, P, m)
                         for m in substance_problems(engine, symbol, T, P, 1e-9 if from_reaction(record) else 0.0)]
    for symbol in db.mapReactions():
        for T, P in POINTS:
            problems += ["reaction %s at %s K %s Pa: %s" % (symbol, T, P, m) for m in reaction_problems(engine, symbol, T, P)]
    assert not problems, "\n".join(problems[:40]) + "\n%d problems" % len(problems)


FLUID_POINTS = [(298.15, 1e5), (373.15, 5e7), (573.15, 5e7), (680.0, 1e8), (773.15, 1e8), (1073.15, 8e8)]
FIXTURES = ["pytests/data/legacy-synthetic/s%s.json" % n for n in
            ("10-fluid_churakov_gottschalk", "11-fluid_soave_redlich_kwong", "12-fluid_sterner_pitzer", "13-fluid_peng_robinson78")]
GEMS = {m: "gems" for m in ("PRSV86", "CorkHP91", "SoaveRedlichKwong72", "PengRobinson78", "SternerPitzer94", "ChurakovGottschalk03",
                            "LandauHP98", "MurnaghanHP98")}


def gas_records(path):
    db = tf.Database(path)
    return db, [s for s in db.mapSubstances() if {m.method for m in db.getSubstance(s).record().methods} & FLUIDS]


@pytest.mark.parametrize("path", ["Resources/databases/aq17-thermofun.json", "Resources/databases/mines16-thermofun.json"] + FIXTURES,
                         ids=lambda p: p.split("/")[-1])
def test_every_fluid_is_consistent_below_and_above_its_critical_point(path):
    """PRSV86 below Tc (H was the form for a constant k, d2a had a sign wrong), CorkHP91 of H2O (da of the supercritical
    branch, the virial term above p0, the parts at Psat of the liquid branch), CGF (S and H at the density), and the
    residual Cp of every fluid. The residual Cp of ChurakovGottschalk03 is not made (see the xfail below)."""
    db, symbols = gas_records(path)
    engine = tf.ThermoEngine(db)
    problems = []
    for symbol in symbols:
        cgf = "ChurakovGottschalk03" in {m.method for m in db.getSubstance(symbol).record().methods}
        for T, P in FLUID_POINTS:
            for m in substance_problems(engine, symbol, T, P):
                # CGF: no residual Cp, and below about 1 bar an ideal-gas volume (both are xfail tests below)
                if not (cgf and (m.startswith("dH/dT") or (m.startswith("dG/dP") and P <= 1e5))):
                    problems.append("%s at %s K %s Pa: %s" % (symbol, T, P, m))
    assert symbols and not problems, "\n".join(problems[:40])


def aq17(T, P, symbol, implementation=None):
    engine = tf.ThermoEngine("Resources/databases/aq17-thermofun.json")
    if implementation:
        prefs = engine.preferences
        prefs.method_implementation = {m: implementation for m in GEMS}
        engine.setPreferences(prefs)
    return engine.thermoPropertiesSubstance(T, P, symbol)


@pytest.mark.parametrize("implementation", [None, "consistent"])
def test_a_hp98_mineral_is_continuous_at_the_reference_point(implementation):
    """G(Tr, Pr) = G0, and G and V do not jump there (MurnaghanHP98 integrated V from P = 0 except at Tr, Pr)."""
    G0 = tf.Database("Resources/databases/aq17-thermofun.json").getSubstance("Albite").record().standard_state_properties[tf.PropertyName.G].value
    at, near = aq17(TR, PR, "Albite", implementation), aq17(TR + 1e-6, PR, "Albite", implementation)
    assert at.gibbs_energy.val == pytest.approx(G0, abs=1e-6)
    assert near.gibbs_energy.val == pytest.approx(at.gibbs_energy.val, abs=1e-3)
    assert near.volume.val == pytest.approx(at.volume.val, rel=1e-8)


def test_the_implementation_gems_keeps_the_formulas_of_gems():
    """The values of GEMS stay reproducible: the jump of Albite at the reference point, no residual Cp of a fluid, and
    S of CORK water that is not -dG/dT."""
    at, near = aq17(TR, PR, "Albite", "gems"), aq17(TR + 1e-6, PR, "Albite", "gems")
    assert near.gibbs_energy.val - at.gibbs_energy.val == pytest.approx(10.0, rel=0.01)   # about V x 1 bar
    ch4 = aq17(373.15, 5e7, "CH4", "gems")
    assert ch4.enthalpy.ddt - ch4.heat_capacity_cp.val == pytest.approx(13.4, rel=0.01)
    water = aq17(373.15, 5e7, "H2O", "gems")
    assert water.gibbs_energy.ddt + water.entropy.val == pytest.approx(53.1, rel=0.01)


@pytest.mark.parametrize("implementation, step", [(None, -3.2), ("gems", 108.9)])
def test_cork_water_is_continuous_at_695_K(implementation, step):
    """Between 673 and 695 K, a(T) of H2O is a0 + a4 (T - 673) + ... (Holland and Powell 1991, Eq. 6; Perple_X crkh2o).
    GEMS had (673 - T), so G jumped by 112 J/mol at 695 K and 1 kbar (the step here is that, with -S dT = -3.2 J/mol)."""
    below, above = aq17(694.99, 1e8, "H2O", implementation), aq17(695.01, 1e8, "H2O", implementation)
    assert above.gibbs_energy.val - below.gibbs_energy.val == pytest.approx(step, abs=0.5)


@pytest.mark.xfail(strict=True, reason="ChurakovGottschalk03 has no residual Cp: it needs the second derivatives of F")
def test_known_the_residual_cp_of_churakov_gottschalk():
    p = tf.ThermoEngine(FIXTURES[0]).thermoPropertiesSubstance(373.15, 5e7, "CO-cgf")
    assert p.enthalpy.ddt == pytest.approx(p.heat_capacity_cp.val, rel=1e-6)


@pytest.mark.xfail(strict=True, reason="ChurakovGottschalk03 below about 1 bar: V is RT/P (ROTOTALMIX), G is not that of an ideal gas")
def test_known_the_volume_of_churakov_gottschalk_at_low_pressure():
    p = tf.ThermoEngine(FIXTURES[0]).thermoPropertiesSubstance(298.15, 1e5, "CO-cgf")   # 4e-4 relative
    assert p.gibbs_energy.ddp == pytest.approx(p.volume.val, rel=1e-6)
