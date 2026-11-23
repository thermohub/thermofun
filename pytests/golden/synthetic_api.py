# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""A database built through the API for the methods that legacy files cannot hold.

The HP98 solute model (legacy id 41) and the Anderson91 model (id 42) are ignored in legacy files.
Only setMethodGenEoS selects them. This follows examples/solute-hp98 and examples/solute-anderson91.
"""

import thermofun

# Legacy method ids that this database covers
API_METHOD_IDS = {41, 42}
SOLVENT = "water"


def _solute(symbol, method):
    sub = thermofun.Substance()
    sub.setSymbol(symbol)
    sub.setFormula("OH-")
    sub.setSubstanceClass(thermofun.SubstanceClass.AQSOLUTE)
    sub.setAggregateState(thermofun.AggregateState.AQUEOUS)
    sub.setMethodGenEoS(method)
    sub.setReferenceP(1e5)
    sub.setReferenceT(298.15)
    params = thermofun.ThermoParametersSubstance()
    params.solute_holland_powell98_coeff = [0.0]
    sub.setThermoParameters(params)
    props = thermofun.ThermoPropertiesSubstance()
    props.gibbs_energy = thermofun.ThermoScalar(-157220.0)
    props.enthalpy = thermofun.ThermoScalar(-230020.0)
    props.entropy = thermofun.ThermoScalar(-10.71)
    props.heat_capacity_cp = thermofun.ThermoScalar(-137.2)
    props.volume = thermofun.ThermoScalar(-0.418)
    sub.setThermoReferenceProperties(props)
    return sub


def build_database():
    db = thermofun.Database()
    water = thermofun.Substance()
    water.setSymbol(SOLVENT)
    water.setFormula("H2O")
    water.setSubstanceClass(thermofun.SubstanceClass.AQSOLVENT)
    water.setAggregateState(thermofun.AggregateState.AQUEOUS)
    water.setMethodGenEoS(thermofun.MethodGenEoS_Thrift.CTPM_WJNR)
    water.setMethod_T(thermofun.MethodCorrT_Thrift.CTM_WWP)
    water.setReferenceP(1e5)
    water.setReferenceT(298.15)
    db.addSubstance(water)
    db.addSubstance(_solute("OH-hp98", thermofun.MethodGenEoS_Thrift.CTPM_HP98))
    db.addSubstance(_solute("OH-an91", thermofun.MethodGenEoS_Thrift.CTPM_AN91))
    return db


def make_engine():
    engine = thermofun.ThermoEngine(build_database())
    engine.setSolventSymbol(SOLVENT)
    return engine
