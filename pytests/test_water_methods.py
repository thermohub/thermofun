# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The water methods alone, with no database and no engine, against the engine and against independent codes:
# jjgomera/iapws (IAPWS-95, R8-97; make_iapws_check.py) and CHNOSZ 2.2.0 (the model DEW).

import json
import math
import pathlib

import pytest
import thermofun as tf

HERE = pathlib.Path(__file__).resolve().parent
IAPWS = json.loads((HERE / "data" / "sources" / "iapws-check.json").read_text())
DEW = json.loads((HERE / "data" / "sources" / "chnosz-dew.json").read_text())["water"]
CAL = 4.184
DEW_COEFFICIENTS = [2.6880734e-09, 6.3163061e-07, -0.019372355, -16.945093, -55769.287]


def state_of(row):
    return 1 if row["phase"] == "gas" else 0


def iapws_id(row):
    return f"{row['T']}K-{row['P']:g}Pa"


@pytest.mark.parametrize("row", IAPWS["rows"], ids=iapws_id)
def test_iapws95_agrees_with_an_independent_code(row):
    water = tf.methods.solventMethod("WaterIAPWS95")
    ps = water.properties(row["T"], row["P"], state_of(row))
    assert ps.density.val == pytest.approx(row["rho"], rel=1e-6)


@pytest.mark.parametrize("row", IAPWS["rows"], ids=iapws_id)
def test_the_energies_of_iapws95_agree_with_an_independent_code(row):
    water = tf.methods.solventMethod("WaterIAPWS95")
    ref = IAPWS["reference"]
    p = water.substanceProperties(row["T"], row["P"], state_of(row))
    r = water.substanceProperties(ref["T"], ref["P"], 0)
    assert p.heat_capacity_cp.val == pytest.approx(row["cp"], rel=1e-5)
    assert p.entropy.val - r.entropy.val == pytest.approx(row["s"] - ref["s"], abs=1e-3)
    assert p.enthalpy.val - r.enthalpy.val == pytest.approx(row["h"] - ref["h"], abs=0.5)


@pytest.mark.parametrize("row", [r for r in IAPWS["rows"] if "epsilon" in r], ids=iapws_id)
def test_fernandez97_agrees_with_an_independent_code(row):
    ps = tf.methods.solventMethod("WaterIAPWS95").properties(row["T"], row["P"], state_of(row))
    electro = tf.methods.dielectricMethod("FernandezEtAl97").properties(row["T"], row["P"], ps, state_of(row))
    assert electro.epsilon.val == pytest.approx(row["epsilon"], rel=1e-6)


@pytest.mark.parametrize("T,P", [(298.15, 1e5), (473.15, 1e7), (673.15, 5e7), (873.15, 5e8)])
def test_the_implementations_of_hgk84_agree(T, P):
    gems = tf.methods.solventMethod("WaterHGK84", implementation="gems")
    reaktoro = tf.methods.solventMethod("WaterHGK84", implementation="reaktoro")
    assert gems.properties(T, P).density.val == pytest.approx(reaktoro.properties(T, P).density.val, rel=2e-3)
    g = gems.substanceProperties(T, P)
    r = reaktoro.substanceProperties(T, P)
    assert g.gibbs_energy.val == pytest.approx(r.gibbs_energy.val, rel=2e-3)


@pytest.mark.parametrize("T,P", [(298.15, 1e5), (473.15, 1e7), (673.15, 5e7)])
def test_the_implementations_of_johnson_norton91_agree(T, P):
    solvent = tf.methods.solventMethod("WaterHGK84", implementation="reaktoro").properties(T, P)
    gems = tf.methods.dielectricMethod("JohnsonNorton91", implementation="gems").properties(T, P, solvent)
    reaktoro = tf.methods.dielectricMethod("JohnsonNorton91", implementation="reaktoro").properties(T, P, solvent)
    assert gems.epsilon.val == pytest.approx(reaktoro.epsilon.val, rel=2e-3)


@pytest.mark.parametrize("path,symbol,implementation,method_id", [
    ("Substances/Solvent/water-thermofun.json", "H2O@", "gems", "WaterHGK84"),
    ("Substances/Solvent/water-thermofun.json", "H2O@reak", "reaktoro", "WaterIAPWS95"),
])
def test_a_method_alone_gives_the_result_of_the_engine(path, symbol, implementation, method_id):
    engine = tf.ThermoEngine(str(HERE / path))
    water = tf.methods.solventMethod(method_id, implementation=implementation)
    for T, P in [(298.15, 1e5), (473.15, 1e7), (673.15, 5e7)]:
        alone = water.properties(T, P)
        ps = engine.propertiesSolvent(T, P, symbol, 0)
        assert alone.density.val == ps.density.val
        assert alone.densityT.val == ps.densityT.val
        assert alone.densityP.ddp == ps.densityP.ddp


def dew_row_id(row):
    return f"{int(row['T_C'])}C-{int(row['P_bar'])}bar"


@pytest.mark.parametrize("row", DEW, ids=dew_row_id)
def test_zhang_duan_and_sverjensky_agree_with_chnosz(row):
    T, P = row["T_C"] + 273.15, row["P_bar"] * 1e5
    water = tf.methods.solventMethod("WaterZhangDuan05")
    ps = water.properties(T, P)
    assert ps.density.val == pytest.approx(row["rho"], rel=1e-5)
    assert tf.methods.dielectricMethod("SverjenskyEtAl14").properties(T, P, ps).epsilon.val == pytest.approx(row["epsilon"], rel=1e-5)
    assert water.substanceProperties(T, P).volume.val * 10 == pytest.approx(row["V"], rel=1e-5)  # J/bar to cm3/mol


def chnosz_integral(water, T, P):
    """The integral of V dP as CHNOSZ calculates it: a sum of rectangles, the spacing of 20 bar or (P - 1000) / 500,
    with both ends of the range counted"""
    spacing = 20.0 if (P - 1000.0) / 500.0 < 20 else (P - 1000.0) / 500.0
    total, p = 0.0, 1000.0
    while p <= P + 1e-9:
        total += water.substanceProperties(T, p * 1e5).volume.val * spacing  # J/bar x bar
        p += spacing
    return total


def dew_water():
    return tf.methods.solventMethod("WaterZhangDuan05", parameters={"dew_gibbs": 1.0})


@pytest.mark.parametrize("row", [DEW[0], DEW[4], DEW[7], DEW[12]], ids=dew_row_id)
def test_the_gibbs_energy_of_water_follows_the_dew_equations(row):
    water = dew_water()
    T, P = row["T_C"] + 273.15, row["P_bar"]
    g1kb = sum(c * row["T_C"] ** (4 - i) for i, c in enumerate(DEW_COEFFICIENTS)) * CAL
    ours = water.substanceProperties(T, P * 1e5).gibbs_energy.val
    # with the sum of rectangles of CHNOSZ in place of the exact integral, the two agree to 3e-4 of G
    assert g1kb + chnosz_integral(water, T, P) == pytest.approx(row["G"], rel=3e-4)
    # the exact integral is smaller than the sum by about half a rectangle at each end
    assert ours < row["G"] and row["G"] - ours < 700.0


@pytest.mark.parametrize("row", DEW, ids=dew_row_id)
def test_the_gibbs_energy_of_water_is_consistent(row):
    water = dew_water()
    T, P = row["T_C"] + 273.15, row["P_bar"] * 1e5
    p = water.substanceProperties(T, P)
    assert p.gibbs_energy.ddp * 1e5 == pytest.approx(p.volume.val, rel=1e-6)  # dG/dP = V, in J/bar
    h = 0.01
    lo, hi = (water.substanceProperties(T + d, P).gibbs_energy.val for d in (-h, h))
    assert (hi - lo) / (2 * h) == pytest.approx(p.gibbs_energy.ddt, rel=1e-6)


def test_the_gibbs_energy_of_dew_needs_one_kilobar_and_is_off_by_default():
    with pytest.raises(RuntimeError, match="lower than the minimum"):  # the model itself starts at 1 kbar
        dew_water().substanceProperties(700.0, 5e7)
    p = tf.methods.solventMethod("WaterZhangDuan05").substanceProperties(700.0, 5e8)
    assert p.volume.val > 0
    assert p.gibbs_energy.sta[0] == tf.Status.notdefined


@pytest.mark.parametrize("T,P,inside", [(2000.0, 1e9, True), (2000.5, 1e9, False), (1000.0, 3e10, True), (1000.0, 3.1e10, False)])
def test_the_limits_of_zhang_duan(T, P, inside):
    water = tf.methods.solventMethod("WaterZhangDuan05")
    if inside:
        assert water.properties(T, P).density.val > 0
    else:
        with pytest.raises(RuntimeError, match="Out of"):
            water.properties(T, P)


def test_a_legacy_water_eos_of_gems_that_does_not_exist_runs_iapws95():
    # water_eos_iapws95_gems was ignored; there is no GEMS code, so it is the Reaktoro implementation of the model
    path = HERE / "data" / "legacy-synthetic" / "s30-water_eos_iapws95_gems.json"
    engine = tf.ThermoEngine(str(path))
    ps = engine.propertiesSolvent(298.15, 1e5, "H2O@", 0)
    assert ps.density.val == tf.methods.solventMethod("WaterIAPWS95").properties(298.15, 1e5).density.val
    assert not math.isnan(engine.thermoPropertiesSubstance(298.15, 1e5, "Al+3").gibbs_energy.val)


# The second derivatives of the density by T of HGK (Reaktoro) had two errors in the third derivative of the Helmholtz function by T, D, D.
@pytest.mark.parametrize("T, P", [(298.15, 1e5), (473.15, 2e6), (673.15, 1e8), (1073.15, 5e8), (298.15, 1e9)])
def test_the_second_derivatives_of_the_density_of_hgk_are_those_of_its_first_derivatives(T, P):
    db = "pytests/test-aq17-gem-lma-thermofun.json"
    e = tf.ThermoEngine(db)
    q = e.propertiesSolvent(T, P, "H2O@")
    assert q.densityTT.val == pytest.approx(q.densityT.ddt, rel=1e-8)
    assert q.dAldT.val == pytest.approx(q.Alpha.ddt, rel=1e-8)
    assert q.densityTP.val == pytest.approx(q.densityT.ddp, rel=1e-8)


@pytest.mark.parametrize("symbol", ["Ca+2", "Cl-", "Al(OH)2+"])
@pytest.mark.parametrize("T, P", [(298.15, 1e5), (473.15, 2e6), (673.15, 1e8)])
def test_the_heat_capacity_of_a_solute_is_the_same_with_both_implementations_of_hgk(symbol, T, P):
    def cp(implementation):
        e = tf.ThermoEngine("pytests/test-aq17-gem-lma-thermofun.json")
        p = tf.EnginePreferences()
        p.method_implementation = {"WaterHGK84": implementation, "HKF88": implementation}
        e.setPreferences(p)
        return e.thermoPropertiesSubstance(T, P, symbol).heat_capacity_cp.val
    assert cp("reaktoro") == pytest.approx(cp("gems"), abs=2e-3)
