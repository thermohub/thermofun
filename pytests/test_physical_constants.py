# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The constants of physics at run time (step 3b of the vocabulary plan): the engine option physical_constants is the R of the
calculation; fitted equations keep the R of their fit."""

import json

import pytest

import thermofun as tf

PSI = "pytests/PsiTDB2020-subset-thermofun.json"
AQ17 = "Resources/databases/aq17-thermofun.json"
R1986 = tf.methods.physicalConstants("codata1986")["R"]
R2018 = tf.methods.physicalConstants("codata2018")["R"]


def engine(path, name="codata1986", standard_state_of_gases=False):
    e = tf.ThermoEngine(path)
    p = e.preferences
    p.physical_constants = name
    if standard_state_of_gases:
        p.useStandardStateOfGases()
    e.setPreferences(p)
    return e


def test_the_sets():
    assert sorted(tf.methods.physicalConstantsNames()) == ["codata1986", "codata2018"]
    assert R1986 / R2018 - 1 == pytest.approx(5.6987e-6, rel=1e-4)
    with pytest.raises(ValueError, match="Allowed: "):
        engine(AQ17, "codata2020")


@pytest.mark.parametrize("T", [298.15, 373.15, 573.15])
def test_log_k_and_dg_of_a_reaction_follow_r(T):
    old, new = engine(PSI), engine(PSI, "codata2018")
    for reaction in tf.Database(PSI).getReactionsList()[:30]:
        a, b = old.thermoPropertiesReaction(T, 1e5, reaction), new.thermoPropertiesReaction(T, 1e5, reaction)
        # one of the two is the stored value, the other follows from dG = -R T ln(10) log K
        lgK = a.log_equilibrium_constant.val, b.log_equilibrium_constant.val
        dG = a.reaction_gibbs_energy.val, b.reaction_gibbs_energy.val
        assert dG[1] / (-R2018 * T * 2.302585092994046 * lgK[1]) == pytest.approx(1.0, rel=1e-12) if lgK[1] != 0 else True, reaction
        assert lgK[0] == pytest.approx(lgK[1], rel=1e-12) or dG[0] == pytest.approx(dG[1], rel=1e-12), reaction


def test_the_ideal_gas_term_follows_r():
    old, new = engine(AQ17), engine(AQ17, "codata2018")
    T, P = 473.15, 1e7
    a, b = old.thermoPropertiesSubstance(T, P, "CO2"), new.thermoPropertiesSubstance(T, P, "CO2")
    import math
    assert b.gibbs_energy.val - a.gibbs_energy.val == pytest.approx((R2018 - R1986) * T * math.log(P / 1e5), rel=1e-9)
    assert b.enthalpy.val == a.enthalpy.val
    # a condensed phase from its Cp does not use R
    assert old.thermoPropertiesSubstance(T, P, "Calcite").gibbs_energy.val == new.thermoPropertiesSubstance(T, P, "Calcite").gibbs_energy.val


@pytest.mark.parametrize("path,symbol", [(AQ17, "H2O@"), ("pytests/data/legacy-synthetic/s33-water_pvt_zhang_duan05.json", None),
                                         ("pytests/data/legacy-synthetic/s10-fluid_churakov_gottschalk.json", None),
                                         ("pytests/data/legacy-synthetic/s12-fluid_sterner_pitzer.json", None)])
def test_fitted_equations_keep_the_r_of_their_fit(path, symbol):
    symbols = [symbol] if symbol else tf.Database(path).getSubstancesList()
    # without the terms of the standard state of a gas, which are thermodynamics, not the fit
    old, new = engine(path, standard_state_of_gases=True), engine(path, "codata2018", standard_state_of_gases=True)
    for s in symbols:
        for T, P in [(573.15, 1e8), (873.15, 5e8)]:
            try:
                a = old.thermoPropertiesSubstance(T, P, s)
            except RuntimeError:
                continue
            b = new.thermoPropertiesSubstance(T, P, s)
            for field in ("gibbs_energy", "enthalpy", "entropy", "volume"):
                assert getattr(b, field).val == getattr(a, field).val, (s, T, P, field)


@pytest.mark.parametrize("path", ["pytests/data/legacy-synthetic/s11-fluid_soave_redlich_kwong.json",
                                  "pytests/data/legacy-synthetic/s13-fluid_peng_robinson78.json"])
def test_the_cubic_equations_follow_r(path):
    # in Peng-Robinson, PRSV and Soave-Redlich-Kwong R is a physical constant (a = 0.457 R^2 Tc^2/Pc, ...), not a fitted value
    symbols = tf.Database(path).getSubstancesList()
    old, new = engine(path, standard_state_of_gases=True), engine(path, "codata2018", standard_state_of_gases=True)
    changed = 0
    for s in symbols:
        a, b = old.thermoPropertiesSubstance(573.15, 1e8, s), new.thermoPropertiesSubstance(573.15, 1e8, s)
        # with the standard state of gases G keeps no term of the equation; V and H carry it
        for field in ("volume", "enthalpy"):
            assert getattr(b, field).val == pytest.approx(getattr(a, field).val, rel=1e-4)   # R changed by 6e-6 only
        changed += b.volume.val != a.volume.val and b.enthalpy.val != a.enthalpy.val
    assert changed == len(symbols)


def test_a_change_in_place_applies_from_the_next_calculation():
    e = engine(PSI)
    reaction = tf.Database(PSI).getReactionsList()[1]
    before = e.thermoPropertiesReaction(373.15, 1e5, reaction).log_equilibrium_constant.val
    e.preferences.physical_constants = "codata2018"
    after = e.thermoPropertiesReaction(373.15, 1e5, reaction).log_equilibrium_constant.val
    assert after != before


def test_a_dataset_says_its_set():
    doc = {"thermohub": "2.0.0-draft.1", "thermodataset": {"symbol": "d", "physical_constants": "codata2018"},
           "substances": {"A": {"formula": "Ca", "class": "component", "aggregate_state": "crystal",
                                "standard_state_properties": {"G": -1000.0, "H": -1000.0, "S": 10.0, "Cp": 20.0, "V": 1e-5},
                                "methods": [{"method": "ConstantCp"}]}}}
    db = tf.Database(json.dumps(doc))
    assert db.thermoDataSet().physical_constants == "codata2018"
    assert json.loads(db.toJson(True))["thermodataset"]["physical_constants"] == "codata2018"
    doc["thermodataset"]["physical_constants"] = "codata"
    with pytest.raises(RuntimeError, match="codata1986"):
        tf.Database(json.dumps(doc))
