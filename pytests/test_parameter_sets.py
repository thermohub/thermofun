# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The parameters of a method can come from a parameter set of the file: the same results as inline, and an inline value goes first.

import json
import math

import pytest
import thermofun as tf

T, P = 473.15, 5e7


def with_set():
    """Na+ takes its HKF parameters from a set."""
    doc = json.loads(tf.Database("pytests/data/v2/hkf.json").toJson(readable=False))
    method = next(m for m in doc["substances"]["Na+"]["methods"] if m["method"] == "HKF88")
    parameters = method.pop("parameters")
    method["parameter_set"] = "Na_hkf"
    doc["parameter_sets"] = {"Na_hkf": {"method": "HKF88", "parameters": parameters}}
    return doc, method, parameters


def gibbs(doc):
    return tf.ThermoEngine(tf.Database(json.dumps(doc))).thermoPropertiesSubstance(T, P, "Na+").gibbs_energy.val


def test_a_set_gives_the_results_of_inline_values():
    doc, _, _ = with_set()
    inline = json.loads(tf.Database("pytests/data/v2/hkf.json").toJson(readable=False))
    assert gibbs(doc) == gibbs(inline)


def test_an_inline_value_goes_before_the_set():
    doc, method, parameters = with_set()
    plain = gibbs(doc)
    number = parameters["omega"]
    method["parameters"] = {"omega": number * 1.5}
    assert gibbs(doc) != plain
    whole = json.loads(tf.Database("pytests/data/v2/hkf.json").toJson(readable=False))
    next(m for m in whole["substances"]["Na+"]["methods"] if m["method"] == "HKF88")["parameters"]["omega"] = number * 1.5
    assert gibbs(doc) == gibbs(whole)


def test_a_handle_on_a_value_of_a_set_changes_only_this_record():
    doc, _, parameters = with_set()
    second = json.loads(json.dumps(doc["substances"]["Na+"]))
    second["name"] = "copy"
    doc["substances"]["K2"] = second
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    before = engine.thermoPropertiesSubstance(T, P, "K2").gibbs_energy.val
    omega = engine.parameter("Na+", "HKF88", "omega")
    start = omega.value()
    assert start != 0.0
    omega.setValue(start * 1.5)
    assert engine.thermoPropertiesSubstance(T, P, "K2").gibbs_energy.val == before
    assert engine.thermoPropertiesSubstance(T, P, "Na+").gibbs_energy.val != before


def test_the_derivatives_and_the_errors_read_the_set():
    doc, _, parameters = with_set()
    doc["parameter_sets"]["Na_hkf"]["parameters"]["c1"] = {"value": parameters["c1"], "uncertainty": 1.0}
    engine = tf.ThermoEngine(tf.Database(json.dumps(doc)))
    d = engine.parameterSensitivities(T, P, "Na+", [("Na+", "HKF88", "c1")])["Cp"]["Na+/HKF88/c1"]
    assert d != 0.0
    engine.preferences.propagate_parameter_errors = True
    assert engine.thermoPropertiesSubstance(T, P, "Na+").heat_capacity_cp.err == pytest.approx(abs(d) * 1.0, rel=1e-10)


# The constants of a method are data: a file next to the code, a set of a file, or the record
def water_with_dielectric():
    return json.loads(tf.Database("pytests/data/legacy-synthetic/s27-water_diel_sverj14.json").toJson(readable=False))


def epsilon(doc, T=473.15, P=1e8):
    return tf.ThermoEngine(tf.Database(json.dumps(doc))).electroPropertiesSolvent(T, P, solvent_symbol(doc)).epsilon.val


def solvent_symbol(doc):
    return next(s for s, r in doc["substances"].items() if r.get("class") == "water_solvent")


def dielectric_of(doc):
    return next(m for m in doc["substances"][solvent_symbol(doc)]["methods"] if m["method"] == "SverjenskyEtAl14")


def test_the_built_in_constants_are_the_values_of_the_paper():
    doc = water_with_dielectric()
    plain = epsilon(doc)
    t, rho = 473.15 - 273.15, tf.ThermoEngine(tf.Database(json.dumps(doc))).propertiesSolvent(473.15, 1e8, solvent_symbol(doc)).density.val / 1000.0
    a, b = (-1.576377e-03, 6.810288e-02, 7.548755e-01), (-8.016651e-05, -6.871618e-02, 4.747973)
    assert plain == pytest.approx(math.exp(b[0] * t + b[1] * math.sqrt(t) + b[2]) * rho ** (a[0] * t + a[1] * math.sqrt(t) + a[2]), rel=1e-10)


def test_a_record_and_a_set_can_replace_a_built_in_constant():
    doc = water_with_dielectric()
    plain = epsilon(doc)
    method = dielectric_of(doc)
    method["parameters"] = {**method.get("parameters", {}), "b3": 5.0}
    inline = epsilon(doc)
    assert inline != plain
    doc = water_with_dielectric()
    method = dielectric_of(doc)
    method["parameter_set"] = "mine"
    doc["parameter_sets"] = {"mine": {"method": "SverjenskyEtAl14", "parameters": {"b3": 5.0}}}
    assert epsilon(doc) == inline


def test_each_implementation_of_hkf_has_its_own_constants():
    doc, method, parameters = with_set()
    doc["substances"]["Na+"]["methods"] = [dict(m, parameters=parameters) if m["method"] == "HKF88" else m for m in doc["substances"]["Na+"]["methods"]]
    for m in doc["substances"]["Na+"]["methods"]:
        m.pop("parameter_set", None)
    doc.pop("parameter_sets")
    values = {}
    for implementation in ("reaktoro", "gems"):
        next(m for m in doc["substances"]["Na+"]["methods"] if m["method"] == "HKF88")["implementation"] = implementation
        values[implementation] = gibbs(doc)
    # the digits of Shock et al. against those of SUPCRT: close, and not the same
    assert values["reaktoro"] != values["gems"]
    assert values["reaktoro"] == pytest.approx(values["gems"], rel=1e-3)


def test_a_record_can_replace_a_constant_of_hkf():
    doc, method, parameters = with_set()
    plain = gibbs(doc)
    method["parameters"] = {"eta": 1.7e5}
    assert gibbs(doc) != plain


def test_a_constant_is_not_a_parameter_to_fit():
    names = [h.key()[2] for h in tf.ThermoEngine("pytests/data/v2/hkf.json").parameters("Na+")]
    assert "a1" in names and "eta" not in names


def test_each_built_in_set_says_what_it_holds_and_where_it_comes_from():
    sets = [(spec["id"], s) for spec in tf.methods.catalog() for s in spec["constant_sets"]]
    assert len(sets) >= 9
    for method, s in sets:
        assert len(s["description"]) > 40, method
        assert s["reference"], method


def test_the_page_of_a_method_gives_its_constants_in_one_place():
    from thermofun import docs
    spec = next(s for s in tf.methods.catalog() if s["id"] == "SverjenskyEtAl14")
    page = docs.method_page(spec, None)
    assert "## Built-in constants" in page and spec["constant_sets"][0]["description"] in page
    assert "| a1 |" not in page


def test_each_file_of_constants_describes_each_set():
    import pathlib
    import re
    folder = pathlib.Path(__file__).resolve().parent.parent / "ThermoFun" / "Methods" / "ParameterSets"
    files = list(folder.glob("*.yaml"))
    assert files
    for file in files:
        text = file.read_text()
        sets = len(re.findall(r"^  \w+:$", text, re.M))
        assert sets and len(re.findall(r'^    description: ".{40,}"$', text, re.M)) == sets, file.name
        assert len(re.findall(r'^    reference: ".+"$', text, re.M)) == sets, file.name


def water_of(doc):
    return next(m for m in doc["substances"][solvent_symbol(doc)]["methods"] if m["method"] == "WaterHGK84")


def gems_water():
    doc = water_with_dielectric()
    water_of(doc)["implementation"] = "gems"
    for m in doc["substances"][solvent_symbol(doc)]["methods"]:
        if m["method"] == "SverjenskyEtAl14":
            m["method"] = "JohnsonNorton91"
            m["implementation"] = "gems"
    return doc


def gibbs_of_water(doc, T=473.15, P=1e8):
    return tf.ThermoEngine(tf.Database(json.dumps(doc))).thermoPropertiesSubstance(T, P, solvent_symbol(doc)).gibbs_energy.val


def test_a_record_can_replace_a_constant_of_the_gems_water_and_only_for_itself():
    doc = gems_water()
    plain = gibbs_of_water(doc)
    changed = json.loads(json.dumps(doc))
    water_of(changed)["parameters"] = {**water_of(changed).get("parameters", {}), "nc_g_1": -0.53062968529023e3 * 1.001}
    assert gibbs_of_water(changed) != plain
    assert gibbs_of_water(doc) == plain


def test_a_record_can_replace_a_constant_of_johnson_norton_in_both_implementations():
    for implementation in ("gems", "reaktoro"):
        doc = gems_water()
        method = next(m for m in doc["substances"][solvent_symbol(doc)]["methods"] if m["method"] == "JohnsonNorton91")
        method["implementation"] = implementation
        plain = epsilon(doc)
        method["parameters"] = {**method.get("parameters", {}), "a2": 0.2128462733e+03 * 1.01}
        assert epsilon(doc) != plain, implementation
        assert epsilon(doc) == pytest.approx(plain, rel=0.1), implementation


def test_a_constant_of_another_implementation_is_an_error():
    doc = water_with_dielectric()
    water_of(doc)["implementation"] = "reaktoro"
    water_of(doc)["parameters"] = {**water_of(doc).get("parameters", {}), "nc_g_1": 1.0}
    with pytest.raises(Exception, match="does not belong"):
        gibbs_of_water(doc)


@pytest.mark.parametrize("method_id", ["WaterHGK84", "WaterIAPWS95"])
def test_a_record_can_replace_the_saturation_pressure_and_only_for_itself(method_id):
    doc = water_with_dielectric()
    water = water_of(doc)
    water["method"] = method_id
    water["implementation"] = "reaktoro"
    plain = gibbs_of_water(doc, P=0)
    changed = json.loads(json.dumps(doc))
    water_of_changed = next(m for m in changed["substances"][solvent_symbol(changed)]["methods"] if m["method"] == method_id)
    water_of_changed["parameters"] = {**water_of_changed.get("parameters", {}), "sat_a1": -7.85951783 * 1.01}
    assert gibbs_of_water(changed, P=0) != plain
    assert gibbs_of_water(doc, P=0) == plain


def test_a_record_can_replace_the_critical_point_of_the_water_model():
    doc = water_with_dielectric()
    plain = gibbs_of_water(doc)
    water_of(doc)["implementation"] = "reaktoro"
    water_of(doc)["parameters"] = {**water_of(doc).get("parameters", {}), "M": 0.018015268 * 1.01}
    assert gibbs_of_water(doc) != plain


def test_a_record_can_replace_a_scalar_of_the_gems_water():
    def water_properties(doc):
        return tf.ThermoEngine(tf.Database(json.dumps(doc))).propertiesSolvent(473.15, 1e7, solvent_symbol(doc))
    doc = gems_water()
    plain = water_properties(doc)
    water_of(doc)["parameters"] = {**water_of(doc).get("parameters", {}), "surf_stref": 0.2358 * 2}
    changed = water_properties(doc)
    assert changed.surface_tension == pytest.approx(2 * plain.surface_tension, rel=1e-12)
    assert changed.density.val == plain.density.val
    water_of(doc)["parameters"] = {"trans_ustar": 1.0e-6 * 2}
    assert water_properties(doc).dynamic_viscosity == pytest.approx(2 * plain.dynamic_viscosity, rel=1e-12)


def test_a_record_can_replace_a_constant_of_a_method_of_a_reaction():
    doc = json.loads(tf.Database("pytests/Reactions/Co_Cu_aq-thermofun.json").toJson(readable=False))
    method = next(m for m in doc["reactions"]["CoCl4-2"]["methods"] if m["method"] == "RyzhenkoBryzgalin")
    logK = lambda: tf.ThermoEngine(tf.Database(json.dumps(doc))).thermoPropertiesReaction(673.15, 800e5, "CoCl4-2").log_equilibrium_constant.val
    plain = logK()
    method["parameters"] = {**method.get("parameters", {}), "zzH2O": 1.0107 * 1.01}
    assert logK() != plain


def test_the_dielectric_constant_of_gems_reads_the_water_of_the_record():
    doc = gems_water()
    plain = epsilon(doc)
    water_of(doc)["parameters"] = {**water_of(doc).get("parameters", {}), "nc_g_1": -0.53062968529023e3 * 1.001}
    assert epsilon(doc) != plain
    assert epsilon(doc) == pytest.approx(plain, rel=0.1)


def test_both_implementations_of_johnson_norton_agree_on_the_same_water():
    values = {}
    for implementation in ("gems", "reaktoro"):
        doc = gems_water()
        water_of(doc)["implementation"] = "reaktoro"
        next(m for m in doc["substances"][solvent_symbol(doc)]["methods"] if m["method"] == "JohnsonNorton91")["implementation"] = implementation
        values[implementation] = epsilon(doc)
    assert values["gems"] == pytest.approx(values["reaktoro"], rel=1e-12)
