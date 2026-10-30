# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The aqueous solute methods alone, with no database and no engine, against the engine, the old classes and CHNOSZ.

import json
import math
import pathlib

import pytest
import thermofun as tf

HERE = pathlib.Path(__file__).resolve().parent
DEW = json.loads((HERE / "data" / "sources" / "chnosz-dew.json").read_text())
AQ17 = HERE / "test-aq17-gem-lma-thermofun.json"
CAL = 4.184
NAMES = ["a1", "a2", "a3", "a4", "c1", "c2", "omega"]
TO_SI = [CAL * 1e-5, CAL, CAL * 1e-5, CAL, CAL, CAL, CAL]  # from cal, bar to J, Pa


def record(symbol):
    return next(r for r in json.loads(AQ17.read_text())["substances"] if r["symbol"] == symbol)


def hkf_method(symbol, implementation):
    r = record(symbol)
    values = next(m for m in r["TPMethods"] if "eos_hkf_coeffs" in m)["eos_hkf_coeffs"]["values"]
    parameters = {n: v * f for n, v, f in zip(NAMES, values, TO_SI)}
    return tf.methods.substanceMethod("HKF88", implementation=implementation, parameters=parameters, symbol=symbol, charge=r["formula_charge"])


def reference_of(symbol):
    r = record(symbol)
    value = lambda k: r[k]["values"][0]
    return tf.methods.Reference(T=298.15, P=1e5, G=value("sm_gibbs_energy"), H=value("sm_enthalpy"), S=value("sm_entropy_abs"),
                                Cp=value("sm_heat_capacity_p"), V=value("sm_volume") * 1e-5)


def solvent_at(engine, T, P):
    s = tf.methods.SolventInputs()
    s.properties = engine.propertiesSolvent(T, P, "H2O@")
    s.electro = engine.electroPropertiesSolvent(T, P, "H2O@")
    return s


@pytest.mark.parametrize("symbol", ["Na+", "Cl-", "Ca+2", "Al(OH)3@"])
@pytest.mark.parametrize("T,P", [(298.15, 1e5), (423.15, 1e7), (573.15, 5e7)])
def test_hkf88_alone_gives_the_result_of_the_engine(symbol, T, P):
    engine = tf.ThermoEngine(str(AQ17))
    now = hkf_method(symbol, "reaktoro").evaluate(T, P, reference_of(symbol), solvent_at(engine, T, P))
    old = engine.thermoPropertiesSubstance(T, P, symbol)
    assert now.G.val == pytest.approx(old.gibbs_energy.val, rel=1e-12)
    assert now.H.val == pytest.approx(old.enthalpy.val, rel=1e-12)
    assert now.S.val == pytest.approx(old.entropy.val, rel=1e-12, abs=1e-9)
    assert now.Cp.val == pytest.approx(old.heat_capacity_cp.val, rel=1e-12, abs=1e-9)
    assert now.V.val * 1e5 == pytest.approx(old.volume.val, rel=1e-12, abs=1e-12)
    assert now.G.ddt == pytest.approx(old.gibbs_energy.ddt, rel=1e-10)
    assert now.G.ddp * 1e5 == pytest.approx(old.gibbs_energy.ddp * 1e5, rel=1e-10)


def test_the_derivative_of_hkf88_by_each_parameter_agrees_with_finite_differences():
    symbol, T, P = "Na+", 423.15, 1e7
    engine = tf.ThermoEngine(str(AQ17))
    solvent, reference = solvent_at(engine, T, P), reference_of(symbol)
    method = hkf_method(symbol, "reaktoro")
    values = method.parameterValues()
    for i in range(len(values)):
        step = abs(values[i]) * 1e-5
        high = method.withParameter(i, values[i] + step).evaluate(T, P, reference, solvent).G.val
        low = method.withParameter(i, values[i] - step).evaluate(T, P, reference, solvent).G.val
        exact = method.parameterDerivative(i, T, P, reference, solvent)["G"]
        assert exact == pytest.approx((high - low) / (2 * step), rel=1e-5), NAMES[i]


@pytest.mark.parametrize("symbol", ["Na+", "Cl-", "Ca+2"])
@pytest.mark.parametrize("T,P", [(298.15, 1e5), (423.15, 1e7), (573.15, 5e7)])
def test_the_implementations_of_hkf88_agree(symbol, T, P):
    engine = tf.ThermoEngine(str(AQ17))
    solvent, reference = solvent_at(engine, T, P), reference_of(symbol)
    gems = hkf_method(symbol, "gems").evaluate(T, P, reference, solvent)
    reaktoro = hkf_method(symbol, "reaktoro").evaluate(T, P, reference, solvent)
    assert gems.G.val == pytest.approx(reaktoro.G.val, abs=30.0)  # J/mol
    assert gems.V.val == pytest.approx(reaktoro.V.val, abs=1e-6)


def test_hkf88_says_what_is_missing():
    method = hkf_method("Na+", "reaktoro")
    with pytest.raises(RuntimeError, match="needs the state of the solvent"):
        method.evaluate(298.15, 1e5, reference_of("Na+"))
    empty = tf.methods.substanceMethod("HKF88", symbol="X", charge=1)
    engine = tf.ThermoEngine(str(AQ17))
    with pytest.raises(RuntimeError, match="not defined"):
        empty.evaluate(298.15, 1e5, reference_of("Na+"), solvent_at(engine, 298.15, 1e5))


@pytest.mark.parametrize("row", DEW["species"], ids=lambda r: f"{r['name']}-{int(r['T'])}C-{int(r['P'])}bar")
def test_hkf88_in_the_water_of_dew_agrees_with_chnosz(row):
    T, P = row["T"] + 273.15, row["P"] * 1e5
    o = DEW["obigt"][row["name"]]
    reference = tf.methods.Reference(T=298.15, P=1e5, G=o["G"] * CAL, H=o["H"] * CAL, S=o["S"] * CAL, Cp=o["Cp"] * CAL, V=o["V"] * 1e-6)
    water = tf.methods.solventMethod("WaterZhangDuan05").properties(T, P)
    solvent = tf.methods.SolventInputs()
    solvent.properties = water
    solvent.electro = tf.methods.dielectricMethod("SverjenskyEtAl14").properties(T, P, water)
    result = hkf_method(row["name"], "reaktoro").evaluate(T, P, reference, solvent)
    assert result.G.val == pytest.approx(row["G"], abs=25.0)  # J/mol of about 300000


def density_substance():
    s = tf.Substance()
    s.setSymbol("OH-")
    s.setReferenceT(298.15)
    s.setReferenceP(1e5)
    parameters = tf.ThermoParametersSubstance()
    parameters.solute_holland_powell98_coeff = [1e-3]
    s.setThermoParameters(parameters)
    reference = tf.ThermoPropertiesSubstance()
    reference.gibbs_energy.val, reference.enthalpy.val, reference.entropy.val = -157220.0, -230020.0, -10.71
    reference.heat_capacity_cp.val, reference.volume.val = -137.2, -0.418
    s.setThermoReferenceProperties(reference)
    return s


DENSITY_REFERENCE = dict(T=298.15, P=1e5, G=-157220.0, H=-230020.0, S=-10.71, Cp=-137.2, V=-0.418e-5)


@pytest.mark.parametrize("T,P", [(298.15, 1e5), (373.15, 1e7), (573.15, 5e7)])
@pytest.mark.parametrize("old_class,method_id,parameters", [
    ("SoluteHollandPowell98", "SoluteHP98", {"b": 1.0}),   # J/(mol K2): 1e-3 kJ/(mol K2) in the old class
    ("SoluteAnderson91", "AndersonEtAl91", {}),
])
def test_the_density_models_alone_give_the_result_of_the_old_classes(old_class, method_id, parameters, T, P):
    engine = tf.ThermoEngine(str(AQ17))
    wp, wpr = engine.propertiesSolvent(T, P, "H2O@"), engine.propertiesSolvent(298.15, 1e5, "H2O@")
    old = getattr(tf, old_class)(density_substance()).thermoProperties(T, P, wpr, wp)
    solvent = tf.methods.SolventInputs()
    solvent.properties = wp
    solvent.properties_at_reference = wpr
    now = tf.methods.substanceMethod(method_id, parameters=parameters, symbol="OH-").evaluate(T, P, tf.methods.Reference(**DENSITY_REFERENCE), solvent)
    assert now.G.val == pytest.approx(old.gibbs_energy.val, rel=1e-12)
    assert now.H.val == pytest.approx(old.enthalpy.val, rel=1e-12)
    assert now.S.val == pytest.approx(old.entropy.val, rel=1e-11)
    assert now.Cp.val == pytest.approx(old.heat_capacity_cp.val, rel=1e-11)
    assert now.V.val * 1e5 == pytest.approx(old.volume.val, rel=1e-12)
    assert math.isfinite(now.G.ddt)


def test_the_hydrogen_ion_is_zero_whatever_its_symbol():
    doc = json.loads(AQ17.read_text())
    for r in doc["substances"]:
        if r["symbol"] == "H+":
            r["symbol"] = "Hydrogen"
    engine = tf.ThermoEngine(json.dumps(doc))
    p = engine.thermoPropertiesSubstance(373.15, 1e7, "Hydrogen")
    assert p.gibbs_energy.val == 0.0 and p.entropy.val == 0.0 and p.volume.val == 0.0
