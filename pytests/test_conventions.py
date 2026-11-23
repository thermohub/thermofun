# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The conventions of 14: each value set is converted to the canonical convention after its method, and at the end to the one asked for.
Expected values are made by hand here, from a table of the element entropies in the test."""

import json
import math

import pytest

import thermofun as tf

R = 8.31451
Tr, Pr = 298.15, 1e5
LN_F = math.log(1.01325)
LN_X = math.log(1000.0 / 18.01528)
HEAD = {"thermohub": "2.0.0-draft.1"}
ELEMENTS = {"Ca": 41.59, "C": 5.74, "O": 102.58, "Zz": -65.34, "H": 65.34, "Na": 51.3, "Cl": 111.54}
ELEMENT_RECORDS = {k: {"standard_entropy": v, **({"class": "charge"} if k == "Zz" else {})} for k, v in ELEMENTS.items()}
SIGMA_CACO3 = 41.59 + 5.74 + 3 * 102.58
SIGMA_CO2 = 5.74 + 2 * 102.58
SIGMA_NACL = 51.3 + 111.54
BASE = {"G": -1.2e6, "H": -1.3e6, "S": 90.0, "Cp": 80.0, "V": 3e-5}
WATER = json.loads(tf.Database("pytests/test-aq17-gem-lma-thermofun.json").toJson(True))["substances"]["H2O@"]
LN_10 = math.log(10)
VOLUME = [{"method": "ConstantCp"}, {"method": "ConstantVolume"}]   # U and A need a volume


def substance(values=None, formula="CaCO3", cls="component", state="crystal", methods=None, **more):
    record = {"formula": formula, "class": cls, "aggregate_state": state, "standard_state_properties": dict(values or BASE),
              "methods": methods or [{"method": "ConstantCp"}]}
    record.update(more)
    return record


def engine(substances, reactions=None, dataset=None, memoize=False, elements=None):
    doc = dict(HEAD, elements=elements if elements is not None else ELEMENT_RECORDS, substances=substances)
    if reactions:
        doc["reactions"] = reactions
    if dataset:
        doc["thermodataset"] = dataset
    e = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    if not memoize:
        p = tf.EnginePreferences()
        p.enable_memoize = False
        e.setPreferences(p)
    return e


def output(e, pressure=None, concentration_scale=None, elements=None, water=None):
    """The conventions asked for the results: the groups standard_state and energy_reference (decision 0031)."""
    c = tf.EngineConventions()
    if pressure: c.standard_state.pressure = pressure
    if concentration_scale: c.standard_state.concentration_scale = concentration_scale
    if elements: c.energy_reference.elements = elements
    if water: c.energy_reference.water = water
    e.setConventions(c)
    return e


def props(e, symbol="X", T=400.0, P=Pr):
    p = e.thermoPropertiesSubstance(T, P, symbol)
    return {"G": p.gibbs_energy.val, "H": p.enthalpy.val, "S": p.entropy.val, "Cp": p.heat_capacity_cp.val, "V": p.volume.val,
            "U": p.internal_energy.val, "A": p.helmholtz_energy.val, "dGdT": p.gibbs_energy.ddt, "dGdP": p.gibbs_energy.ddp}


def same(got, want, rel=1e-12, abs_=1e-9):
    for key in want:
        assert got[key] == pytest.approx(want[key], rel=rel, abs=abs_), key


def written_in(reference, sigma=SIGMA_CACO3):
    """BASE, in benson_helgeson, written by hand in another energy reference."""
    values = dict(BASE)
    if reference in ("berman_brown", "ser", "assigned_enthalpy", "dft_formation_0K"):
        values["G"] -= Tr * sigma
    elif reference == "gibbs_helmholtz_formation":
        values["S"] -= sigma
    return values


# --- input: the same species in every convention ------------------------------------------------------------------

@pytest.mark.parametrize("reference", ["benson_helgeson", "berman_brown", "ser", "assigned_enthalpy", "gibbs_helmholtz_formation", "dft_formation_0K"])
@pytest.mark.parametrize("T, P", [(Tr, Pr), (400.0, Pr), (1000.0, 1e8)])
def test_a_mineral_in_each_energy_reference_gives_the_same_result(reference, T, P):
    want = props(engine({"X": substance()}), T=T, P=P)
    got = props(engine({"X": substance(written_in(reference), energy_reference={"elements": reference})}), T=T, P=P)
    same(got, want)


@pytest.mark.parametrize("pressure, ln", [({"value": 1000, "unit": "bar"}, math.log(1000.0)), (1e8, math.log(1000.0)), (5e4, math.log(0.5))])
def test_any_standard_pressure_of_a_gas(pressure, ln):
    # GB4 4.11.1 (v): p° can be chosen freely; a gas tabulated at p° gives the same result in 1 bar (decision 0031, row 7)
    gas = dict(formula="CO2", state="gas", cls="gas_fluid")
    want = props(engine({"X": substance(**gas)}), T=400.0)
    there = dict(BASE, G=BASE["G"] + R * Tr * ln, S=BASE["S"] - R * ln)
    same(props(engine({"X": substance(there, standard_state={"pressure": pressure}, **gas)}), T=400.0), want, rel=1e-11)
    # and asked for: from 1 bar to p°, and back
    got = props(output(engine({"X": substance(**gas)}), pressure=pressure if isinstance(pressure, float) else 1e8), T=400.0)
    assert got["G"] == pytest.approx(want["G"] + R * 400.0 * (ln if isinstance(pressure, float) else math.log(1000.0)), rel=1e-12)


def test_the_reference_pressure_is_the_standard_pressure():
    # reference_conditions keeps only T; a P equal to p° is accepted, another P is an error (CNV-010)
    same_p = substance(reference_conditions={"T": 298.15, "P": 101325}, standard_state={"pressure": "1atm"})
    assert tf.Database(json.dumps(dict(HEAD, elements=ELEMENT_RECORDS, substances={"X": same_p}))).getSubstance("X").record().reference_conditions.P == 101325.0
    other = substance(reference_conditions={"T": 298.15, "P": 2e5})
    with pytest.raises(RuntimeError, match="CNV-010"):
        tf.Database(json.dumps(dict(HEAD, elements=ELEMENT_RECORDS, substances={"X": other})))
    # the reference pressure of a record follows its standard pressure, written once, in standard_state
    db = tf.Database(json.dumps(dict(HEAD, elements=ELEMENT_RECORDS, substances={"X": substance(standard_state={"pressure": 1e8})})))
    assert db.getSubstance("X").record().reference_conditions.P == 1e8
    written = json.loads(db.toJson(False))["substances"]["X"]
    assert written["standard_state"]["pressure"] == 1e8 and "P" not in written.get("reference_conditions", {})


@pytest.mark.parametrize("T", [Tr, 400.0, 1000.0])
def test_a_gas_in_one_atmosphere_gives_the_result_in_one_bar(T):
    gas = dict(formula="CO2", state="gas", cls="gas_fluid")
    want = props(engine({"X": substance(**gas)}), T=T)
    atm = dict(BASE, G=BASE["G"] + R * Tr * LN_F, S=BASE["S"] - R * LN_F)
    same(props(engine({"X": substance(atm, standard_state={"pressure": "1atm"}, **gas)}), T=T), want)


def test_one_atmosphere_is_only_for_gases():
    same(props(engine({"X": substance(standard_state={"pressure": "1atm"})})), props(engine({"X": substance()})))


def test_a_solute_in_mole_fraction_gives_the_result_in_molality():
    solute = dict(formula="NaCl", state="aqueous", cls="aqueous_solute")
    want = props(engine({"X": substance(**solute)}))
    x = dict(BASE, G=BASE["G"] + R * Tr * LN_X, S=BASE["S"] - R * LN_X)
    same(props(engine({"X": substance(x, standard_state={"concentration_scale": "mole_fraction"}, **solute)})), want)


def test_the_record_goes_before_the_dataset():
    values = written_in("berman_brown")
    dataset = {"symbol": "d", "energy_reference": {"elements": "berman_brown"}}
    want = props(engine({"X": substance()}))
    same(props(engine({"X": substance(values)}, dataset=dataset)), want)
    same(props(engine({"X": substance(energy_reference={"elements": "benson_helgeson"})}, dataset=dataset)), want)


def test_legacy_defaults_are_the_identity():
    # 1bar, molality, helgeson_kirkham_1974, benson_helgeson written out give the same bits as nothing written
    canonical = {"standard_state": {"pressure": "1bar", "concentration_scale": "molality"},
                 "energy_reference": {"elements": "benson_helgeson", "water": "helgeson_kirkham_1974"}}
    assert props(engine({"X": substance(**canonical)})) == props(engine({"X": substance()}))


def test_the_reference_properties_of_a_record_without_methods_are_converted():
    values = written_in("berman_brown")
    record = substance(values, energy_reference={"elements": "berman_brown"})
    del record["methods"]
    got = engine({"X": record}).thermoPropertiesSubstance(400.0, Pr, "X").gibbs_energy.val
    assert got == pytest.approx(BASE["G"], rel=1e-12)   # the value at the reference temperature, in benson_helgeson


# --- a method with its own convention: conversion after each method -----------------------------------------------

NASA = [{"method": "NASA9", "segments": [{"t_min": 200.0, "t_max": 1000.0, "parameters":
        {"a1": 0.0, "a2": 0.0, "a3": 3.5, "a4": 0.0, "a5": 0.0, "a6": 0.0, "a7": 0.0, "b1": -47000.0, "b2": 5.0}}]}]


def nasa_raw(T):
    H = R * (3.5 * T - 47000.0)
    S = R * (3.5 * math.log(T) + 5.0)
    return H - T * S, H, S


def test_nasa_is_converted_from_assigned_enthalpy():
    T = 500.0
    G, H, S = nasa_raw(T)
    got = props(engine({"X": substance(formula="CO2", state="gas", cls="gas_fluid", methods=NASA)}), T=T)
    assert got["G"] == pytest.approx(G + Tr * SIGMA_CO2, rel=1e-12)
    assert got["H"] == pytest.approx(H, rel=1e-12)
    assert got["S"] == pytest.approx(S, rel=1e-12)


def test_the_method_goes_before_the_record():
    plain = props(engine({"X": substance(formula="CO2", state="gas", cls="gas_fluid", methods=NASA)}))
    declared = props(engine({"X": substance(formula="CO2", state="gas", cls="gas_fluid", methods=NASA, energy_reference={"elements": "gibbs_helmholtz_formation"})}))
    assert declared == plain


def test_mixed_record_a_base_in_its_convention_and_a_correction_in_the_default():
    # the correction is not converted, and the base is: this fails if the conversion runs once at the end
    methods = NASA + [{"method": "ConstantVolume"}]
    e = engine({"X": substance(dict(BASE, V=2e-5), formula="CO2", state="crystal", methods=methods)})
    T, P = 500.0, 1e8
    G, _, _ = nasa_raw(T)
    got = props(e, T=T, P=P)
    assert got["G"] == pytest.approx(G + Tr * SIGMA_CO2 + 2e-5 * (P - Pr), rel=1e-12)
    assert got["V"] == pytest.approx(2e-5, rel=1e-12)   # m3/mol


def test_mixed_record_of_a_declared_base_and_a_correction():
    methods = [{"method": "ConstantCp"}, {"method": "ConstantVolume"}]
    want = props(engine({"X": substance(methods=methods)}), P=1e8)
    got = props(engine({"X": substance(written_in("gibbs_helmholtz_formation"), methods=methods, energy_reference={"elements": "gibbs_helmholtz_formation"})}), P=1e8)
    same(got, want)


# --- output ----------------------------------------------------------------------------------------------------------

@pytest.mark.parametrize("reference", ["berman_brown", "ser", "assigned_enthalpy", "dft_formation_0K"])
def test_output_of_a_reference_without_a_term_in_t(reference):
    plain, T = props(engine({"X": substance()})), 400.0
    got = props(output(engine({"X": substance()}), elements=reference))
    assert got["G"] == pytest.approx(plain["G"] - Tr * SIGMA_CACO3, rel=1e-12)
    for key in ("H", "S", "Cp", "V", "U", "A", "dGdT", "dGdP"):
        assert got[key] == plain[key], key


def test_output_gibbs_helmholtz_formation():
    record, T = substance(methods=VOLUME), 400.0
    plain = props(engine({"X": record}))
    got = props(output(engine({"X": record}), elements="gibbs_helmholtz_formation"))
    assert got["G"] == pytest.approx(plain["G"] + (T - Tr) * SIGMA_CACO3, rel=1e-12)
    assert got["S"] == pytest.approx(plain["S"] - SIGMA_CACO3, rel=1e-12)
    assert got["H"] == plain["H"]
    assert got["dGdT"] == pytest.approx(-got["S"], rel=1e-12)
    assert got["A"] == pytest.approx(got["U"] - T * got["S"], rel=1e-12)


def test_output_one_atmosphere():
    gas = substance(formula="CO2", state="gas", cls="gas_fluid", methods=VOLUME)
    plain, T = props(engine({"X": gas})), 400.0
    got = props(output(engine({"X": gas}), pressure="1atm"))
    assert got["G"] == pytest.approx(plain["G"] + R * T * LN_F, rel=1e-12)
    assert got["S"] == pytest.approx(plain["S"] - R * LN_F, rel=1e-12)
    assert got["dGdT"] == pytest.approx(-got["S"], rel=1e-12)
    assert got["A"] == pytest.approx(got["U"] - T * got["S"], rel=1e-12)
    # a mineral does not change
    assert props(output(engine({"X": substance()}), pressure="1atm")) == props(engine({"X": substance()}))


def test_output_mole_fraction():
    solute = substance(formula="NaCl", state="aqueous", cls="aqueous_solute")
    plain, T = props(engine({"X": solute})), 400.0
    got = props(output(engine({"X": solute}), concentration_scale="mole_fraction"))
    assert got["G"] == pytest.approx(plain["G"] + R * T * LN_X, rel=1e-12)


def test_a_conversion_and_its_inverse_are_the_identity():
    want = props(engine({"X": substance()}))
    for reference in ("berman_brown", "gibbs_helmholtz_formation"):
        out = output(engine({"X": substance()}), elements=reference)
        # the values at the reference conditions, in that convention, give the same species back
        v = out.thermoPropertiesSubstance(Tr, Pr, "X")
        values = {"G": v.gibbs_energy.val, "H": v.enthalpy.val, "S": v.entropy.val, "Cp": v.heat_capacity_cp.val, "V": BASE["V"]}
        same(props(engine({"X": substance(values, energy_reference={"elements": reference})})), want)


def test_a_change_of_the_output_after_a_calculation_changes_the_next_result():
    e = engine({"X": substance()}, memoize=True)
    first = props(e)
    e.conventions.energy_reference.elements = "berman_brown"
    second = props(e)
    assert second["G"] == pytest.approx(first["G"] - Tr * SIGMA_CACO3, rel=1e-12)
    output(e, elements="benson_helgeson")
    assert props(e) == first


def test_the_old_words_still_work():
    a = engine({"X": substance()})
    a.conventions.apparent_properties = "BERMAN_BROWN"
    b = output(engine({"X": substance()}), elements="berman_brown")
    assert props(a) == props(b)


def test_the_route_says_what_was_converted():
    e = output(engine({"X": substance(formula="CO2", state="gas", cls="gas_fluid", methods=NASA)}), pressure="1atm")
    result = e.thermoPropertiesSubstance(500.0, Pr, "X")
    conversions = [(c.what, c.detail) for c in result.info.route.conversions]
    assert ("convention", "NASA9 to canonical") in conversions and ("convention", "1atm") in conversions
    assert result.info.route.total()["G"] == pytest.approx(result.gibbs_energy.val, rel=1e-12)


# --- the hydrogen ion -----------------------------------------------------------------------------------------------

def hydrogen(**more):
    record = {"formula": "H+", "class": "aqueous_solute", "aggregate_state": "aqueous", "charge": 1}
    record.update(more)
    return record


def test_the_hydrogen_ion_stays_zero_where_the_table_makes_its_entropy_zero():
    got = output(engine({"H+": hydrogen()}), elements="berman_brown").thermoPropertiesSubstance(Tr, Pr, "H+")
    assert got.gibbs_energy.val == 0.0


def test_the_output_of_the_hydrogen_ion_with_another_charge_entropy():
    elements = dict(ELEMENT_RECORDS, Zz={"class": "charge", "standard_entropy": -60.0})
    e = engine({"H+": hydrogen()}, elements=elements)
    assert e.thermoPropertiesSubstance(Tr, Pr, "H+").gibbs_energy.val == 0.0
    output(e, elements="berman_brown")
    assert e.thermoPropertiesSubstance(Tr, Pr, "H+").gibbs_energy.val == pytest.approx(-Tr * (65.34 - 60.0), rel=1e-12)


def test_the_hydrogen_ion_in_mole_fraction_output():
    e = output(engine({"H+": hydrogen()}), concentration_scale="mole_fraction")
    assert e.thermoPropertiesSubstance(400.0, Pr, "H+").gibbs_energy.val == pytest.approx(R * 400.0 * LN_X, rel=1e-12)


def test_the_hydrogen_ion_in_berman_brown_with_values_is_converted():
    values = {"G": -1000.0, "H": -1000.0, "S": 1.0, "Cp": 0.0, "V": 0.0}
    elements = dict(ELEMENT_RECORDS, Zz={"class": "charge", "standard_entropy": -60.0})
    e = engine({"H+": hydrogen(standard_state_properties=values, methods=[{"method": "ConstantCp"}], energy_reference={"elements": "berman_brown", "ions": "none"})}, elements=elements)
    assert e.thermoPropertiesSubstance(Tr, Pr, "H+").gibbs_energy.val == pytest.approx(-1000.0 + Tr * 5.34, rel=1e-12)


def test_hkf_of_the_hydrogen_ion_in_another_convention_keeps_the_rule_of_hkf():
    # HKF has no Born term for H+, whatever the convention; the values follow by the same shift
    aq = tf.Database("pytests/test-aq17-gem-lma-thermofun.json")
    doc = json.loads(aq.toJson(True))
    hkf = next(m for m in doc["substances"]["Ca+2"]["methods"] if m["method"] == "HKF88")
    values = {"G": -1000.0, "H": -1000.0, "S": 1.0, "Cp": 0.0, "V": 0.0}
    elements = doc["elements"]
    sigma = 2 * 0 + elements["H"]["standard_entropy"] + elements["Zz"]["standard_entropy"]
    water = next(k for k, v in doc["substances"].items() if v.get("class") == "water_solvent")
    subs = {water: doc["substances"][water],
            "H+": hydrogen(standard_state_properties=values, methods=[hkf], energy_reference={"ions": "none"}),
            "HB": hydrogen(standard_state_properties=values, methods=[hkf], energy_reference={"elements": "berman_brown", "ions": "none"})}
    e = tf.ThermoEngine(tf.Database(json.dumps(dict(HEAD, elements=elements, substances=subs))))
    a = e.thermoPropertiesSubstance(373.15, Pr, "H+").gibbs_energy.val
    b = e.thermoPropertiesSubstance(373.15, Pr, "HB").gibbs_energy.val
    assert b - a == pytest.approx(Tr * sigma, abs=1e-6)


# --- molarity --------------------------------------------------------------------------------------------------------

def molarity_engine(**more):
    subs = {"H2O@": WATER, "X": substance(formula="NaCl", state="aqueous", cls="aqueous_solute", **more)}
    return engine(subs)


def density(e, T, P):
    return e.propertiesSolvent(T, P, "H2O@").density.val


@pytest.mark.parametrize("T, P", [(298.15, 1e5), (473.15, 2e6)])
def test_a_solute_in_molarity_converts_to_molality_with_the_density_of_water(T, P):
    plain = molarity_engine()
    e = molarity_engine(standard_state={"concentration_scale": "molarity"})
    rho = density(e, T, P)
    assert 800.0 < rho < 1000.0
    a, b = props(plain, T=T, P=P), props(e, T=T, P=P)
    assert b["G"] - a["G"] == pytest.approx(R * T * math.log(rho / 1000.0), rel=1e-10)

    # S, H, V: the derivatives of that term, here against finite differences of it
    term = lambda t, p: R * t * math.log(density(e, t, p) / 1000.0)
    dt, dp = 1e-2, 1e3
    dSdiff = -(term(T + dt, P) - term(T - dt, P)) / (2 * dt)
    dVdiff = (term(T, P + dp) - term(T, P - dp)) / (2 * dp)
    assert b["S"] - a["S"] == pytest.approx(dSdiff, rel=1e-5, abs=1e-6)
    assert b["H"] - a["H"] == pytest.approx(b["G"] - a["G"] + T * (b["S"] - a["S"]), rel=1e-10)
    assert b["V"] - a["V"] == pytest.approx(dVdiff, rel=1e-4, abs=1e-12)
    h = lambda t: R * t * math.log(density(e, t, P) / 1000.0) + t * -(term(t + dt, P) - term(t - dt, P)) / (2 * dt)
    assert b["Cp"] - a["Cp"] == pytest.approx((h(T + 0.5) - h(T - 0.5)) / 1.0, rel=2e-3, abs=2e-3)


def test_output_in_molarity_is_the_inverse():
    subs = lambda **more: {"H2O@": WATER, "X": substance(formula="NaCl", state="aqueous", cls="aqueous_solute", **more)}
    declared = engine(subs(standard_state={"concentration_scale": "molarity"}))
    output(declared, concentration_scale="molarity")
    same(props(declared, T=350.0), {k: v for k, v in props(engine(subs()), T=350.0).items() if k in ("G", "H", "S", "Cp", "V")}, rel=1e-10, abs_=1e-7)


def test_molarity_without_a_solvent_is_an_error():
    e = engine({"X": substance(formula="NaCl", state="aqueous", cls="aqueous_solute", standard_state={"concentration_scale": "molarity"})})
    with pytest.raises(RuntimeError, match="CNV-002"):
        props(e)


# --- reactions -------------------------------------------------------------------------------------------------------

def reaction_db(reaction=None):
    doc = json.loads(tf.Database("pytests/test-aq17-gem-lma-thermofun.json").toJson(True))
    subs = doc["substances"]
    water = next(k for k, v in subs.items() if v.get("class") == "water_solvent")
    uq = {"formula": "CaCO3", "class": "aqueous_solute", "aggregate_state": "aqueous",
          "standard_state_properties": {"G": -1.1e6, "H": -1.2e6, "S": 10.0, "Cp": 50.0, "V": 1e-5},
          "methods": [{"method": "CpExtendedUNIQUAC", "parameters": {"a": 80.0, "b": 0.05, "c": -10000.0, "alpha": 0.0, "beta": 0.0, "Theta": 200.0}}]}
    gas = {"formula": "CO2", "class": "gas_fluid", "aggregate_state": "gas", "standard_state_properties": {}, "methods": NASA}
    keep = {k: subs[k] for k in (water, "Ca+2", "H+")}
    keep.update(UQ=uq, NG=gas)
    equation = reaction or {"equation": "UQ + 2 H+ = Ca+2 + NG + %s" % water}
    return json.dumps(dict(HEAD, elements=doc["elements"], substances=keep, reactions={"R": equation})), doc["elements"], water


def test_a_reaction_of_species_in_different_conventions_is_converted_by_hand():
    text, elements, water = reaction_db()
    S = {k: v["standard_entropy"] for k, v in elements.items() if "standard_entropy" in v}
    sigma_uq = S["Ca"] + S["C"] + 3 * S["O"]
    sigma_ng = S["C"] + 2 * S["O"]
    T, P = 400.0, 1e5
    e = tf.ThermoEngine(tf.Database(text))
    native = tf.ThermoEngine(tf.Database(text))
    G = lambda en, s: en.thermoPropertiesSubstance(T, P, s).gibbs_energy.val

    c = tf.EngineConventions()
    c.energy_reference.elements = "gibbs_helmholtz_formation"
    native.setConventions(c)
    uq_raw = G(native, "UQ")
    uq = uq_raw + (Tr - T) * sigma_uq                        # by hand, from its native convention
    ng = nasa_raw(T)[0] + Tr * sigma_ng                      # by hand, from the polynomial
    dG = G(e, "Ca+2") + ng + G(e, water) - uq - 2 * G(e, "H+")

    r = e.thermoPropertiesReaction(T, P, "R")
    assert r.reaction_gibbs_energy.val == pytest.approx(dG, rel=1e-10)
    assert r.log_equilibrium_constant.val == pytest.approx(-dG / (R * T) * 0.4342944819032518, rel=1e-10)


@pytest.mark.parametrize("field, word", [("elements", "berman_brown"), ("elements", "gibbs_helmholtz_formation"), ("water", "steam_tables")])
def test_log_k_does_not_depend_on_the_energy_output(field, word):
    text, _, _ = reaction_db()
    plain = tf.ThermoEngine(tf.Database(text)).thermoPropertiesReaction(400.0, 1e5, "R").log_equilibrium_constant.val
    other = output(tf.ThermoEngine(tf.Database(text)), **{field: word}).thermoPropertiesReaction(400.0, 1e5, "R").log_equilibrium_constant.val
    assert other == pytest.approx(plain, rel=1e-12)


def test_log_k_in_one_atmosphere():
    text, _, _ = reaction_db()
    plain = tf.ThermoEngine(tf.Database(text)).thermoPropertiesReaction(400.0, 1e5, "R")
    atm = output(tf.ThermoEngine(tf.Database(text)), pressure="1atm").thermoPropertiesReaction(400.0, 1e5, "R")
    # one gas with the coefficient 1 on the right
    assert atm.reaction_gibbs_energy.val - plain.reaction_gibbs_energy.val == pytest.approx(R * 400.0 * LN_F, rel=1e-9)
    assert atm.log_equilibrium_constant.val - plain.log_equilibrium_constant.val == pytest.approx(-LN_F / LN_10, rel=1e-9)


def test_the_declared_standard_pressure_of_a_reaction_is_converted_by_stoichiometry():
    text, _, water = reaction_db()
    doc = json.loads(text)
    base = next(iter(json.loads(tf.Database("pytests/test-aq17-gem-lma-thermofun.json").toJson(True))["reactions"].values()), None)
    assert base is not None
    methods = base.get("methods")
    props_ = base.get("standard_state_properties") or base.get("reaction_properties")
    key = "standard_state_properties" if "standard_state_properties" in base else "reaction_properties"
    record = {"equation": "UQ + 2 H+ = Ca+2 + NG + %s" % water, "methods": methods, key: props_}
    doc["reactions"] = {"R": record, "A": dict(record, standard_state={"pressure": "1atm"})}
    e = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    plain = e.thermoPropertiesReaction(400.0, 1e5, "R").log_equilibrium_constant.val
    atm = e.thermoPropertiesReaction(400.0, 1e5, "A").log_equilibrium_constant.val
    # G(1 bar) = G(1 atm) - R T ln(1.01325), so log K rises
    assert atm - plain == pytest.approx(LN_F / LN_10, rel=1e-9)


MINES = "pytests/mines16-thermofun.json"


@pytest.mark.parametrize("symbol", ["Ti(OH)3+", "Ti(OH)4@", "Ti(OH)5-"])
def test_a_substance_from_a_reaction_with_water_is_converted_once(symbol):
    db = tf.Database(MINES)
    assert db.getSubstance(symbol).reactionSymbol() == symbol
    plain = tf.ThermoEngine(MINES).thermoPropertiesSubstance(400.0, 1e5, symbol).gibbs_energy.val
    sigma = db.elementalEntropyFormula(db.getSubstance(symbol).formula())
    bb = output(tf.ThermoEngine(MINES), elements="berman_brown")
    assert bb.thermoPropertiesSubstance(400.0, 1e5, symbol).gibbs_energy.val == pytest.approx(plain - Tr * sigma, rel=1e-12)
    steam = output(tf.ThermoEngine(MINES), water="steam_tables")
    assert steam.thermoPropertiesSubstance(400.0, 1e5, symbol).gibbs_energy.val == plain


@pytest.mark.parametrize("symbol", ["CH4@", "CO2@", "H2@", "O2@"])
def test_the_reference_of_water_does_not_reach_a_method_that_reads_water(symbol):
    aq = "pytests/test-aq17-gem-lma-thermofun.json"
    plain = tf.ThermoEngine(aq).thermoPropertiesSubstance(400.0, 1e5, symbol).gibbs_energy.val
    steam = output(tf.ThermoEngine(aq), water="steam_tables")
    assert steam.thermoPropertiesSubstance(400.0, 1e5, symbol).gibbs_energy.val == plain
    # and water itself changes
    assert steam.thermoPropertiesSubstance(400.0, 1e5, "H2O@").gibbs_energy.val != tf.ThermoEngine(aq).thermoPropertiesSubstance(400.0, 1e5, "H2O@").gibbs_energy.val


# --- diagnostics -----------------------------------------------------------------------------------------------------

def codes(db):
    return {d.code for d in db.validate().all()}


def database(substances, reactions=None, dataset=None, elements=ELEMENT_RECORDS):
    doc = dict(HEAD, elements=elements, substances=substances)
    if reactions:
        doc["reactions"] = reactions
    if dataset:
        doc["thermodataset"] = dataset
    return tf.Database(json.dumps(doc))


def test_cnv_001_no_source_has_the_entropy_of_an_element():
    elements = dict(ELEMENT_RECORDS, Ca={"standard_entropy": 0.0})
    record = substance(energy_reference={"elements": "berman_brown"})
    assert "CNV-001" in codes(database({"X": record}, elements=elements))
    e = tf.ThermoEngine(database({"X": record}, elements=elements))
    with pytest.raises(RuntimeError, match="CNV-001.*Ca"):
        props(e)


def test_cnv_008_dft_energies_are_estimates():
    """dft_formation_0K is converted as Berman-Brown, with the energy at 0 K as the enthalpy at 298.15 K: a warning at
    reading, for a record and for a dataset (decision 5 of the review of v2)."""
    def loaded(db):
        return {d.code for d in db.loadDiagnostics().all()}
    assert "CNV-008" in loaded(database({"X": substance(energy_reference={"elements": "dft_formation_0K"})}))
    assert "CNV-008" in loaded(database({"X": substance()}, dataset={"symbol": "d", "energy_reference": {"elements": "dft_formation_0K"}}))
    assert "CNV-008" not in loaded(database({"X": substance(energy_reference={"elements": "berman_brown"})}))


def test_cnv_002_molarity_without_a_solvent():
    record = substance(formula="NaCl", state="aqueous", cls="aqueous_solute", standard_state={"concentration_scale": "molarity"})
    assert "CNV-002" in codes(database({"X": record}))
    assert "CNV-002" not in codes(database({"X": record, "H2O@": WATER}))


def test_cnv_003_a_method_in_another_convention_than_the_dataset():
    record = substance(formula="CO2", state="gas", cls="gas_fluid", methods=NASA)
    assert "CNV-003" in codes(database({"X": record}))
    assert "CNV-003" not in codes(database({"X": dict(record, energy_reference={"elements": "assigned_enthalpy"})}))
    assert "CNV-003" not in codes(database({"X": record}, dataset={"symbol": "d", "energy_reference": {"elements": "assigned_enthalpy"}}))


def test_cnv_004_a_reaction_of_species_in_different_references():
    subs = {"X": substance(), "Y": substance(energy_reference={"elements": "berman_brown"}), "Z": substance()}
    assert "CNV-004" in codes(database(subs, reactions={"R": {"equation": "X = Y"}}))
    assert "CNV-004" not in codes(database(subs, reactions={"R": {"equation": "X = Z"}}))


def test_cnv_005_an_unknown_word_is_an_error_with_the_allowed_words():
    with pytest.raises(Exception, match="CNV-005"):
        database({"X": substance(energy_reference={"elements": "nonsense"})})


def test_cnv_006_an_element_from_the_table_of_chemicalfun():
    record = substance(energy_reference={"elements": "berman_brown"})
    assert "CNV-006" in codes(database({"X": record}, elements={}))
    assert "CNV-006" not in codes(database({"X": record}))
    # and the value of that table is used
    got = props(tf.ThermoEngine(database({"X": record}, elements={})))
    assert math.isfinite(got["G"])


def test_water_on_the_steam_tables_has_its_own_entropy_scale():
    # at the triple point S = G = 0, and G = H - T S at all T
    steam = output(tf.ThermoEngine("pytests/test-aq17-gem-lma-thermofun.json"), water="steam_tables")
    symbol = "H2O@"
    at = steam.thermoPropertiesSubstance(273.16, 611.655, symbol)
    assert at.entropy.val == pytest.approx(0.0, abs=0.05)
    assert at.gibbs_energy.val == pytest.approx(0.0, abs=2.0)
    warm = steam.thermoPropertiesSubstance(298.15, 1e5, symbol)
    assert warm.gibbs_energy.val == pytest.approx(warm.enthalpy.val - 298.15 * warm.entropy.val, abs=2.0)


# --- the water anchors (decision 0031, correction of row 8) ---------------------------------------------------------

IAPWS_WATER = dict(WATER, methods=[{"method": "WaterIAPWS95", "implementation": "reaktoro"}, {"method": "JohnsonNorton91", "implementation": "reaktoro"}])


def water_engine(water=WATER, dataset=None, record_water=None):
    record = dict(water)
    if record_water:
        record["energy_reference"] = {"water": record_water}
    return engine({"H2O@": record}, dataset=dataset)


def water_gibbs(e, T=Tr, P=Pr):
    return e.thermoPropertiesSubstance(T, P, "H2O@").gibbs_energy.val


@pytest.mark.parametrize("water,method,triple", [(WATER, "WaterHGK84", "NEA_HGK"), (IAPWS_WATER, "WaterIAPWS95", "NEA_IAPWS")])
def test_codata_1989_takes_the_triple_point_set_of_the_water_method(water, method, triple):
    codata = water_engine(water, dataset={"symbol": "d", "energy_reference": {"water": "codata_1989"}})
    alone = tf.methods.solventMethod(method, implementation="reaktoro").substanceProperties(Tr, Pr, 0, triple).gibbs_energy.val
    assert water_gibbs(codata) == alone
    assert water_gibbs(codata) == pytest.approx(-237140.0, abs=0.5)      # CODATA 1989
    assert water_gibbs(water_engine(water)) == pytest.approx(-237181.7, abs=0.5)   # Helgeson and Kirkham 1974, the default
    assert water_gibbs(water_engine(water, record_water="codata_1989")) == water_gibbs(codata)   # declared on the water record


def test_another_anchor_cannot_be_asked_for():
    e = water_engine()
    with pytest.raises(ValueError, match="does not convert between the two anchors"):
        output(e, water="codata_1989")
    # the anchor of the dataset itself, and the steam tables, can
    output(e, water="helgeson_kirkham_1974")
    assert water_gibbs(e) == water_gibbs(water_engine())
    codata = output(water_engine(dataset={"symbol": "d", "energy_reference": {"water": "codata_1989"}}), water="codata_1989")
    assert water_gibbs(codata) == pytest.approx(-237140.0, abs=0.5)


def test_steam_tables_from_either_anchor_is_the_same_water():
    hk = output(water_engine(), water="steam_tables")
    codata = output(water_engine(dataset={"symbol": "d", "energy_reference": {"water": "codata_1989"}}), water="steam_tables")
    for T, P in ((273.16, 611.655), (298.15, 1e5), (473.15, 50e5)):
        assert water_gibbs(hk, T, P) == pytest.approx(water_gibbs(codata, T, P), abs=1.0)


def test_the_0x_view_of_the_anchor():
    db = tf.Database(json.dumps(dict(HEAD, elements=ELEMENT_RECORDS, substances={"H2O@": IAPWS_WATER},
                                     thermodataset={"symbol": "d", "energy_reference": {"water": "codata_1989"}})))
    assert db.conventions.water_triple_point_reference == "NEA_IAPWS"   # the set of the water method
    db.conventions.water_triple_point_reference = "HKF_HGK_1974"
    assert db.thermoDataSet().convention.energy_reference.water == tf.WaterReference.HelgesonKirkham1974
    with pytest.raises(Exception, match="NEA_HGK or NEA_IAPWS"):
        db.conventions.water_triple_point_reference = "NEA"
        db.thermoDataSet()
    # GEOCHEMICAL of 0.x is the water of the dataset, whichever its anchor
    e = water_engine(dataset={"symbol": "d", "energy_reference": {"water": "codata_1989"}})
    c = e.conventions
    c.water_properties = "GEOCHEMICAL"
    e.conventions = c
    assert c.energy_reference.water is None and water_gibbs(e) == pytest.approx(-237140.0, abs=0.5)


def test_the_old_conventions_block_is_an_unknown_key():
    db = tf.Database(json.dumps(dict(HEAD, elements=ELEMENT_RECORDS, substances={"H2O@": WATER},
                                     thermodataset={"symbol": "d", "conventions": {"water_triple_point_reference": "NEA_HGK"}})))
    assert "V2-KEY-001" in {d.code for d in db.loadDiagnostics().all()}
    assert db.thermoDataSet().convention.energy_reference.water is None


@pytest.mark.parametrize("water,label", [(WATER, "NEA_HGK"), (IAPWS_WATER, "NEA_IAPWS")])
def test_legacy_files_keep_the_triple_point_set(water, label, tmp_path):
    db = tf.Database(json.dumps(dict(HEAD, elements=ELEMENT_RECORDS, substances={"H2O@": water},
                                     thermodataset={"symbol": "d", "energy_reference": {"water": "codata_1989"}})))
    legacy = json.loads(db.toLegacyJson(False))
    assert legacy["conventions"] == {"water_triple_point_reference": label}
    back = tf.Database(json.dumps(legacy))
    assert back.thermoDataSet().convention.energy_reference.water == tf.WaterReference.Codata1989
    assert water_gibbs(tf.ThermoEngine(back)) == water_gibbs(tf.ThermoEngine(db))


# --- ions: the hydrogen ion and the electron (decision 0031, row 6; NEA TDB-5 1.6, 1.7.e) ----------------------------

PSI = "pytests/PsiTDB2020-subset-thermofun.json"


def psi_with_electron(**energy_reference):
    doc = json.loads(tf.Database(PSI).toJson(True))
    if energy_reference:
        doc["substances"]["e-"]["energy_reference"] = energy_reference
    return doc


@pytest.mark.parametrize("T", [298.15, 323.15, 373.15, 423.15, 473.15, 523.15, 573.15])
@pytest.mark.parametrize("P", [1e5, 5e7])
def test_the_electron_is_half_of_hydrogen_gas_at_its_standard_pressure(T, P):
    e = tf.ThermoEngine(PSI)
    electron = e.thermoPropertiesSubstance(T, P, "e-")
    h2 = e.thermoPropertiesSubstance(T, 1e5, "H2(g)")
    assert electron.gibbs_energy.val == 0.5 * h2.gibbs_energy.val
    assert electron.enthalpy.val == 0.5 * h2.enthalpy.val
    assert electron.entropy.val == 0.5 * h2.entropy.val
    assert electron.heat_capacity_cp.val == 0.5 * h2.heat_capacity_cp.val
    assert electron.volume.val == 0.0 and electron.gibbs_energy.ddp == 0.0
    # the reaction of the standard hydrogen electrode H+ + e- = 1/2 H2(g): drG = 0 at every T
    assert e.thermoPropertiesSubstance(T, P, "H+").gibbs_energy.val == 0.0


def test_the_electron_at_25_degC_is_the_codata_value():
    e = tf.ThermoEngine(PSI).thermoPropertiesSubstance(298.15, 1e5, "e-")
    assert e.gibbs_energy.val == pytest.approx(0.0, abs=1e-9)
    assert e.entropy.val == pytest.approx(65.34, abs=0.005)   # 1/2 S(H2, g), CODATA 1989: 130.68


def test_the_electron_of_its_record_with_ions_none():
    doc = psi_with_electron(ions="none")
    e = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    hydrogen_electrode = tf.ThermoEngine(PSI)
    at = lambda engine, T: engine.thermoPropertiesSubstance(T, 1e5, "e-").gibbs_energy.val
    # PSI 2020 gives a constant Cp 14.418: 0.55 J/mol at 100 degC, 13 J/mol at 300 degC from 1/2 H2(g)
    assert at(e, 373.15) - at(hydrogen_electrode, 373.15) == pytest.approx(0.5475, abs=1e-3)
    assert at(e, 573.15) - at(hydrogen_electrode, 573.15) == pytest.approx(13.02, abs=1e-2)


def test_without_hydrogen_gas_the_electron_is_its_record_with_a_note():
    doc = psi_with_electron()
    del doc["substances"]["H2(g)"]
    for name, r in list(doc["reactions"].items()) if "reactions" in doc else []:
        if "H2(g)" in r.get("equation", ""):
            del doc["reactions"][name]
    db = tf.Database(json.dumps(doc))
    assert "CNV-011" in {d.code for d in db.validate().all()}
    p = tf.ThermoEngine(db).thermoPropertiesSubstance(373.15, 1e5, "e-")
    assert any("no H2(g)" in n.text for n in p.info.notes)
    assert p.entropy.val != 0.0


def test_the_results_are_always_on_hydrogen_ion():
    e = tf.ThermoEngine(PSI)
    c = tf.EngineConventions()
    c.energy_reference.ions = "none"
    with pytest.raises(ValueError, match="always on hydrogen_ion"):
        e.setConventions(c)
    c.energy_reference.ions = tf.Ions.HydrogenIon
    e.setConventions(c)


def test_ions_is_read_and_written_and_checked():
    doc = psi_with_electron(ions="none")
    back = json.loads(tf.Database(json.dumps(doc)).toJson(True))
    assert back["substances"]["e-"]["energy_reference"] == {"ions": "none"}
    doc["substances"]["e-"]["energy_reference"] = {"ions": "hydrogen"}
    with pytest.raises(RuntimeError, match="hydrogen_ion"):
        tf.Database(json.dumps(doc))


def test_an_electron_without_hydrogen_gas_and_without_values_is_an_error():
    db = tf.Database("thermohub: 2.0.0-draft.1\nsubstances:\n  e-: {formula: \"-\", class: aqueous_solute, aggregate_state: aqueous}\n")
    assert "CNV-011" in {d.code for d in db.validate().all()}
    with pytest.raises(RuntimeError, match="no H2\\(g\\)"):
        tf.ThermoEngine(db).thermoPropertiesSubstance(373.15, 1e5, "e-")
