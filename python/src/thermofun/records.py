# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Builders of v2 records.

    from thermofun import records as rec

    quartz = (rec.substance("Quartz", formula="SiO2", aggregate_state="crystal", class_="component")
                 .standard_state(G=-856288, H=-910700, S=41.46, Cp=44.6, V="22.688 cm3/mol")
                 .method("CpIntegration", segments=[dict(t_min=298.15, t_max=844, a0=44.6, a1=0.0378, a2=-1.0e6)])
                 .method("MurnaghanHP98", alpha0=1.24e-5, kappa0="730 kbar"))
    problems = quartz.check()   # the reader and validate() on this record, now
    database.addSubstance(quartz.to_json())

A value is a number, a text "number unit", or a dictionary {value, unit, uncertainty}: the same forms as in a file.
The builders write ordinary v2 records; the reader checks them.
"""

import copy
import json


def _clean(value):
    return copy.deepcopy(value)


class _Builder:
    kind = "record"
    section = "records"

    def __init__(self, symbol, **fields):
        self._symbol = symbol
        self._data = {}
        for key, value in fields.items():
            self._data[key.rstrip("_")] = _clean(value)
        self._methods = []

    # the methods of the record
    def method(self, name, segments=None, parameters=None, parameter_set=None, applicability=None, table=None,
               transitions=None, implementation=None, **values):
        """Adds a method. The keywords that are not named are the parameters of the method."""
        entry = {"method": name}
        if implementation:
            entry["implementation"] = implementation
        params = dict(parameters or {})
        params.update(values)
        if segments is not None:
            entry["segments"] = [self._segment(s) for s in segments]
        elif params:
            entry["parameters"] = _clean(params)
        if parameter_set:
            entry["parameter_set"] = parameter_set
        if transitions:
            entry["transitions"] = _clean(transitions)
        if table:
            entry["table"] = _clean(table)
        if applicability:
            entry["applicability"] = _clean(applicability)
        self._methods.append(entry)
        return self

    @staticmethod
    def _segment(segment):
        segment = dict(segment)
        out = {}
        for key in ("t_min", "t_max"):
            if key in segment:
                out[key] = segment.pop(key)
        explicit = segment.pop("parameters", None)
        out["parameters"] = _clean(explicit if explicit is not None else segment)
        return out

    def set(self, **fields):
        """Sets other fields of the record."""
        for key, value in fields.items():
            self._data[key.rstrip("_")] = _clean(value)
        return self

    def to_dict(self):
        data = dict(self._data)
        if self._methods:
            data["methods"] = copy.deepcopy(self._methods)
        data["symbol"] = self._symbol
        return data

    def to_json(self):
        return json.dumps(self.to_dict())

    def check(self):
        """The problems of this record, now: a list of Diagnostic from the reader and from validate(). The record is alone
        here, so a reference to another record is not a problem."""
        import thermofun
        return thermofun.checkRecord(self.section, self._symbol, self.to_json()).all()


class _Substance(_Builder):
    kind = "substance"
    section = "substances"

    def standard_state(self, **properties):
        """The standard-state properties G, H, S, Cp, V."""
        block = self._data.setdefault("standard_state_properties", {})
        for name, value in properties.items():
            block[name] = _clean(value)
        return self

    def record(self):
        import thermofun
        return thermofun.Substance(self.to_json()).record()


class _Reaction(_Builder):
    kind = "reaction"
    section = "reactions"

    def properties(self, **properties):
        """The reaction properties logK, dG, dH, dS, dCp, dV."""
        block = self._data.setdefault("reaction_properties", {})
        for name, value in properties.items():
            block[name] = _clean(value)
        return self

    def record(self):
        import thermofun
        return thermofun.Reaction(self.to_json()).record()


class _Element(_Builder):
    kind = "element"
    section = "elements"

    def record(self):
        import thermofun
        return thermofun.Element(self.to_json()).record()


def substance(symbol, **fields):
    """A substance record: formula, class_, aggregate_state, charge, name, ..."""
    return _Substance(symbol, **fields)


def reaction(symbol, **fields):
    """A reaction record: equation or stoichiometry, name, ..."""
    return _Reaction(symbol, **fields)


def element(symbol, **fields):
    """An element record: number, molar_mass, class_, ..."""
    return _Element(symbol, **fields)
