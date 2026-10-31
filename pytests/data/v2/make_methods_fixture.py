# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Writes pytests/data/v2/methods.json: a dataset with its own solvent, a method record, a parameter set and data sources."""

import json
import os

OUT = os.path.dirname(os.path.abspath(__file__))

doc = {
    "thermohub": "2.0.0-draft.1",
    "thermodataset": {"symbol": "projectrecords", "name": "Records of the project scope", "solvent": "WATER"},
    "data_sources": {
        "1998HOL/POW": {"title": "An internally consistent thermodynamic data set", "year": 1998, "journal": "J. Metamorph. Geol."},
        "mydata": {"title": "A source with a free symbol"},
    },
    "methods": {
        "CpPolynomial": {
            "kind": "reference", "tier": 2, "operates_on": "substance", "contributes_to": ["G", "H", "S", "Cp", "V"],
            "citation": "1998HOL/POW",
        },
    },
    "parameter_sets": {
        "hkf_na": {"method": "HKF88", "parameters": {"a1": 1.0e-6, "a2": 100.0}, "reference": "1998HOL/POW"},
    },
    "elements": {"H": {"number": 1, "molar_mass": 1.00794}},
    "substances": {
        "WATER": {
            "name": "Water", "formula": "H2O@", "charge": 0, "class": "water_solvent", "aggregate_state": "aqueous",
            "standard_state_properties": {"G": -237183, "H": -285881, "S": 69.923, "Cp": 75.36, "V": 1.80684e-5},
            "methods": [{"method": "WaterIAPWS95"}, {"method": "JohnsonNorton91", "implementation": "reaktoro"}],
            "data_source": "1998HOL/POW",
        },
    },
}

with open(os.path.join(OUT, "methods.json"), "w") as f:
    json.dump(doc, f, indent=1)
    f.write("\n")

bad = {
    "V2-MTH-010-parameter-definitions": {"thermohub": "2.0.0-draft.1", "methods": {"HKF88": {
        "operates_on": "substance", "parameter_definitions": {"zz": {"unit": "J/mol"}}}}},
    "V2-MTH-011-operates-on": {"thermohub": "2.0.0-draft.1", "methods": {"HKF88": {"operates_on": "reaction"}}},
}
for name, d in bad.items():
    with open(os.path.join(OUT, "invalid", name + ".json"), "w") as f:
        json.dump(d, f, indent=1)
        f.write("\n")
