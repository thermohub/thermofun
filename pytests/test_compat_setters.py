# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The old setters edit the record: set then get, copy on write, clear errors, and the same results as the file."""

import glob
import json
import math
import os

import pytest
import thermofun as tf

HERE = os.path.dirname(os.path.abspath(__file__))
DATABASES = sorted(glob.glob(os.path.join(HERE, "*.json")))
GRID = [(298.15, 1e5), (423.15, 5e6), (573.15, 1e8)]
FIELDS = ["gibbs_energy", "enthalpy", "entropy", "heat_capacity_cp", "volume"]


def rebuilt_substance(s):
    """The same substance, written only with the old setters, in an order that old code can use."""
    out = tf.Substance()
    out.setName(s.name())
    out.setSymbol(s.symbol())
    out.setFormula(s.formula())
    out.setCharge(s.charge())
    out.setSubstanceClass(s.substanceClass())
    out.setAggregateState(s.aggregateState())
    out.setMolarMass(s.molarMass())
    out.setReferenceT(s.referenceT())
    out.setReferenceP(s.referenceP())
    out.setReactionSymbol(s.reactionSymbol())
    out.setThermoParameters(s.thermoParameters())       # before the methods that read them
    out.setThermoReferenceProperties(s.thermoReferenceProperties())
    if s.methodGenEOS():
        out.setMethodGenEoS(s.methodGenEOS())
    if s.method_T():
        out.setMethod_T(s.method_T())
    if s.method_P():
        out.setMethod_P(s.method_P())
    out.setThermoCalculationType(s.thermoCalculationType())
    return out


def rebuilt_reaction(r):
    out = tf.Reaction()
    out.setName(r.name())
    out.setSymbol(r.symbol())
    out.setEquation(r.equation())
    out.setReactants(r.reactants())
    out.setReferenceT(r.referenceT())
    out.setReferenceP(r.referenceP())
    out.setThermoReferenceProperties(r.thermoReferenceProperties())
    if r.methodGenEOS():
        out.setMethodGenEoS(r.methodGenEOS())
    if r.method_T():
        out.setMethod_T(r.method_T())
    if r.method_P():
        out.setMethod_P(r.method_P())
    out.setThermoParameters(r.thermoParameters())       # after the methods
    return out


def table_method(r):
    return r.method_T() == tf.MethodCorrT_Thrift.CTM_IKZ


def result_of(function, *args):
    try:
        return function(*args)
    except RuntimeError:
        return None


def same(a, b, what):
    if a is None or b is None:
        assert a is None and b is None, what
        return
    for field in dir(a):
        if field.startswith("_"):
            continue
        va, vb = getattr(a, field), getattr(b, field)
        if not hasattr(va, "val"):
            continue
        if math.isnan(va.val) and math.isnan(vb.val):
            continue
        assert vb.val == pytest.approx(va.val, rel=1e-9, abs=1e-9), f"{what} {field}"


@pytest.mark.parametrize("path", DATABASES, ids=os.path.basename)
def test_records_written_with_setters_give_the_results_of_the_file(path):
    from_file = tf.Database(path)
    written = tf.Database()
    written.conventions.water_triple_point_reference = from_file.conventions.water_triple_point_reference
    for e in from_file.getElements():
        written.addElement(e)
    for s in from_file.getSubstances():
        written.addSubstance(rebuilt_substance(s))
    for r in from_file.getReactions():
        written.addReaction(r if table_method(r) else rebuilt_reaction(r))

    engine_file, engine_written = tf.ThermoEngine(from_file), tf.ThermoEngine(written)
    for s in from_file.getSubstances():
        for T, P in GRID:
            same(result_of(engine_file.thermoPropertiesSubstance, T, P, s.symbol()),
                 result_of(engine_written.thermoPropertiesSubstance, T, P, s.symbol()), f"{s.symbol()} {T} {P}")
    for r in from_file.getReactions():
        for T, P in GRID:
            same(result_of(engine_file.thermoPropertiesReaction, T, P, r.symbol()),
                 result_of(engine_written.thermoPropertiesReaction, T, P, r.symbol()), f"{r.symbol()} {T} {P}")


def test_set_then_get_on_a_substance():
    s = tf.Substance()
    s.setName("n")
    s.setSymbol("sym")
    s.setFormula("CaCO3")
    s.setReactionSymbol("re")
    s.setCharge(2)
    s.setMolarMass(100.0)
    s.setReferenceT(300.0)
    s.setReferenceP(2e5)
    s.setLowerT(250.0)
    s.setUpperT(900.0)
    s.setLowerP(1.0)
    s.setUpperP(1e9)
    s.setSubstanceClass(tf.SubstanceClass.AQSOLUTE)
    s.setAggregateState(tf.AggregateState.AQUEOUS)
    assert (s.name(), s.symbol(), s.formula(), s.reactionSymbol(), s.charge()) == ("n", "sym", "CaCO3", "re", 2)
    assert s.molarMass() == pytest.approx(100.0)
    assert (s.referenceT(), s.referenceP()) == (300.0, 2e5)
    assert (s.lowerT(), s.upperT(), s.lowerP(), s.upperP()) == (250.0, 900.0, 1.0, 1e9)
    assert s.substanceClass() == tf.SubstanceClass.AQSOLUTE
    assert s.aggregateState() == tf.AggregateState.AQUEOUS

    reference = tf.ThermoPropertiesSubstance()
    reference.gibbs_energy = tf.ThermoScalar(-1e5)
    reference.volume = tf.ThermoScalar(3.0)
    s.setThermoReferenceProperties(reference)
    got = s.thermoReferenceProperties()
    assert got.gibbs_energy.val == pytest.approx(-1e5, rel=1e-14)
    assert got.volume.val == pytest.approx(3.0, rel=1e-14)    # J/bar in the old API

    s.setMethodGenEoS(tf.MethodGenEoS_Thrift.CTPM_HKF)
    parameters = tf.ThermoParametersSubstance()
    parameters.HKF_parameters = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0]
    s.setThermoParameters(parameters)
    assert s.thermoParameters().HKF_parameters == pytest.approx([1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0], rel=1e-13)
    assert s.methodGenEOS() == tf.MethodGenEoS_Thrift.CTPM_HKF


@pytest.mark.parametrize("first", ["parameters", "method"])
def test_the_order_of_parameters_and_method_does_not_count(first):
    parameters = tf.ThermoParametersSubstance()
    parameters.HKF_parameters = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0]
    s = tf.Substance()
    if first == "parameters":
        s.setThermoParameters(parameters)
        assert s.thermoParameters().HKF_parameters == [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0]  # no method reads it yet
    s.setMethodGenEoS(tf.MethodGenEoS_Thrift.CTPM_HKF)
    if first == "method":
        s.setThermoParameters(parameters)
    assert s.thermoParameters().HKF_parameters == pytest.approx([1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0], rel=1e-13)


def test_set_then_get_on_a_reaction():
    r = tf.Reaction()
    r.setName("n")
    r.setSymbol("sym")
    r.setEquation("A = B")
    r.setReactants({"A": -1.0, "B": 1.0})
    r.setReferenceT(300.0)
    r.setReferenceP(2e5)
    assert (r.name(), r.symbol(), r.equation(), r.reactants()) == ("n", "sym", "A = B", {"A": -1.0, "B": 1.0})
    assert (r.referenceT(), r.referenceP()) == (300.0, 2e5)
    r.setMethod_T(tf.MethodCorrT_Thrift.CTM_EK3)
    r.setMethod_P(tf.MethodCorrP_Thrift.CPM_VKE)
    parameters = tf.ThermoParametersReaction()
    parameters.reaction_V_fT_coeff = [1e-4, 2e-7, 1e-10, -4e-5, 1e-9]
    r.setThermoParameters(parameters)
    assert r.method_T() == tf.MethodCorrT_Thrift.CTM_EK3
    assert r.method_P() == tf.MethodCorrP_Thrift.CPM_VKE
    assert r.thermoParameters().reaction_V_fT_coeff == pytest.approx([1e-4, 2e-7, 1e-10, -4e-5, 1e-9], rel=1e-13)


def test_set_then_get_on_an_element():
    e = tf.Element()
    e.setName("Calcium")
    e.setSymbol("Ca")
    e.setMolarMass(40.078)
    e.setEntropy(41.6)
    assert (e.name(), e.symbol()) == ("Calcium", "Ca")
    assert e.molarMass() == pytest.approx(40.078, rel=1e-14)
    assert e.entropy() == 41.6


def test_a_change_of_method_keeps_the_values_of_the_record():
    source = {"symbol": "X", "name": "X", "formula": "Ca",
              "TPMethods": [{"method": {"0": "cp_ft_equation"}, "limitsTP": {"lowerT": 273.15, "upperT": 900.0},
                             "m_heat_capacity_ft_coeffs": {"values": [100.0, 0.1]}}],
              "sm_volume": {"values": [3.0]}, "sm_gibbs_energy": {"values": [-1e5]}}
    s = tf.Substance(json.dumps(source))
    s.setMethod_P(tf.MethodCorrP_Thrift.CPM_CON)
    assert s.method_P() == tf.MethodCorrP_Thrift.CPM_CON
    assert s.thermoParameters().Cp_coeff[0][:2] == pytest.approx([100.0, 0.1])
    assert s.thermoParameters().temperature_intervals[0] == [273.15, 900.0]
    assert s.thermoReferenceProperties().gibbs_energy.val == pytest.approx(-1e5)
    s.setMethod_P(tf.MethodCorrP_Thrift.CPM_NUL)
    assert s.method_P() == tf.MethodCorrP_Thrift.CPM_NUL
    assert s.thermoParameters().Cp_coeff[0][:2] == pytest.approx([100.0, 0.1])


def test_a_copy_is_not_changed():
    s = tf.Substance()
    s.setName("first")
    copy = tf.Substance(s)
    copy.setName("second")
    assert s.name() == "first"
    assert copy.name() == "second"


def test_a_code_without_a_method_gives_a_clear_error_and_changes_nothing():
    s = tf.Substance()
    s.setMethodGenEoS(tf.MethodGenEoS_Thrift.CTPM_HKF)
    with pytest.raises(ValueError, match="204 has no method for a substance"):
        s.setMethod_T(tf.MethodCorrT_Thrift.CTM_LGX)
    assert s.methodGenEOS() == tf.MethodGenEoS_Thrift.CTPM_HKF
    assert int(s.method_T()) == 0


def test_a_short_array_names_the_array_and_the_position():
    s = tf.Substance()
    s.setMethodGenEoS(tf.MethodGenEoS_Thrift.CTPM_HKF)
    parameters = tf.ThermoParametersSubstance()
    parameters.HKF_parameters = [1.0, 2.0]
    with pytest.raises(ValueError, match=r"eos_hkf_coeffs has 2 values: position 2 \(a3\)"):
        s.setThermoParameters(parameters)
    assert s.thermoParameters().HKF_parameters == []


def test_records_written_with_setters_run_on_the_method_objects():
    from_file = tf.Database(os.path.join(HERE, "test-aq17-gem-lma-thermofun.json"))
    written = tf.Database()
    written.conventions.water_triple_point_reference = from_file.conventions.water_triple_point_reference
    for s in from_file.getSubstances():
        written.addSubstance(rebuilt_substance(s))
    for r in from_file.getReactions():
        written.addReaction(r)
    engine_file, engine_written = tf.ThermoEngine(from_file), tf.ThermoEngine(written)
    on_objects = 0
    for s in from_file.getSubstances():
        steps = [step["method"] for step in engine_file.explain(373.15, 1e6, s.symbol())]
        assert [step["method"] for step in engine_written.explain(373.15, 1e6, s.symbol())] == steps, s.symbol()
        on_objects += bool(steps)
    assert on_objects > 100


def test_a_changed_parameter_changes_the_result():
    from_file = tf.Database(os.path.join(HERE, "test-aq17-gem-lma-thermofun.json"))
    symbol = "Ca+2"
    original = tf.ThermoEngine(from_file).thermoPropertiesSubstance(373.15, 1e6, symbol).gibbs_energy.val

    changed = tf.Database(from_file)
    substance = rebuilt_substance(from_file.getSubstance(symbol))
    parameters = substance.thermoParameters()
    parameters.HKF_parameters = [v * 1.5 if i == 4 else v for i, v in enumerate(parameters.HKF_parameters)]
    substance.setThermoParameters(parameters)
    changed.setSubstance(substance)
    assert tf.ThermoEngine(changed).thermoPropertiesSubstance(373.15, 1e6, symbol).gibbs_energy.val != pytest.approx(original, rel=1e-6)

    same_again = tf.Database(from_file)
    same_again.setSubstance(rebuilt_substance(from_file.getSubstance(symbol)))
    assert tf.ThermoEngine(same_again).thermoPropertiesSubstance(373.15, 1e6, symbol).gibbs_energy.val == pytest.approx(original, rel=1e-10)


def constant_substance(general=None):
    s = tf.Substance()
    s.setSymbol("X")
    s.setFormula("Ca")
    s.setAggregateState(tf.AggregateState.CRYSTAL)
    reference = tf.ThermoPropertiesSubstance()
    reference.gibbs_energy = tf.ThermoScalar(-1e5)
    reference.enthalpy = tf.ThermoScalar(-1.2e5)
    reference.entropy = tf.ThermoScalar(50.0)
    reference.heat_capacity_cp = tf.ThermoScalar(100.0)
    reference.volume = tf.ThermoScalar(3.0)
    s.setThermoReferenceProperties(reference)
    if general is not None:
        s.setMethodGenEoS(general)
    return s


def engine_with(*records):
    db = tf.Database()
    for r in records:
        db.addSubstance(r)
    return tf.ThermoEngine(db)


def test_constant_properties_give_the_reference_values_and_stop_other_methods():
    s = constant_substance(tf.MethodGenEoS_Thrift.CTPM_CON)
    s.setMethod_P(tf.MethodCorrP_Thrift.CPM_CON)
    assert s.methodGenEOS() == tf.MethodGenEoS_Thrift.CTPM_CON
    assert s.method_P() == tf.MethodCorrP_Thrift.CPM_CON
    p = engine_with(s).thermoPropertiesSubstance(500.0, 1e8, "X")
    assert p.gibbs_energy.val == pytest.approx(-1e5)
    assert p.enthalpy.val == pytest.approx(-1.2e5)
    assert p.volume.val == pytest.approx(3.0)


def test_a_record_without_methods_and_reaction_has_its_reference_values():
    p = engine_with(constant_substance()).thermoPropertiesSubstance(500.0, 1e7, "X")
    assert p.gibbs_energy.val == pytest.approx(-1e5)
    assert p.entropy.val == pytest.approx(50.0)


def test_cpm_off_is_the_ideal_gas_volume():
    s = constant_substance(tf.MethodGenEoS_Thrift.CTPM_CPT)
    s.setSubstanceClass(tf.SubstanceClass.GASFLUID)
    s.setAggregateState(tf.AggregateState.GAS)
    parameters = tf.ThermoParametersSubstance()
    parameters.Cp_coeff = [[100.0]]
    parameters.temperature_intervals = [[273.15, 2000.0]]
    s.setThermoParameters(parameters)
    s.setMethod_P(tf.MethodCorrP_Thrift.CPM_OFF)
    assert s.method_P() == tf.MethodCorrP_Thrift.CPM_OFF
    p = engine_with(s).thermoPropertiesSubstance(500.0, 1e6, "X")
    assert p.volume.val == pytest.approx(8.31451 * 500.0 / 10.0, rel=1e-3)   # J/bar at 10 bar


@pytest.mark.parametrize("second", [{"31": "water_eos_hgk84_reaktoro"}, None])
def test_hkf_with_water_means_vapour(second):
    """HKF with the flag of the gas, or with a water equation of state, on a record that is no solute."""
    methods = [{"method": {"2": "solute_hkf88_gems"}}]
    methods.append({"method": second} if second else {"method": {"8": "fug_critical_param"}})
    record = {"symbol": "H2O(g)", "formula": "H2O", "class_": {"1": "SC_GASFLUID"}, "aggregate_state": {"0": "AS_GAS"},
              "TPMethods": methods, "sm_gibbs_energy": {"values": [-228582.0]}, "sm_enthalpy": {"values": [-241816.0]}}
    s = tf.Substance(json.dumps(record))
    assert s.methodGenEOS() == tf.MethodGenEoS_Thrift.CTPM_HKF
    engine = engine_with(s)
    low, high = (engine.thermoPropertiesSubstance(T, 1e5, "H2O(g)") for T in (400.0, 800.0))
    assert high.enthalpy.val - low.enthalpy.val == pytest.approx(14400.0, rel=0.05)    # steam, not the reference values


def test_a_reaction_with_logk_coefficients_and_a_density_model_runs_the_model():
    record = {"symbol": "R", "equation": "A = B",
              "TPMethods": [{"method": {"0": "logk_fpt_function"}, "logk_ft_coeffs": {"values": [1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]}},
                            {"method": {"9": "logk_marshall_frank78"}, "dr_marshall_franck_coeffs": {"values": [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0]}}]}
    r = tf.Reaction(json.dumps(record))
    assert r.method_T() == tf.MethodCorrT_Thrift.CTM_DKR
    assert r.methodGenEOS() == tf.MethodGenEoS_Thrift.CTPM_REA
    assert r.thermoParameters().reaction_logK_fT_coeff[0] == 1.0
    assert r.thermoParameters().reaction_FM_coeff == pytest.approx([1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0])


def test_parse_functions_give_records_that_run():
    s = tf.parseSubstance(json.dumps({"symbol": "Y", "formula": "Ca", "sm_gibbs_energy": {"values": [-5.0]}}))
    assert engine_with(s).thermoPropertiesSubstance(300.0, 1e5, "Y").gibbs_energy.val == pytest.approx(-5.0)
