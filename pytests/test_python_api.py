"""The Python API of ThermoFun: enumerations, models, units, records, database, engine and batch."""
import json
import os

import pytest
import thermofun as tf

needs_autodiff = pytest.mark.skipif(not getattr(tf, "with_autodiff", True), reason="built with -DTFUN_USE_AUTODIFF=OFF: the derivatives are 0")

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
    s.setMethod_T(tf.MethodCorrT_Thrift.CTM_LGX)
    s.setMethod_P(tf.MethodCorrP_Thrift.CPM_NUL)
    assert s.substanceClass() == tf.SubstanceClass.AQSOLUTE
    assert s.aggregateState() == tf.AggregateState.CRYSTAL
    assert s.thermoCalculationType() == tf.SubstanceThermoCalculationType.REACDC
    assert s.methodGenEOS() == tf.MethodGenEoS_Thrift.CTPM_CON
    assert s.method_T() == tf.MethodCorrT_Thrift.CTM_LGX
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

def test_substance_models_agree_with_the_engine(database, engine):
    cal = database.getSubstance("Cal")
    T, P = 450.0, 1e5
    ref = engine.thermoPropertiesSubstance(T, P, "Cal")
    cp = tf.EmpiricalCpIntegration(cal).thermoProperties(T, P)
    assert cp.gibbs_energy.val == pytest.approx(ref.gibbs_energy.val, rel=1e-12)
    assert cp.gibbs_energy.ddt == pytest.approx(ref.gibbs_energy.ddt, rel=1e-12)
    assert cp.heat_capacity_cp.val == pytest.approx(ref.heat_capacity_cp.val, rel=1e-12)
    # the volume model corrects the properties for the pressure
    out = tf.ConMolVol(cal).thermoProperties(T, 5e7, cp)
    v = cal.thermoReferenceProperties().volume.val
    assert out.gibbs_energy.val == pytest.approx(cp.gibbs_energy.val + v * (5e7 - 1e5) / 1e5, rel=1e-10)
    # all the models are constructed from a Substance or a Reaction
    for name in ("ThermoModelsSubstance", "WaterIdealGasWoolley", "EntropyCpIntegration", "HPLandau", "MinBerman88",
                 "MinBMGottschalk", "MinMurnaghanEOSHP98", "GasCORK", "GasPRSV", "GasCGF", "GasSRK", "GasPR78", "GasSTP",
                 "IdealGasLawVol", "SoluteHKFgems", "SoluteHKFreaktoro", "SoluteHollandPowell98", "SoluteAnderson91",
                 "SoluteAkinfievDiamondEOS", "WaterHGK", "WaterHGKreaktoro", "WaterWP95reaktoro", "WaterZhangDuan2005",
                 "WaterJNreaktoro", "WaterJNgems", "WaterElectroSverjensky2014", "WaterElectroFernandez1997"):
        assert hasattr(tf, name), name
        getattr(tf, name)(cal)
    for name in ("ReactionDolejsManning10", "ReactionFrantzMarshall", "ReactionRyzhenkoBryzgalin", "Reaction_LogK_fT",
                 "Reaction_Vol_fT", "ReactionFromReactantsProperties"):
        getattr(tf, name)(database.getReactions()[0])


@needs_autodiff


def test_solvent_models(database):
    water = database.getSubstance("H2O@")
    for model in (tf.WaterHGKreaktoro(water), tf.WaterWP95reaktoro(water), tf.WaterHGK(water)):
        ps = model.propertiesSolvent(298.15, 1e5, 0)
        assert ps.density.val == pytest.approx(997.0, abs=1.5)
        assert ps.density.ddt < 0
    eps = tf.WaterJNgems(water).electroPropertiesSolvent(298.15, 1e5, 0)
    assert eps.epsilon.val == pytest.approx(78.4, abs=0.5)
    ps = tf.WaterHGKreaktoro(water).propertiesSolvent(298.15, 1e5, 0)
    eps2 = tf.WaterJNreaktoro(water).electroPropertiesSolvent(298.15, 1e5, ps, 0)
    assert eps2.epsilon.val == pytest.approx(78.4, abs=0.5)
    assert tf.WaterHGKreaktoro(water).thermoPropertiesSubstance(298.15, 1e5, 0, "NEA_HGK").gibbs_energy.val < 0


@needs_autodiff


def test_reaction_models():
    reaction = tf.Reaction()
    reaction.setReferenceT(298.15)
    reaction.setReferenceP(1e5)
    ref = tf.ThermoPropertiesReaction()
    ref.reaction_volume.val = 3.0
    reaction.setThermoReferenceProperties(ref)
    parameters = tf.ThermoParametersReaction()
    parameters.reaction_V_fT_coeff = [1e-4, 2e-7, 1e-10, -4e-5, 1e-9]
    reaction.setThermoParameters(parameters)
    model = tf.Reaction_Vol_fT(reaction)
    tpr = tf.ThermoPropertiesReaction()
    tpr.reaction_gibbs_energy.val = -5e4
    at_reference = model.thermoProperties(298.15, 1e5, tpr)
    assert at_reference.reaction_gibbs_energy.val == pytest.approx(-5e4)
    assert at_reference.reaction_volume.val == pytest.approx(3.0)
    high = model.thermoProperties(450.0, 5e7, tpr)
    assert high.reaction_gibbs_energy.ddp * 1e5 == pytest.approx(high.reaction_volume.val, rel=1e-9)


@needs_autodiff


def test_hollandpowell98_solute_relations(engine):
    solute = tf.Substance()
    solute.setSymbol("X")
    solute.setReferenceT(298.15)
    solute.setReferenceP(1e5)
    ref = tf.ThermoPropertiesSubstance()
    ref.gibbs_energy.val, ref.enthalpy.val, ref.entropy.val = -1e5, -1.2e5, 50.0
    ref.volume.val, ref.heat_capacity_cp.val = 2.0, 150.0
    solute.setThermoReferenceProperties(ref)
    parameters = tf.ThermoParametersSubstance()
    parameters.solute_holland_powell98_coeff = [0.2]
    solute.setThermoParameters(parameters)
    model = tf.SoluteHollandPowell98(solute)
    wpr = engine.propertiesSolvent(298.15, 1e5, "H2O@")
    wp = engine.propertiesSolvent(450.0, 2e7, "H2O@")
    x = model.thermoProperties(450.0, 2e7, wpr, wp)
    assert x.gibbs_energy.ddt == pytest.approx(-x.entropy.val, rel=1e-6)
    assert x.gibbs_energy.ddp * 1e5 == pytest.approx(x.volume.val, rel=1e-6)


def test_internal_helmholtz_energy_and_cv_are_completed_from_the_other_properties(database, engine):
    """U = H - P V, A = U - T S and Cv = Cp - T V alpha^2/beta for every substance with a molar volume."""
    checked = 0
    for symbol in ["H2O@", "Al(OH)2+", "Al(OH)3@", "Al(OH)4-", "Quartz"]:
        for T, P in [(298.15, 1e5), (473.15, 5e7)]:
            x = engine.thermoPropertiesSubstance(T, P, symbol)
            u = x.enthalpy.val - P / 1e5 * x.volume.val
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

@needs_autodiff

def test_zhang_duan_and_dielectric_models_have_exact_derivatives(database):
    water = database.getSubstance("H2O@")
    zd = tf.WaterZhangDuan2005(water)
    for T, P in ((400.0, 3e8), (700.0, 6e8)):
        x = zd.propertiesSolvent(T, P, 0)
        # first and second derivatives are the derivatives of the density; the mixed derivatives are symmetric (to rounding)
        assert x.density.ddt == pytest.approx(x.densityT.val, rel=1e-12)
        assert x.density.ddp == pytest.approx(x.densityP.val, rel=1e-12)
        assert x.densityT.ddt == pytest.approx(x.densityTT.val, rel=1e-12)
        assert x.densityP.ddp == pytest.approx(x.densityPP.val, rel=1e-12)
        assert x.densityT.ddp == pytest.approx(x.densityP.ddt, rel=1e-12)
        assert x.densityTP.val == pytest.approx(x.densityT.ddp, rel=1e-12)
        h = 1e-3 * T
        fd = (zd.propertiesSolvent(T + h, P, 0).densityTT.val - zd.propertiesSolvent(T - h, P, 0).densityTT.val) / (2 * h)
        assert x.densityTT.ddt == pytest.approx(fd, rel=1e-3)

    for model in (tf.WaterElectroSverjensky2014(water), tf.WaterElectroFernandez1997(water)):
        x = model.electroPropertiesSolvent(500.0, 1e8, 0)
        assert x.epsilon.ddt == pytest.approx(x.epsilonT.val, rel=1e-12)
        assert x.epsilonT.ddt == pytest.approx(x.epsilonTT.val, rel=1e-12)
        assert x.epsilon.ddp * 1e5 == pytest.approx(x.epsilonP.val, rel=1e-12)     # per bar and per Pa
        assert x.epsilonT.ddp * 1e5 == pytest.approx(x.epsilonP.ddt, rel=1e-12)    # symmetric mixed derivatives
        assert x.bornY.val == pytest.approx(x.epsilonT.val / x.epsilon.val ** 2, rel=1e-12)


@pytest.mark.skipif(getattr(tf, "with_autodiff", True), reason="only for the build without autodiff (-DTFUN_USE_AUTODIFF=OFF)")
def test_built_without_autodiff_all_the_derivatives_are_zero(engine):
    """The models calculate with plain numbers: the values are those of the autodiff build, the derivatives are 0 and
    the properties that need them (Cv) are not defined."""
    for symbol in ("Quartz", "Ca+2", "CO2@", "H2O@"):
        tps = engine.thermoPropertiesSubstance(373.15, 1e7, symbol)
        for p in ("gibbs_energy", "enthalpy", "entropy", "volume", "heat_capacity_cp", "internal_energy", "helmholtz_energy"):
            x = getattr(tps, p)
            assert x.sta[0] != tf.Status.notdefined and x.ddt == 0.0 and x.ddp == 0.0, (symbol, p)
    assert engine.thermoPropertiesSubstance(373.15, 1e7, "Quartz").heat_capacity_cv.sta[0] == tf.Status.notdefined
