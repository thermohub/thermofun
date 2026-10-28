# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Fail when a legacy database holds a key that the legacy reader does not handle.

KNOWN is the table of docs/redesign/05-legacy-compatibility.md section 2.
A new key must be added to the reader and to this table together.
"""

import json
import re

import pytest

import golden_common as gc
import synthetic_api

KNOWN = {
    "file": {"thermodataset", "datasources", "date", "conventions"},
    "substance": {
        "symbol", "name", "formula", "formula_charge", "aggregate_state", "class_", "Tst", "Pst",
        "sm_gibbs_energy", "sm_enthalpy", "sm_entropy_abs", "sm_heat_capacity_p", "sm_volume",
        "TPMethods", "m_expansivity", "m_compressibility", "mass_per_mole", "reaction",
        "datasources", "references", "_label", "limitsTP", "_comment",
    },
    "tpmethod": {
        "method", "eos_hkf_coeffs", "m_heat_capacity_ft_coeffs", "limitsTP", "m_phase_trans_props",
        "m_landau_phase_trans_props", "eos_gas_crit_props", "eos_akinfiev_diamond_coeffs",
        "eos_birch_murnaghan_coeffs", "eos_churakov_gottschalk_coeffs", "m_volume_fpt_coeffs",
        "solute_holland_powell98_coeff",
        # added by new_models
        "eos_tait_hp11_coeffs", "eos_extended_uniquac_coeffs", "nasa_polynomial_coeffs", "tp_table",
    },
    "reaction": {
        "symbol", "equation", "reactants", "Tst", "Pst", "logKr", "drsm_gibbs_energy", "drsm_enthalpy",
        "drsm_entropy", "drsm_volume", "drsm_heat_capacity_p", "limitsTP", "TPMethods",
        "datasources", "references", "_comment",
    },
    "rtpmethod": {"method", "logk_ft_coeffs", "dr_heat_capacity_ft_coeffs", "dr_ryzhenko_coeffs", "limitsTP",
                 "dr_volume_fpt_coeffs", "dr_marshall_franck_coeffs", "dr_dolejs_manning10_coeffs"},
    "element": {"symbol", "class_", "entropy", "atomic_mass", "datasources", "references"},
}
RECORD_LISTS = {"substances", "reactions", "elements"}


def method_ids(map_name):
    """Ids of a method map in ThermoFun/GlobalVariables.h."""
    text = (gc.ROOT / "ThermoFun" / "GlobalVariables.h").read_text()
    body = text.split(map_name, 1)[1].split("};", 1)[0]
    body = "\n".join(line for line in body.splitlines() if not line.lstrip().startswith("//"))
    return {int(i) for i in re.findall(r"\{\s*(\d+)\s*,", body)}


SUBSTANCE_METHODS = method_ids("enum_method_substance")
REACTION_METHODS = method_ids("enum_method_reaction")


def method_id(entry):
    """The id of a TPMethods entry, {"<id>": "<name>"}."""
    value = entry.get("method")
    if not isinstance(value, dict) or len(value) != 1:
        return None
    key = next(iter(value))
    return int(key) if key.isdigit() else None


def inventory(path):
    """Unknown keys and method ids of one database file."""
    found = []
    data = json.loads(path.read_text())
    for key in data:
        if key not in KNOWN["file"] | RECORD_LISTS:
            found.append(("file", key))
    for record in data.get("substances", []):
        found += [("substance", k) for k in record if k not in KNOWN["substance"]]
        for entry in record.get("TPMethods", []):
            found += [("tpmethod", k) for k in entry if k not in KNOWN["tpmethod"]]
            if method_id(entry) not in SUBSTANCE_METHODS:
                found.append(("substance method id", entry.get("method")))
    for record in data.get("reactions", []):
        found += [("reaction", k) for k in record if k not in KNOWN["reaction"]]
        for entry in record.get("TPMethods", []):
            found += [("rtpmethod", k) for k in entry if k not in KNOWN["rtpmethod"]]
            if method_id(entry) not in REACTION_METHODS:
                found.append(("reaction method id", entry.get("method")))
    for record in data.get("elements", []):
        found += [("element", k) for k in record if k not in KNOWN["element"]]
    return found


DATABASES = gc.databases()
JSON_DATABASES = [p for p in DATABASES if p.suffix == ".json"]


@pytest.mark.parametrize("path", JSON_DATABASES, ids=[gc.db_name(p) for p in JSON_DATABASES])
def test_no_unknown_legacy_keys(path):
    unknown = sorted({(kind, str(key)) for kind, key in inventory(path)})
    assert not unknown, "keys or method ids the legacy reader does not handle: %s" % unknown


def test_method_id_maps():
    assert SUBSTANCE_METHODS and max(SUBSTANCE_METHODS) >= 46
    assert REACTION_METHODS == {0} | set(range(3, 15))
    assert not SUBSTANCE_METHODS & {7, 15, 24}


def seen_method_ids():
    """Legacy method ids used by the golden databases, and those covered by the database built through the API."""
    substances, reactions = set(), set()
    for path in DATABASES:
        if path.suffix != ".json":
            continue
        data = json.loads(path.read_text())
        for record in data.get("substances", []):
            substances |= {method_id(e) for e in record.get("TPMethods", [])}
        for record in data.get("reactions", []):
            reactions |= {method_id(e) for e in record.get("TPMethods", [])}
    return substances | synthetic_api.API_METHOD_IDS, reactions


def test_every_method_is_covered():
    substances, reactions = seen_method_ids()
    assert SUBSTANCE_METHODS - substances == set(), "substance methods without a golden record"
    assert REACTION_METHODS - reactions == set(), "reaction methods without a golden record"
