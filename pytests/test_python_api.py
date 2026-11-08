# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The Python API of ThermoFun: enumerations, models, units, records, database, engine and batch."""
import json
import os

import pytest
import thermofun as tf


HERE = os.path.dirname(os.path.abspath(__file__))
DB = os.path.join(HERE, "test-thermoengine-thermofun.json")


@pytest.fixture(scope="module")
def database():
    return tf.Database(DB)


@pytest.fixture(scope="module")
def engine(database):
    return tf.ThermoEngine(database)


# --- enumerations ------------------------------------------------------------------------------------------------

def test_enumerations_of_the_setters_and_getters():
    s = tf.Substance()
    s.setSubstanceClass(tf.SubstanceClass.AQSOLUTE)
    s.setAggregateState(tf.AggregateState.CRYSTAL)
    s.setThermoCalculationType(tf.SubstanceThermoCalculationType.REACDC)
    s.setMethodGenEoS(tf.MethodGenEoS_Thrift.CTPM_CON)
    s.setMethod_T(tf.MethodCorrT_Thrift.CTM_CHP)
    s.setMethod_P(tf.MethodCorrP_Thrift.CPM_NUL)
    assert s.substanceClass() == tf.SubstanceClass.AQSOLUTE
    assert s.aggregateState() == tf.AggregateState.CRYSTAL
    assert s.thermoCalculationType() == tf.SubstanceThermoCalculationType.REACDC
    assert s.methodGenEOS() == tf.MethodGenEoS_Thrift.CTPM_CON
    assert s.method_T() == tf.MethodCorrT_Thrift.CTM_CHP
    assert s.method_P() == tf.MethodCorrP_Thrift.CPM_NUL

    r = tf.Reaction()
    r.setMethod_T(tf.MethodCorrT_Thrift.CTM_EK3)
    r.setMethod_P(tf.MethodCorrP_Thrift.CPM_VKE)
    r.setMethodGenEoS(tf.MethodGenEoS_Thrift.CTPM_CON)
    assert r.method_T() == tf.MethodCorrT_Thrift.CTM_EK3
    assert r.method_P() == tf.MethodCorrP_Thrift.CPM_VKE


def test_enumerations_of_the_records_of_a_database(database):
    cal = database.getSubstance("Cal")
    assert cal.aggregateState() == tf.AggregateState.CRYSTAL
    assert cal.substanceClass() == tf.SubstanceClass.COMPONENT
    ca = database.getSubstance("Ca+2")
    assert ca.substanceClass() == tf.SubstanceClass.AQSOLUTE


def test_method_codes_and_constants():
    assert tf.SubstanceTPMethodType.cp_ft_equation == tf.SubstanceTPMethodType(0)
    assert tf.ReactionTPMethodType.logk_fpt_function == tf.ReactionTPMethodType(0)
    assert tf.enum_method_substance[0] == "cp_ft_equation"
    assert tf.enum_method_reaction[0] == "logk_fpt_function"
    assert tf.R_CONSTANT == pytest.approx(8.31451)
    assert tf.lg_to_ln * tf.ln_to_lg == pytest.approx(1.0, abs=1e-8)


# --- units and records -------------------------------------------------------------------------------------------

def test_units():
    assert tf.convert_units(1.0, "cal/mol", "J/mol") == pytest.approx(4.184)
    assert tf.units_convertible("kJ/mol", "J/mol")
    assert not tf.units_convertible("kJ/mol", "K")


def test_parse_records(database):
    cal = tf.parseSubstance(database.getSubstance("Cal").jsonString())
    assert cal.symbol() == "Cal"
    assert tf.parseElement(database.getElements()[0].jsonString()).symbol() == database.getElements()[0].symbol()
    reaction = database.getReactions()[0]
    assert tf.parseReaction(reaction.jsonString()).symbol() == reaction.symbol()
    assert tf.ThermoVariables().temperature.val == 0.0


# --- database and engine -----------------------------------------------------------------------------------------

def test_database_map_setters_and_entropy_of_formula(database):
    db = tf.Database(DB)
    cal = db.getSubstance("Cal")
    cal.setName("changed name")
    db.setMapSubstances({"Cal": cal})
    assert db.getSubstance("Cal").name() == "changed name"
    assert db.numberOfSubstances() == database.numberOfSubstances()
    db.setMapElements({e.symbol(): e for e in db.getElements()})
    db.setMapReactions({r.symbol(): r for r in db.getReactions()})
    assert db.elementalEntropyFormula("CaCO3") > 0


def test_engine_database_and_preferences(engine):
    assert engine.database().containsSubstance("Cal")
    eng = tf.ThermoEngine(DB)
    prefs = tf.EnginePreferences()
    prefs.enable_memoize = False
    prefs.round_to_uncertainty = True
    eng.setPreferences(prefs)
    assert eng.preferences.enable_memoize is False and eng.preferences.round_to_uncertainty is True
    assert eng.thermoPropertiesSubstance(298.15, 1e5, "Cal").gibbs_energy.val == pytest.approx(
        tf.ThermoEngine(DB).thermoPropertiesSubstance(298.15, 1e5, "Cal").gibbs_energy.val, abs=1.0)


def test_check_calc_method_bounds(database):
    cal = database.getSubstance("Cal")
    tps = tf.ThermoPropertiesSubstance()
    out = cal.checkCalcMethodBounds("test", 300.0, 1e5, tps)
    assert isinstance(out, tf.ThermoPropertiesSubstance)
    reaction = database.getReactions()[0]
    assert isinstance(reaction.checkCalcMethodBounds("test", 300.0, 1e5, tf.ThermoPropertiesReaction()), tf.ThermoPropertiesReaction)
    assert isinstance(reaction.thermo_ref_prop(), tf.ThermoPropertiesReaction)


# --- models ------------------------------------------------------------------------------------------------------

def test_internal_helmholtz_energy_and_cv_are_completed_from_the_other_properties(database, engine):
    """U = H - P V, A = U - T S and Cv = Cp - T V alpha^2/beta for every substance with a molar volume."""
    checked = 0
    for symbol in ["H2O@", "Al(OH)2+", "Al(OH)3@", "Al(OH)4-", "Quartz"]:
        for T, P in [(298.15, 1e5), (473.15, 5e7)]:
            x = engine.thermoPropertiesSubstance(T, P, symbol)
            u = x.enthalpy.val - P * x.volume.val  # V in m3/mol, P in Pa
            assert x.internal_energy.val == pytest.approx(u, rel=1e-9, abs=1e-6)
            assert x.helmholtz_energy.val == pytest.approx(u - T * x.entropy.val, rel=1e-9, abs=1e-6)
            if symbol != "H2O@":
                assert x.heat_capacity_cv.val <= x.heat_capacity_cp.val + 1e-9
            checked += 1
    assert checked == 10


# --- batch -------------------------------------------------------------------------------------------------------

def test_a_copy_of_an_engine_does_not_depend_on_the_original():
    """A ThermoBatch (and an engine copy) calculates with its own copy of the engine: the original can go away."""
    ref = tf.ThermoEngine(DB).thermoPropertiesSubstance(298.15, 1e5, "Cal").gibbs_energy.val
    batch = tf.ThermoBatch(tf.ThermoEngine(DB))  # the engine is a temporary
    batch.setPropertiesUnits(["temperature", "pressure"], ["K", "Pa"])
    assert batch.thermoPropertiesSubstance(298.15, 1e5, "Cal", "gibbs_energy").toDouble() == pytest.approx(ref)


def test_batch_units_and_digits(database, tmp_path):
    batch = tf.ThermoBatch(database)
    units, digits = dict(batch.propertyUnits()), dict(batch.propertyDigits())
    units["gibbs_energy"], digits["gibbs_energy"] = "kJ/mol", 3
    batch.setUnits(units)
    batch.setDigits(digits)
    out = batch.thermoPropertiesSubstance(298.15, 1e5, ["Cal"], ["gibbs_energy"])
    assert out.to2DVectorDouble()[0][0] == pytest.approx(-1129178.0)
    out.toCSV(str(tmp_path / "out.csv"))
    lines = (tmp_path / "out.csv").read_text().splitlines()
    assert lines[0].endswith("gibbs_energy(kJ/mol)") and lines[1].split(",")[-1] == "-1129.178"



def test_the_default_digits_show_a_volume_in_m3_per_mol(database, tmp_path):
    """V is in m3/mol: with the 3 decimals of J/bar, every volume was written as 0.000."""
    batch = tf.ThermoBatch(database)
    batch.thermoPropertiesSubstance(298.15, 1e5, ["Cal"], ["volume"]).toCSV(str(tmp_path / "v.csv"))
    value = (tmp_path / "v.csv").read_text().splitlines()[1].split(",")[-1]
    assert float(value) == pytest.approx(3.6934e-5, rel=1e-3), value

# --- ThermoScalar ------------------------------------------------------------------------------------------------

def test_thermoscalar_arithmetic_and_variables():
    x = tf.ThermoScalar(2.0, 3.0, 5.0, 0.1, (tf.Status.calculated, ""))
    y = tf.ThermoScalar(4.0, 1.0, 2.0, 0.2, (tf.Status.calculated, ""))
    s, p = x + y, x * y
    assert (s.val, s.ddt, s.ddp) == (6.0, 4.0, 7.0)
    assert (p.val, p.ddt, p.ddp) == (8.0, 3 * 4.0 + 2.0 * 1.0, 5 * 4.0 + 2.0 * 2.0)
    assert (x - 1.0).val == 1.0 and (1.0 - x).val == -1.0 and (2.0 * x).ddt == 6.0
    assert (x / 2.0).val == 1.0 and (-x).val == -2.0 and (x ** 2).val == 4.0 and float(x) == 2.0
    assert (x + y).err == pytest.approx((0.1 ** 2 + 0.2 ** 2) ** 0.5)
    assert "ThermoScalar" in repr(x)
    v = tf.ThermoVariables()
    v.temperature = tf.Temperature(300.0)
    v.pressure = tf.Pressure(1e5)
    assert v.temperature.val == 300.0 and v.temperature.ddt == 1.0 and v.pressure.ddp == 1.0


# --- exact derivatives of the Zhang-Duan water and of the dielectric models ---------------------------------------

def test_the_water_methods_alone():
    for implementation, method in (("reaktoro", "WaterHGK84"), ("gems", "WaterHGK84"), ("", "WaterIAPWS95")):
        ps = tf.methods.solventMethod(method, implementation=implementation).properties(298.15, 1e5, 0)
        assert ps.density.val == pytest.approx(997.0, abs=1.5)
        assert ps.density.ddt < 0
    ps = tf.methods.solventMethod("WaterHGK84").properties(298.15, 1e5, 0)
    eps = tf.methods.dielectricMethod("JohnsonNorton91", implementation="gems").properties(298.15, 1e5, ps, 0)
    assert eps.epsilon.val == pytest.approx(78.4, abs=0.5)
    assert tf.methods.dielectricMethod("JohnsonNorton91").properties(298.15, 1e5, ps, 0).epsilon.val == pytest.approx(78.4, abs=0.5)


def test_the_old_model_classes_are_gone():
    for name in ("ThermoModelsSubstance", "ConMolVol", "GasPRSV", "WaterHGK", "SoluteHKFgems", "ReactionFromReactantsProperties"):
        assert not hasattr(tf, name), name
