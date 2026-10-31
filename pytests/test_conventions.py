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
LN_10 = 2.302585093   # the value of the engine
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


def output(e, **fields):
    c = tf.EngineConventions()
    for key, value in fields.items():
        setattr(c, key, value)
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
    got = props(engine({"X": substance(written_in(reference), convention={"energy_reference": reference})}), T=T, P=P)
    same(got, want)


@pytest.mark.parametrize("T", [Tr, 400.0, 1000.0])
def test_a_gas_in_one_atmosphere_gives_the_result_in_one_bar(T):
    gas = dict(formula="CO2", state="gas", cls="gas_fluid")
    want = props(engine({"X": substance(**gas)}), T=T)
    atm = dict(BASE, G=BASE["G"] + R * Tr * LN_F, S=BASE["S"] - R * LN_F)
    same(props(engine({"X": substance(atm, convention={"standard_pressure": "1atm"}, **gas)}), T=T), want)


def test_one_atmosphere_is_only_for_gases():
    same(props(engine({"X": substance(convention={"standard_pressure": "1atm"})})), props(engine({"X": substance()})))


def test_a_solute_in_mole_fraction_gives_the_result_in_molality():
    solute = dict(formula="NaCl", state="aqueous", cls="aqueous_solute")
    want = props(engine({"X": substance(**solute)}))
    x = dict(BASE, G=BASE["G"] + R * Tr * LN_X, S=BASE["S"] - R * LN_X)
    same(props(engine({"X": substance(x, convention={"concentration_scale": "mole_fraction"}, **solute)})), want)


def test_the_record_goes_before_the_dataset():
    values = written_in("berman_brown")
    dataset = {"symbol": "d", "convention": {"energy_reference": "berman_brown"}}
    want = props(engine({"X": substance()}))
    same(props(engine({"X": substance(values)}, dataset=dataset)), want)
    same(props(engine({"X": substance(convention={"energy_reference": "benson_helgeson"})}, dataset=dataset)), want)


def test_legacy_defaults_are_the_identity():
    # 1bar, molality, geochemical, benson_helgeson written out give the same bits as nothing written
    all_canonical = {"energy_reference": "benson_helgeson", "standard_pressure": "1bar", "concentration_scale": "molality", "water_reference": "geochemical"}
    assert props(engine({"X": substance(convention=all_canonical)})) == props(engine({"X": substance()}))


def test_the_reference_properties_of_a_record_without_methods_are_converted():
    values = written_in("berman_brown")
    record = substance(values, convention={"energy_reference": "berman_brown"})
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
    declared = props(engine({"X": substance(formula="CO2", state="gas", cls="gas_fluid", methods=NASA, convention={"energy_reference": "gibbs_helmholtz_formation"})}))
    assert declared == plain


def test_mixed_record_a_base_in_its_convention_and_a_correction_in_the_default():
    # the correction is not converted, and the base is: this fails if the conversion runs once at the end
    methods = NASA + [{"method": "ConstantVolume"}]
    e = engine({"X": substance(dict(BASE, V=2e-5), formula="CO2", state="crystal", methods=methods)})
    T, P = 500.0, 1e8
    G, _, _ = nasa_raw(T)
    got = props(e, T=T, P=P)
    assert got["G"] == pytest.approx(G + Tr * SIGMA_CO2 + 2e-5 * (P - Pr), rel=1e-12)
    assert got["V"] == pytest.approx(2.0, rel=1e-12)   # J/bar


def test_mixed_record_of_a_declared_base_and_a_correction():
    methods = [{"method": "ConstantCp"}, {"method": "ConstantVolume"}]
    want = props(engine({"X": substance(methods=methods)}), P=1e8)
    got = props(engine({"X": substance(written_in("gibbs_helmholtz_formation"), methods=methods, convention={"energy_reference": "gibbs_helmholtz_formation"})}), P=1e8)
    same(got, want)


# --- output ----------------------------------------------------------------------------------------------------------

@pytest.mark.parametrize("reference", ["berman_brown", "ser", "assigned_enthalpy", "dft_formation_0K"])
def test_output_of_a_reference_without_a_term_in_t(reference):
    plain, T = props(engine({"X": substance()})), 400.0
    got = props(output(engine({"X": substance()}), energy_reference=reference))
    assert got["G"] == pytest.approx(plain["G"] - Tr * SIGMA_CACO3, rel=1e-12)
    for key in ("H", "S", "Cp", "V", "U", "A", "dGdT", "dGdP"):
        assert got[key] == plain[key], key


def test_output_gibbs_helmholtz_formation():
    record, T = substance(methods=VOLUME), 400.0
    plain = props(engine({"X": record}))
    got = props(output(engine({"X": record}), energy_reference="gibbs_helmholtz_formation"))
    assert got["G"] == pytest.approx(plain["G"] + (T - Tr) * SIGMA_CACO3, rel=1e-12)
    assert got["S"] == pytest.approx(plain["S"] - SIGMA_CACO3, rel=1e-12)
    assert got["H"] == plain["H"]
    assert got["dGdT"] == pytest.approx(-got["S"], rel=1e-12)
    assert got["A"] == pytest.approx(got["U"] - T * got["S"], rel=1e-12)


def test_output_one_atmosphere():
    gas = substance(formula="CO2", state="gas", cls="gas_fluid", methods=VOLUME)
    plain, T = props(engine({"X": gas})), 400.0
    got = props(output(engine({"X": gas}), standard_pressure="1atm"))
    assert got["G"] == pytest.approx(plain["G"] + R * T * LN_F, rel=1e-12)
    assert got["S"] == pytest.approx(plain["S"] - R * LN_F, rel=1e-12)
    assert got["dGdT"] == pytest.approx(-got["S"], rel=1e-12)
    assert got["A"] == pytest.approx(got["U"] - T * got["S"], rel=1e-12)
    # a mineral does not change
    assert props(output(engine({"X": substance()}), standard_pressure="1atm")) == props(engine({"X": substance()}))


def test_output_mole_fraction():
    solute = substance(formula="NaCl", state="aqueous", cls="aqueous_solute")
    plain, T = props(engine({"X": solute})), 400.0
    got = props(output(engine({"X": solute}), concentration_scale="mole_fraction"))
    assert got["G"] == pytest.approx(plain["G"] + R * T * LN_X, rel=1e-12)


def test_a_conversion_and_its_inverse_are_the_identity():
    want = props(engine({"X": substance()}))
    for reference in ("berman_brown", "gibbs_helmholtz_formation"):
        out = output(engine({"X": substance()}), energy_reference=reference)
        # the values at the reference conditions, in that convention, give the same species back
        v = out.thermoPropertiesSubstance(Tr, Pr, "X")
        values = {"G": v.gibbs_energy.val, "H": v.enthalpy.val, "S": v.entropy.val, "Cp": v.heat_capacity_cp.val, "V": BASE["V"]}
        same(props(engine({"X": substance(values, convention={"energy_reference": reference})})), want)


def test_a_change_of_the_output_after_a_calculation_changes_the_next_result():
    e = engine({"X": substance()}, memoize=True)
    first = props(e)
    e.conventions.energy_reference = "berman_brown"
    second = props(e)
    assert second["G"] == pytest.approx(first["G"] - Tr * SIGMA_CACO3, rel=1e-12)
    output(e, energy_reference="benson_helgeson")
    assert props(e) == first


def test_the_old_words_still_work():
    a = engine({"X": substance()})
    a.conventions.apparent_properties = "BERMAN_BROWN"
    b = output(engine({"X": substance()}), energy_reference="berman_brown")
    assert props(a) == props(b)


def test_explain_says_what_was_converted():
    e = output(engine({"X": substance(formula="CO2", state="gas", cls="gas_fluid", methods=NASA)}), standard_pressure="1atm")
    conversions = e.thermoPropertiesSubstanceDetailed(500.0, Pr, "X")["conversions"]
    assert "NASA9 to canonical" in conversions and "1atm" in conversions


# --- the hydrogen ion -----------------------------------------------------------------------------------------------

def hydrogen(**more):
    record = {"formula": "H+", "class": "aqueous_solute", "aggregate_state": "aqueous", "charge": 1}
    record.update(more)
    return record


def test_the_hydrogen_ion_stays_zero_where_the_table_makes_its_entropy_zero():
    got = output(engine({"H+": hydrogen()}), energy_reference="berman_brown").thermoPropertiesSubstance(Tr, Pr, "H+")
    assert got.gibbs_energy.val == 0.0


def test_the_output_of_the_hydrogen_ion_with_another_charge_entropy():
    elements = dict(ELEMENT_RECORDS, Zz={"class": "charge", "standard_entropy": -60.0})
    e = engine({"H+": hydrogen()}, elements=elements)
    assert e.thermoPropertiesSubstance(Tr, Pr, "H+").gibbs_energy.val == 0.0
    output(e, energy_reference="berman_brown")
    assert e.thermoPropertiesSubstance(Tr, Pr, "H+").gibbs_energy.val == pytest.approx(-Tr * (65.34 - 60.0), rel=1e-12)


def test_the_hydrogen_ion_in_mole_fraction_output():
    e = output(engine({"H+": hydrogen()}), concentration_scale="mole_fraction")
    assert e.thermoPropertiesSubstance(400.0, Pr, "H+").gibbs_energy.val == pytest.approx(R * 400.0 * LN_X, rel=1e-12)


def test_the_hydrogen_ion_in_berman_brown_with_values_is_converted():
    values = {"G": -1000.0, "H": -1000.0, "S": 1.0, "Cp": 0.0, "V": 0.0}
    elements = dict(ELEMENT_RECORDS, Zz={"class": "charge", "standard_entropy": -60.0})
    e = engine({"H+": hydrogen(standard_state_properties=values, methods=[{"method": "ConstantCp"}], convention={"energy_reference": "berman_brown"})}, elements=elements)
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
            "H+": hydrogen(standard_state_properties=values, methods=[hkf]),
            "HB": hydrogen(standard_state_properties=values, methods=[hkf], convention={"energy_reference": "berman_brown"})}
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
    e = molarity_engine(convention={"concentration_scale": "molarity"})
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
    assert (b["V"] - a["V"]) * 1e-5 == pytest.approx(dVdiff, rel=1e-4, abs=1e-12)
    h = lambda t: R * t * math.log(density(e, t, P) / 1000.0) + t * -(term(t + dt, P) - term(t - dt, P)) / (2 * dt)
    assert b["Cp"] - a["Cp"] == pytest.approx((h(T + 0.5) - h(T - 0.5)) / 1.0, rel=2e-3, abs=2e-3)


def test_output_in_molarity_is_the_inverse():
    subs = lambda **more: {"H2O@": WATER, "X": substance(formula="NaCl", state="aqueous", cls="aqueous_solute", **more)}
    declared = engine(subs(convention={"concentration_scale": "molarity"}))
    output(declared, concentration_scale="molarity")
    same(props(declared, T=350.0), {k: v for k, v in props(engine(subs()), T=350.0).items() if k in ("G", "H", "S", "Cp", "V")}, rel=1e-10, abs_=1e-7)


def test_molarity_without_a_solvent_is_an_error():
    e = engine({"X": substance(formula="NaCl", state="aqueous", cls="aqueous_solute", convention={"concentration_scale": "molarity"})})
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
    c.energy_reference = "gibbs_helmholtz_formation"
    native.setConventions(c)
    uq_raw = G(native, "UQ")
    uq = uq_raw + (Tr - T) * sigma_uq                        # by hand, from its native convention
    ng = nasa_raw(T)[0] + Tr * sigma_ng                      # by hand, from the polynomial
    dG = G(e, "Ca+2") + ng + G(e, water) - uq - 2 * G(e, "H+")

    r = e.thermoPropertiesReaction(T, P, "R")
    assert r.reaction_gibbs_energy.val == pytest.approx(dG, rel=1e-10)
    assert r.log_equilibrium_constant.val == pytest.approx(-dG / (R * T) * 0.434294481, rel=1e-10)


@pytest.mark.parametrize("field, word", [("energy_reference", "berman_brown"), ("energy_reference", "gibbs_helmholtz_formation"), ("water_reference", "steam_tables")])
def test_log_k_does_not_depend_on_the_energy_output(field, word):
    text, _, _ = reaction_db()
    plain = tf.ThermoEngine(tf.Database(text)).thermoPropertiesReaction(400.0, 1e5, "R").log_equilibrium_constant.val
    other = output(tf.ThermoEngine(tf.Database(text)), **{field: word}).thermoPropertiesReaction(400.0, 1e5, "R").log_equilibrium_constant.val
    assert other == pytest.approx(plain, rel=1e-12)


def test_log_k_in_one_atmosphere():
    text, _, _ = reaction_db()
    plain = tf.ThermoEngine(tf.Database(text)).thermoPropertiesReaction(400.0, 1e5, "R")
    atm = output(tf.ThermoEngine(tf.Database(text)), standard_pressure="1atm").thermoPropertiesReaction(400.0, 1e5, "R")
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
    doc["reactions"] = {"R": record, "A": dict(record, convention={"standard_pressure": "1atm"})}
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
    bb = output(tf.ThermoEngine(MINES), energy_reference="berman_brown")
    assert bb.thermoPropertiesSubstance(400.0, 1e5, symbol).gibbs_energy.val == pytest.approx(plain - Tr * sigma, rel=1e-12)
    steam = output(tf.ThermoEngine(MINES), water_reference="steam_tables")
    assert steam.thermoPropertiesSubstance(400.0, 1e5, symbol).gibbs_energy.val == plain


@pytest.mark.parametrize("symbol", ["CH4@", "CO2@", "H2@", "O2@"])
def test_the_reference_of_water_does_not_reach_a_method_that_reads_water(symbol):
    aq = "pytests/test-aq17-gem-lma-thermofun.json"
    plain = tf.ThermoEngine(aq).thermoPropertiesSubstance(400.0, 1e5, symbol).gibbs_energy.val
    steam = output(tf.ThermoEngine(aq), water_reference="steam_tables")
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
    record = substance(convention={"energy_reference": "berman_brown"})
    assert "CNV-001" in codes(database({"X": record}, elements=elements))
    e = tf.ThermoEngine(database({"X": record}, elements=elements))
    with pytest.raises(RuntimeError, match="CNV-001.*Ca"):
        props(e)


def test_cnv_002_molarity_without_a_solvent():
    record = substance(formula="NaCl", state="aqueous", cls="aqueous_solute", convention={"concentration_scale": "molarity"})
    assert "CNV-002" in codes(database({"X": record}))
    assert "CNV-002" not in codes(database({"X": record, "H2O@": WATER}))


def test_cnv_003_a_method_in_another_convention_than_the_dataset():
    record = substance(formula="CO2", state="gas", cls="gas_fluid", methods=NASA)
    assert "CNV-003" in codes(database({"X": record}))
    assert "CNV-003" not in codes(database({"X": dict(record, convention={"energy_reference": "assigned_enthalpy"})}))
    assert "CNV-003" not in codes(database({"X": record}, dataset={"symbol": "d", "convention": {"energy_reference": "assigned_enthalpy"}}))


def test_cnv_004_a_reaction_of_species_in_different_references():
    subs = {"X": substance(), "Y": substance(convention={"energy_reference": "berman_brown"}), "Z": substance()}
    assert "CNV-004" in codes(database(subs, reactions={"R": {"equation": "X = Y"}}))
    assert "CNV-004" not in codes(database(subs, reactions={"R": {"equation": "X = Z"}}))


def test_cnv_005_an_unknown_word_is_an_error_with_the_allowed_words():
    with pytest.raises(Exception, match="CNV-005"):
        database({"X": substance(convention={"energy_reference": "nonsense"})})


def test_cnv_006_an_element_from_the_table_of_chemicalfun():
    record = substance(convention={"energy_reference": "berman_brown"})
    assert "CNV-006" in codes(database({"X": record}, elements={}))
    assert "CNV-006" not in codes(database({"X": record}))
    # and the value of that table is used
    got = props(tf.ThermoEngine(database({"X": record}, elements={})))
    assert math.isfinite(got["G"])
