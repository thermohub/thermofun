# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# Methods combined by region (decision 0028): methods of a kind that gives the value directly (water equations of state,
# dielectric models) may carry a region; the first whose region holds T and P is used, in the order of the record.

import pytest
import thermofun as tf


def engine(water_methods):
    water = {"formula": "H2O@", "class": "water_solvent", "aggregate_state": "aqueous", "standard_state_properties": {"G": -237181},
             "methods": water_methods}
    return tf.ThermoEngine(tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"H2O@": water}}))


HGK = {"method": "WaterHGK84"}
BP = {"method": "BradleyPitzer79"}
HK = {"method": "HelgesonKirkham74"}


def eps(e, T, P):
    return e.electroPropertiesSolvent(T, P, "H2O@").epsilon.val


def test_tanger_and_helgeson_dielectric_bradley_pitzer_at_low_t_above_1_kbar():
    """Tanger and Helgeson (1988): Bradley-Pitzer below 70 degC above 1 kbar, Helgeson-Kirkham elsewhere."""
    combined = engine([HGK, dict(BP, region={"t_max": 343.15, "p_min": 1.0e8}), HK])
    bp, hk = engine([HGK, BP]), engine([HGK, HK])
    assert eps(combined, 298.15, 2.0e8) == eps(bp, 298.15, 2.0e8)
    assert eps(combined, 298.15, 1.0e7) == eps(hk, 298.15, 1.0e7)
    assert eps(combined, 400.0, 2.0e8) == eps(hk, 400.0, 2.0e8)
    assert eps(bp, 298.15, 2.0e8) != eps(hk, 298.15, 2.0e8)


def test_a_water_equation_in_a_region():
    combined = engine([{"method": "WaterIAPWS95", "region": {"p_min": 1.0e9}}, HGK, HK])
    iapws, hgk = engine([{"method": "WaterIAPWS95"}, HK]), engine([HGK, HK])
    rho = lambda e, T, P: e.propertiesSolvent(T, P, "H2O@").density.val
    assert rho(combined, 600.0, 2.0e9) == rho(iapws, 600.0, 2.0e9)
    assert rho(combined, 600.0, 1.0e8) == rho(hgk, 600.0, 1.0e8)
    assert combined.saturationPressure(500.0) == iapws.saturationPressure(500.0)  # the region holds T (no P bound checked at P = 0)


def test_the_region_is_kept_when_written():
    db = tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"H2O@": {
        "formula": "H2O@", "class": "water_solvent", "aggregate_state": "aqueous", "standard_state_properties": {"G": -237181},
        "methods": [HGK, dict(BP, region={"t_max": 343.15, "p_min": 1.0e8}), HK]}}})
    methods = db.get("/substances/H2O@/methods")
    assert methods[1]["region"] == {"t_max": 343.15, "p_min": 1.0e8}
    assert tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"H2O@": db.get("/substances/H2O@")}}).get("/substances/H2O@/methods") == methods


def test_a_region_on_a_method_of_a_substance_is_refused():
    na = {"formula": "Na+", "charge": 1, "class": "aqueous_solute", "aggregate_state": "aqueous", "standard_state_properties": {"G": -261881.0, "S": 58.4},
          "methods": [{"method": "HKF88", "region": {"t_max": 400.0},
                       "parameters": {"a1": 0.1839, "a2": -228.5, "a3": 3.256, "a4": -27260, "c1": 18.18, "c2": -29810, "omega": 33060}}]}
    with pytest.raises(RuntimeError, match="V2-MTH-027"):
        tf.ThermoEngine(tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"Na+": na}}))


def test_validate_finds_a_region_on_a_method_of_a_substance():
    # CodeRabbit (PR #102) thought validate missed it: Database.validate runs the check of the methods of each substance too
    # loading a file refuses it already; a record changed after loading is checked by validate alone
    mineral = {"formula": "Ca", "class": "component", "aggregate_state": "crystal", "standard_state_properties": {"G": -1.0e6, "S": 40.0, "Cp": 25.0},
               "methods": [{"method": "ConstantCp"}]}
    db = tf.Database({"thermohub": "2.0.0-draft.1", "substances": {"A": mineral}})
    db.set("/substances/A/methods/0/region", {"t_max": 500.0})
    found = [d for d in db.validate().all() if d.code == "V2-MTH-027"]
    assert len(found) == 1 and "ConstantCp has a region" in found[0].message   # once: from the check of the methods
    # water equations and dielectric models keep their regions
    water = engine([HGK, dict(BP, region={"t_max": 343.15, "p_min": 1.0e8}), HK]).database()
    assert not [d for d in water.validate().all() if d.code == "V2-MTH-027"]


@pytest.mark.parametrize("region, word", [({"t_min": 400.0, "t_max": 300.0}, "t_min"), ({"p_min": 2.0e8, "p_max": 1.0e8}, "p_min")])
def test_an_inverted_region_is_reported(region, word):
    # a region whose minimum is at or above its maximum holds nowhere: its method would never be used, without a word
    db = engine([HGK, dict(BP, region=region), HK]).database()
    found = [d for d in db.validate().all() if d.code == "V2-APP-001"]
    assert found and found[0].path.endswith("/region") and word in found[0].message


def test_a_method_in_a_region_needs_one_of_its_kind_after_it():
    with pytest.raises(RuntimeError, match="V2-MTH-027"):
        engine([HGK, HK, dict(BP, region={"t_max": 343.15})])


def test_p_zero_picks_the_equation_whose_region_holds_its_own_saturation_pressure():
    """IAPWS-95 in p_min 1e9: at 500 K its saturation pressure (2.6 MPa) is outside, so P = 0 gives the HGK water, and the dielectric
    constant at the written pressure uses the same one."""
    combined = engine([{"method": "WaterIAPWS95", "region": {"p_min": 1.0e9}}, HGK, HK])
    hgk = engine([HGK, HK])
    assert combined.propertiesSolvent(500.0, 0.0, "H2O@").density.val == hgk.propertiesSolvent(500.0, 0.0, "H2O@").density.val
    assert eps(combined, 500.0, 0.0) == eps(hgk, 500.0, 0.0)


def test_zhang_duan_has_no_saturation_so_p_zero_goes_to_the_next_equation():
    combined = engine([{"method": "WaterZhangDuan05", "region": {"p_min": 1.0e8}}, {"method": "WaterIAPWS95"}, HK])
    iapws = engine([{"method": "WaterIAPWS95"}, HK])
    assert combined.propertiesSolvent(500.0, 0.0, "H2O@").density.val == iapws.propertiesSolvent(500.0, 0.0, "H2O@").density.val


def test_a_region_on_a_method_of_a_reaction_is_refused():
    doc = {"thermohub": "2.0.0-draft.1",
           "substances": {"A": {"formula": "A", "class": "component", "aggregate_state": "crystal", "standard_state_properties": {"G": 1.0, "H": 1.0, "S": 1.0, "Cp": 1.0, "V": 1e-5}, "methods": [{"method": "ConstantCp"}]},
                          "B": {"formula": "B", "class": "component", "aggregate_state": "crystal", "standard_state_properties": {"G": 1.0, "H": 1.0, "S": 1.0, "Cp": 1.0, "V": 1e-5}, "methods": [{"method": "ConstantCp"}]}},
           "reactions": {"r": {"equation": "A = B", "methods": [{"method": "LogKConstant", "region": {"t_max": 400.0}, "parameters": {"a0": 1.0}}]}}}
    assert "V2-MTH-027" in {d.code for d in tf.Database(doc).validate().all()}
