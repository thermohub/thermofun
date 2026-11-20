# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The JSON Schema of the ThermoHub data format 2.0 (review of v2, S12, decision 9).

    python -m thermofun.schema [--write]

The schema is made here, from one description with the type of each key, and committed as
ThermoFun/IO/V2/thermohub.schema.json. pytests/test_schema.py checks that the keys of each object are those the reader knows,
that every v2 file of the repository and the v2 form of every bundled database are valid, and that the committed file is up to
date. The schema is stricter than the reader: a key that the reader keeps with a warning (V2-KEY-001) is not valid here, except
a key that starts with `_`.
"""

import json
import os
import sys

SCHEMA_ID = "https://thermohub.org/schema/thermohub-2.0.schema.json"
HERE = os.path.dirname(os.path.abspath(__file__))


def _obj(properties, required=(), description=""):
    out = {"type": "object", "properties": properties, "patternProperties": {"^_": {}}, "additionalProperties": False}
    if required:
        out["required"] = list(required)
    if description:
        out["description"] = description
    return out


def _map(of, description=""):
    out = {"type": "object", "additionalProperties": of}
    if description:
        out["description"] = description
    return out


STRING = {"type": "string"}
NUMBER = {"type": "number"}
INTEGER = {"type": "integer"}
BOOLEAN = {"type": "boolean"}
STRINGS = {"type": "array", "items": STRING}
QUANTITY = {"$ref": "#/$defs/quantity"}
REF = lambda name: {"$ref": "#/$defs/" + name}


def definitions():
    d = {}
    d["uncertainty"] = {
        "description": "A standard uncertainty (a number), or an object: standard; expanded with coverage_factor; or half_width "
                       "with distribution uniform (u = a/sqrt(3)) or triangular (u = a/sqrt(6)).",
        "oneOf": [
            {"type": "number", "minimum": 0},
            _obj({"standard": {"type": "number", "minimum": 0}, "distribution": {"const": "normal"},
                  "kind": {"enum": ["primary", "derived"]}}, ["standard"]),
            _obj({"expanded": {"type": "number", "minimum": 0}, "coverage_factor": {"type": "number", "exclusiveMinimum": 0},
                  "distribution": {"const": "normal"}, "kind": {"enum": ["primary", "derived"]}}, ["expanded", "coverage_factor"]),
            _obj({"half_width": {"type": "number", "minimum": 0}, "distribution": {"enum": ["uniform", "triangular"]},
                  "kind": {"enum": ["primary", "derived"]}}, ["half_width", "distribution"]),
        ]}
    d["quantity"] = {
        "description": "A value: a number in the default unit of its field, a text \"value unit\", or an object.",
        "oneOf": [
            NUMBER,
            {"type": "string", "pattern": r"^\s*[-+]?[0-9.]+([eE][-+]?[0-9]+)?\s+\S.*$"},
            _obj({"value": NUMBER, "unit": STRING, "uncertainty": REF("uncertainty"), "quality": STRING, "origin": STRING,
                  "calculation_path": {"type": "string", "description": "A note on how the value was made; no calculation reads it."},
                  "source": STRING}, ["value"]),
        ]}
    d["units"] = _obj({k: STRING for k in ("energy", "pressure", "temperature", "length", "mass", "amount", "volume", "entropy",
                                           "heat_capacity")},
                      description="Default units of everything inside the level where it is.")
    d["units"]["additionalProperties"] = STRING
    d["convention"] = _obj({"energy_reference": {"enum": ["benson_helgeson", "berman_brown", "ser", "assigned_enthalpy",
                                                          "gibbs_helmholtz_formation", "dft_formation_0K"]},
                            "standard_pressure": {"enum": ["1bar", "1atm"]},
                            "concentration_scale": {"enum": ["molality", "mole_fraction", "molarity"]},
                            "water_reference": {"enum": ["geochemical", "steam_tables"]}})
    d["reference_conditions"] = _obj({"T": QUANTITY, "P": QUANTITY})
    d["applicability"] = _obj({k: QUANTITY for k in ("t_min", "t_max", "p_min", "p_max")})
    d["covariances"] = {"type": "array", "items": _obj({"paths": STRINGS, "matrix": {"type": "array", "items": {"type": "array", "items": NUMBER}}},
                                                       ["paths", "matrix"])}
    d["parameters"] = _map({"oneOf": [QUANTITY, {"type": "array"}]}, "Parameters of a method, by name.")
    d["method_application"] = _obj({
        "method": STRING, "parameters": REF("parameters"), "parameter_set": STRING,
        "segments": {"type": "array", "items": _obj({"t_min": QUANTITY, "t_max": QUANTITY, "parameters": REF("parameters")})},
        "transitions": {"type": "array", "items": _obj({k: QUANTITY for k in ("T", "dS", "dH", "dV", "dTdP")})},
        "applicability": REF("applicability"), "table": {"type": "object"}, "units": REF("units"), "implementation": STRING,
        "interpolation": STRING, "order": INTEGER}, ["method"])
    d["methods_of_a_record"] = {"type": "array", "items": REF("method_application")}
    record = {"symbol": STRING, "record_type": STRING}
    d["element"] = _obj(dict(record, name=STRING, **{"class": STRING}, number=INTEGER, isotope_mass=NUMBER, valence=INTEGER,
                             molar_mass=QUANTITY, standard_entropy=QUANTITY, heat_capacity=QUANTITY, volume=QUANTITY,
                             reference_form=STRING, data_sources=STRINGS, sourcetdb={"oneOf": [STRING, STRINGS]},
                             structural={"type": "object"}, units=REF("units"), legacy_class=STRING))
    d["substance"] = _obj(dict(record, name=STRING, formula=STRING, charge=INTEGER, **{"class": STRING}, aggregate_state=STRING,
                               sourcetdb={"oneOf": [STRING, STRINGS]}, molar_mass=QUANTITY, convention=REF("convention"),
                               reference_conditions=REF("reference_conditions"),
                               standard_state_properties=_obj({k: QUANTITY for k in ("G", "H", "S", "Cp", "V", "A", "U", "Cv")}),
                               methods=REF("methods_of_a_record"), defining_reaction=STRING, preferred_property_source=STRING,
                               applicability=REF("applicability"), data_source=STRING, data_sources=STRINGS,
                               structural={"type": "object"}, deprecated=BOOLEAN, deprecated_in_favour_of=STRING,
                               units=REF("units"), covariances=REF("covariances")), ["formula"])
    d["reaction"] = _obj(dict(record, name=STRING, equation=STRING, stoichiometry=_map(NUMBER),
                              reference_conditions=REF("reference_conditions"),
                              reaction_properties=_obj({k: QUANTITY for k in ("logK", "dG", "dH", "dS", "dCp", "dV")}),
                              methods=REF("methods_of_a_record"), level=INTEGER, data_source=STRING, data_sources=STRINGS,
                              applicability=REF("applicability"), convention=REF("convention"), preferred_property_source=STRING,
                              sourcetdb={"oneOf": [STRING, STRINGS]}, deprecated=BOOLEAN, units=REF("units"),
                              covariances=REF("covariances")))
    d["method_definition"] = _obj(dict(symbol=STRING, record_type=STRING, kind=STRING, tier=INTEGER, operates_on=STRING, applies_to_classes=STRINGS,
                                       contributes_to=STRINGS, parameter_definitions={"type": ["object", "array"]},
                                       implementation_id=STRING, source={"type": ["string", "object"]}, expression={"type": ["string", "object"]},
                                       generator=STRING, source_hash=STRING, tests={"type": "array"}, effects={"type": "object"},
                                       citation=STRING))
    d["parameter_set"] = _obj(dict(symbol=STRING, record_type=STRING, method=STRING, implementation=STRING, default=BOOLEAN,
                                   formula_units={"type": "object"}, description=STRING, parameters=REF("parameters"),
                                   auxiliary={"type": "object"}, reference=STRING, units=REF("units")), ["method", "parameters"])
    d["data_source"] = _obj(dict(symbol=STRING, record_type=STRING, title=STRING, authors={"oneOf": [STRING, STRINGS]}, year=INTEGER, journal=STRING,
                                 doi=STRING, bibtex=STRING))
    d["thermodataset"] = _obj(dict(symbol=STRING, name=STRING, version=STRING, status=STRING,
                                   reference_conditions=REF("reference_conditions"), sourcetdb={"oneOf": [STRING, STRINGS]},
                                   solvent=STRING, data_source=STRING, defaults={"type": "object"},
                                   conventions=_obj({"water_triple_point_reference": STRING}), convention=REF("convention")))
    return d


def section(name):
    """A section of records: a mapping from the symbol to the record, or a list of records with symbol."""
    return {"oneOf": [_map(REF(name)), {"type": "array", "items": REF(name)}]}


RECORD_TYPES = {"element": "element", "substance": "substance", "reaction": "reaction", "method": "method_definition",
                "parameter_set": "parameter_set", "data_source": "data_source"}


def schema():
    defs = definitions()
    defs["document"] = document()
    defs["single_record"] = {
        "description": "A file of one record: its record_type, and the record without thermohub.",
        "oneOf": [{"allOf": [{"required": ["record_type"], "properties": {"record_type": {"const": t}}}, REF(name)]}
                  for t, name in RECORD_TYPES.items()]}
    return {
        "$schema": "https://json-schema.org/draft/2020-12/schema",
        "$id": SCHEMA_ID,
        "title": "ThermoHub data format 2.0",
        "description": "A file of thermodynamic data: records of elements, substances and reactions, with their methods, parameter "
                       "sets and data sources. Values are in SI unless a unit is given. Written and read by ThermoFun. A file holds "
                       "a document (with thermohub) or one record (with record_type).",
        "oneOf": [REF("document"), REF("single_record")],
        "$defs": defs,
    }


def document():
    return {
        "type": "object",
        "required": ["thermohub"],
        "properties": {
            "thermohub": {"type": "string", "pattern": r"^2\.", "description": "The version of the format."},
            "generator": STRING, "thermodataset": REF("thermodataset"), "units": REF("units"),
            "include": {"oneOf": [STRING, STRINGS]},
            "elements": section("element"), "substances": section("substance"), "reactions": section("reaction"),
            "methods": section("method_definition"), "parameter_sets": section("parameter_set"),
            "data_sources": section("data_source"),
        },
        "patternProperties": {"^_": {}},
        "additionalProperties": False,
    }


def committed_path():
    root = os.path.normpath(os.path.join(HERE, "..", "..", "..", "ThermoFun", "IO", "V2", "thermohub.schema.json"))
    return root


def text():
    return json.dumps(schema(), indent=1, sort_keys=False) + "\n"


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    if "--write" in argv:
        with open(committed_path(), "w") as f:
            f.write(text())
        return 0
    sys.stdout.write(text())
    return 0


if __name__ == "__main__":
    sys.exit(main())
