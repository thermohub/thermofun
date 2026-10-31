# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The settings of the engine are typed; words are checked, and the old string fields still work."""

import json
import os

import pytest
import thermofun as tf

HERE = os.path.dirname(os.path.abspath(__file__))
DB = os.path.join(HERE, "test-aq17-gem-lma-thermofun.json")
T, P = 373.15, 1e6


def gibbs(engine, symbol):
    return engine.thermoPropertiesSubstance(T, P, symbol).gibbs_energy.val


# --- words ---------------------------------------------------------------------------------------

def test_a_wrong_word_gives_a_hint_and_the_words_allowed():
    p = tf.EnginePreferences()
    with pytest.raises(ValueError, match='Did you mean "reaction"') as e:
        p.property_source = "reactoin"
    assert "reactants" in str(e.value) and "record" in str(e.value)
    assert p.property_source == tf.PropertySource.Record


def test_a_setting_takes_the_enum_or_a_word_in_any_case():
    p = tf.EnginePreferences()
    p.property_source = "REACTION"
    assert p.property_source == tf.PropertySource.Reaction
    p.property_source = tf.PropertySource.Direct
    assert p.property_source == tf.PropertySource.Direct
    c = tf.EngineConventions()
    c.water_reference = "steam-tables"
    assert c.water_reference == tf.WaterReference.SteamTables
    c.energy_reference = tf.EnergyReference.BermanBrown
    assert c.energy_reference == tf.EnergyReference.BermanBrown


# --- old fields ----------------------------------------------------------------------------------

def test_the_old_fields_give_their_defaults_and_set_the_typed_values():
    c = tf.EngineConventions()
    assert c.apparent_properties == "BENSON_HELGESON"
    assert c.water_properties == "GEOCHEMICAL"
    c.apparent_properties = "BERMAN_BROWN"
    c.water_properties = "STEAM_TABLES"
    assert c.energy_reference == tf.EnergyReference.BermanBrown
    assert c.water_reference == tf.WaterReference.SteamTables
    assert c.apparent_properties == "BERMAN_BROWN"


def test_a_wrong_old_word_is_refused():
    c = tf.EngineConventions()
    with pytest.raises(ValueError, match='Did you mean "steam_tables"'):
        c.water_properties = "STEAM_TABLE"


@pytest.mark.parametrize("old, typed, symbol", [("water_properties", "water_reference", "H2O@"),
                                                ("apparent_properties", "energy_reference", "Ca+2")])
def test_old_and_typed_fields_give_the_same_result(old, typed, symbol):
    word, value = ("STEAM_TABLES", tf.WaterReference.SteamTables) if old == "water_properties" else ("BERMAN_BROWN", tf.EnergyReference.BermanBrown)
    a, b, plain = tf.ThermoEngine(DB), tf.ThermoEngine(DB), tf.ThermoEngine(DB)
    gibbs(a, symbol)
    setattr(a.conventions, old, word)
    setattr(b.conventions, typed, value)
    assert gibbs(a, symbol) == gibbs(b, symbol)
    assert gibbs(a, symbol) != gibbs(plain, symbol)


# --- checks --------------------------------------------------------------------------------------

@pytest.mark.parametrize("field, value", [("energy_reference", "ser"), ("standard_pressure", "1atm"), ("concentration_scale", "molarity")])
def test_a_convention_that_is_not_calculated_yet_is_refused(field, value):
    c = tf.EngineConventions()
    setattr(c, field, value)
    engine = tf.ThermoEngine(DB)
    with pytest.raises(ValueError, match="not calculated yet"):
        engine.setConventions(c)
    assert gibbs(engine, "Ca+2") == gibbs(tf.ThermoEngine(DB), "Ca+2")


def test_a_change_in_place_is_checked_at_the_next_calculation():
    engine = tf.ThermoEngine(DB)
    gibbs(engine, "Ca+2")
    engine.conventions.standard_pressure = "1atm"
    with pytest.raises(ValueError, match="not calculated yet"):
        gibbs(engine, "Ca+2")


@pytest.mark.parametrize("field, value, message", [("solventState", 2, "solvent state"), ("uncertainty_significant_digits", 0, "significant digits")])
def test_numbers_of_the_preferences_are_checked(field, value, message):
    p = tf.EnginePreferences()
    setattr(p, field, value)
    with pytest.raises(ValueError, match=message):
        tf.ThermoEngine(DB).setPreferences(p)


# --- property source -----------------------------------------------------------------------------

def database_with_both_ways():
    """C is defined directly and by a reaction with other properties."""
    def substance(symbol, G, **more):
        return {"symbol": symbol, "formula": symbol, "class_": {"0": "SC_COMPONENT"}, "aggregate_state": {"3": "AS_CRYSTAL"},
                "sm_gibbs_energy": {"values": [G]}, "sm_enthalpy": {"values": [G]}, "sm_entropy_abs": {"values": [10.0]},
                "sm_heat_capacity_p": {"values": [20.0]}, "sm_volume": {"values": [1.0]}, **more}
    methods = [{"method": {"43": "standard_entropy_cp_integration"}}]
    return tf.Database(json.dumps({
        "substances": [substance("A", -1000.0, TPMethods=methods), substance("B", -2000.0, TPMethods=methods),
                       substance("C", -3000.0, TPMethods=methods, reaction="R")],
        "reactions": [{"symbol": "R", "equation": "A + B = C", "reactants": [{"symbol": "A", "coefficient": -1}, {"symbol": "B", "coefficient": -1}, {"symbol": "C", "coefficient": 1}],
                       "logKr": {"values": [1.0]}, "TPMethods": [{"method": {"4": "logk_1_term_extrap0"}}]}]}))


def test_the_source_of_one_record_changes_only_that_record():
    engine = tf.ThermoEngine(database_with_both_ways())
    direct = gibbs_at_reference(engine, "C")
    assert direct == pytest.approx(-3000.0)
    assert engine.propertySource("C") == tf.PropertySource.Record

    engine.setPropertySource("C", "reaction")
    assert engine.propertySource("C") == tf.PropertySource.Reaction
    assert engine.propertySource("A") == tf.PropertySource.Record
    from_reaction = gibbs_at_reference(engine, "C")
    assert from_reaction != pytest.approx(direct)
    assert gibbs_at_reference(engine, "A") == pytest.approx(-1000.0)

    engine.setPropertySource("C", tf.PropertySource.Record)
    assert gibbs_at_reference(engine, "C") == pytest.approx(direct)


def gibbs_at_reference(engine, symbol):
    return engine.thermoPropertiesSubstance(298.15, 1e5, symbol).gibbs_energy.val


def test_the_source_in_the_preferences_applies_to_all_and_one_record_can_differ():
    engine = tf.ThermoEngine(database_with_both_ways())
    engine.preferences.property_source = "reaction"
    from_reaction = gibbs_at_reference(engine, "C")
    assert from_reaction != pytest.approx(-3000.0)
    assert gibbs_at_reference(engine, "A") == pytest.approx(-1000.0)   # it has no reaction: the source is ignored
    engine.setPropertySource("C", "direct")
    assert gibbs_at_reference(engine, "C") == pytest.approx(-3000.0)


def test_a_reaction_from_its_reactants():
    engine = tf.ThermoEngine(database_with_both_ways())
    own = engine.thermoPropertiesReaction(298.15, 1e5, "R").log_equilibrium_constant.val
    assert own == pytest.approx(1.0)
    engine.setPropertySource("R", "reactants")
    r = engine.thermoPropertiesReaction(298.15, 1e5, "R")
    assert r.reaction_gibbs_energy.val == pytest.approx(-3000.0 + 1000.0 + 2000.0, abs=1e-6)
    engine.setPropertySource("R", "direct")
    assert engine.thermoPropertiesReaction(298.15, 1e5, "R").log_equilibrium_constant.val == pytest.approx(1.0)


def test_a_substance_defined_by_its_reaction_and_taken_from_the_reactants_names_the_problem():
    engine = tf.ThermoEngine(database_with_both_ways())
    engine.setPropertySource("C", "reaction")
    engine.setPropertySource("R", "reactants")
    with pytest.raises(RuntimeError):
        engine.thermoPropertiesSubstance(298.15, 1e5, "C")


def test_a_wrong_source_is_refused():
    engine = tf.ThermoEngine(database_with_both_ways())
    with pytest.raises(ValueError, match="Allowed"):
        engine.setPropertySource("C", "nowhere")


def test_the_source_is_also_used_for_the_errors_of_the_parameters():
    engine = tf.ThermoEngine(database_with_both_ways())
    engine.setPropertySource("C", "reaction")
    copy = tf.ThermoEngine(engine)
    assert gibbs_at_reference(copy, "C") == pytest.approx(gibbs_at_reference(engine, "C"))


# --- implementations -----------------------------------------------------------------------------

def test_an_implementation_replaces_that_of_the_records():
    plain, chosen = tf.ThermoEngine(DB), tf.ThermoEngine(DB)
    assert gibbs(plain, "Ca+2") == gibbs(chosen, "Ca+2")
    chosen.preferences.method_implementation = {"HKF88": "gems"}
    value = gibbs(chosen, "Ca+2")
    assert value != gibbs(plain, "Ca+2")
    assert value == pytest.approx(gibbs(plain, "Ca+2"), rel=1e-3)
    chosen.preferences.method_implementation = {}
    assert gibbs(chosen, "Ca+2") == gibbs(plain, "Ca+2")


@pytest.mark.parametrize("choice, message", [({"NoSuchMethod": "gems"}, "no method NoSuchMethod"), ({"HKF88": "nowhere"}, "has no implementation")])
def test_a_wrong_implementation_is_refused(choice, message):
    p = tf.EnginePreferences()
    p.method_implementation = choice
    with pytest.raises(ValueError, match=message):
        tf.ThermoEngine(DB).setPreferences(p)
