# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Shared code of the golden reference suite.

The committed subset (make_references.py, test_golden.py) and the two-build comparison (compare_builds.py)
both call compute() and compare(), so they ask the same questions.
This module uses only the Python API that exists in ThermoFun 0.7.3.
"""

import hashlib
import math
import pathlib

import thermofun

ROOT = pathlib.Path(__file__).resolve().parents[2]

SUBSET_T = [273.15, 298.15, 373.15, 623.15, 1273.15]  # K
SUBSET_P = [0.0, 1e5, 1e8, 5e8]  # Pa, 0 = saturation
FULL_T = [273.15, 283.15, 298.15, 323.15, 348.15, 373.15, 423.15, 473.15, 523.15,
          573.15, 623.15, 673.15, 873.15, 1073.15, 1273.15]
FULL_P = [0.0, 1e5, 1e7, 1e8, 5e8, 1e9]

SUBST_PROPS = ["gibbs_energy", "enthalpy", "entropy", "heat_capacity_cp", "volume",
               "heat_capacity_cv", "internal_energy", "helmholtz_energy"]
REAC_PROPS = ["log_equilibrium_constant", "reaction_gibbs_energy", "reaction_enthalpy",
              "reaction_entropy", "reaction_heat_capacity_cp", "reaction_volume"]
VARIANTS = ["default", "gas", "errors", "steam", "berman"]

SCALAR_FIELDS = ["val", "ddt", "ddp", "err", "sta"]
SOLVENT_CALLS = ["propertiesSolvent", "electroPropertiesSolvent"]
FIRST_N = 2  # symbols taken from each group of the subset selection (3 gave 5.5 MB)
# errors, steam and berman run only on these databases (09 section 1.3 rule 5)
STEAM_BERMAN_DATABASES = {"aq17-thermofun", "mines16-thermofun", "pytests--Substances--Solute--test-hkf-thermofun",
                          "tests--extra_models--extra_models_db"}

# Databases outside Resources/databases that are part of the suite
EXTRA_DATABASES = ["tests/extra_models/extra_models_db.json"]
SYNTHETIC_DIR = "pytests/data/legacy-synthetic/"
SYNTHETIC_API = "pytests/golden/synthetic_api.py"
SYNTHETIC_API_NAME = "synthetic-api"
# Files of pytests/ that are not databases
NOT_DATABASES = {"pytests/golden/data/subset.json"}


def _rel(path):
    return pathlib.Path(path).resolve().relative_to(ROOT).as_posix()


def is_synthetic(db_path):
    rel = _rel(db_path)
    return rel.startswith(SYNTHETIC_DIR) or rel == SYNTHETIC_API


def _loads(path):
    """Return a ThermoEngine for the file, or None if the file is not a database."""
    try:
        engine = thermofun.ThermoEngine(str(path))
        if engine.database().numberOfSubstances() == 0:
            return None
        return engine
    except Exception:
        return None


def databases():
    """Paths of all databases of the suite: bundled, test databases that load, extra-model database."""
    found = sorted((ROOT / "Resources" / "databases").glob("*.json"))
    for path in sorted((ROOT / "pytests").rglob("*.json")):
        if _rel(path) in NOT_DATABASES or "golden" in path.parts:
            continue
        if _loads(path) is not None:
            found.append(path)
    for name in EXTRA_DATABASES:
        if (ROOT / name).exists():
            found.append(ROOT / name)
    found.append(ROOT / SYNTHETIC_API)
    unique, seen = [], set()
    for path in found:  # a file with the same content as an earlier one is skipped
        digest = hashlib.md5(path.read_bytes()).hexdigest()
        if digest not in seen:
            seen.add(digest)
            unique.append(path)
    names = [db_name(p) for p in unique]
    if len(set(names)) != len(names):
        raise RuntimeError("two databases have the same name")
    return unique


def db_name(db_path):
    """Short unique name of a database: the file stem for bundled ones, the path below the root for others."""
    rel = _rel(db_path)
    if rel == SYNTHETIC_API:
        return SYNTHETIC_API_NAME
    if rel.startswith("Resources/databases/"):
        return pathlib.Path(rel).stem
    return rel[:-len(".json")].replace("/", "--")


# Synthetic databases with a water solvent: large files, so `gas` and `errors` are left out (09 section 1.3 rule 5)
WATER_SYNTHETIC = {"r09-logk_marshall_frank78", "r14-logk_dolejs_manning10", "s27-water_diel_sverj14",
                   "s28-water_diel_fern97", "s33-water_pvt_zhang_duan05", "synthetic-api"}


def variants_for(db_path):
    """The variants stored for a database in the committed subset."""
    if db_name(db_path).rsplit("--", 1)[-1] in WATER_SYNTHETIC:
        return ["default", "steam", "berman"]
    if db_name(db_path) in STEAM_BERMAN_DATABASES or is_synthetic(db_path):
        return list(VARIANTS)
    return [v for v in VARIANTS if v not in ("errors", "steam", "berman")]


def make_engine(db_path, variant):
    """Load the database alone and set the preferences of the variant."""
    if pathlib.Path(db_path).suffix == ".py":  # the database built through the API
        import synthetic_api
        engine = synthetic_api.make_engine()
    else:
        engine = thermofun.ThermoEngine(str(db_path))
    if variant == "default":
        pass
    elif variant == "gas":
        prefs = thermofun.EnginePreferences()
        prefs.apply_pressure_correction_to_gas_props = True
        prefs.cancel_fugacity_coefficient_in_gas_props = False
        engine.setPreferences(prefs)
    elif variant == "errors":
        prefs = thermofun.EnginePreferences()
        prefs.propagate_parameter_errors = True
        engine.setPreferences(prefs)
    elif variant == "steam":
        engine.conventions.water_properties = "STEAM_TABLES"
    elif variant == "berman":
        engine.conventions.apparent_properties = "BERMAN_BROWN"
    else:
        raise ValueError("unknown variant " + variant)
    return engine


def scalar(x):
    """[val, ddt, ddp, err, status code] of a ThermoScalar; the code is the integer of the Status enum."""
    return [x.val, x.ddt, x.ddp, x.err, int(x.sta[0])]


def _grid(T, P):
    return [(t, p) for t in T for p in P]


def _first_line(exc):
    for line in str(exc).splitlines():  # the messages start with a line of asterisks
        if line.strip("* \t"):
            return line.strip("* \t")
    return type(exc).__name__


def _scalar_members(obj):
    """Names of the ThermoScalar members of a properties object, sorted."""
    names = []
    for name in dir(obj):
        if name.startswith("_"):
            continue
        try:
            if isinstance(getattr(obj, name), thermofun.ThermoScalar):
                names.append(name)
        except Exception:
            pass
    return sorted(names)


def _is_solvent(substance):
    return substance.substanceClass() == thermofun.SubstanceClass.AQSOLVENT


def compute(db_path, variant, T, P, symbols=None):
    """Properties of the records at every (T, P) pair, T outer loop, P inner loop.

    symbols: None for all records, or {"substances": [...], "reactions": [...]}.
    A point that raises an exception is stored as {"error": first line of the message}.
    """
    engine = make_engine(db_path, variant)
    db = engine.database()
    if symbols is None:
        subst = sorted(db.getSubstancesList())
        reacs = sorted(db.getReactionsList())
    else:
        subst = list(symbols["substances"])
        reacs = list(symbols["reactions"])
    points = _grid(T, P)
    out = {"grid": {"T": list(T), "P": list(P)}, "substances": {}, "reactions": {}, "solvent": {}}

    for sym in subst:
        rec = {prop: [] for prop in SUBST_PROPS}
        for t, p in points:
            try:
                res = engine.thermoPropertiesSubstance(t, p, sym)
                for prop in SUBST_PROPS:
                    rec[prop].append(scalar(getattr(res, prop)))
            except Exception as exc:
                for prop in SUBST_PROPS:
                    rec[prop].append({"error": _first_line(exc)})
        out["substances"][sym] = rec

    for sym in reacs:
        rec = {prop: [] for prop in REAC_PROPS}
        for t, p in points:
            try:
                res = engine.thermoPropertiesReaction(t, p, sym)
                for prop in REAC_PROPS:
                    rec[prop].append(scalar(getattr(res, prop)))
            except Exception as exc:
                for prop in REAC_PROPS:
                    rec[prop].append({"error": _first_line(exc)})
        out["reactions"][sym] = rec

    for sym in subst:
        if not _is_solvent(db.getSubstance(sym)):
            continue
        rec = {}
        for call in SOLVENT_CALLS:
            for t, p in points:
                try:
                    res = getattr(engine, call)(t, p, sym)
                    for name in _scalar_members(res):
                        rec.setdefault(call + "." + name, []).append(scalar(getattr(res, name)))
                except Exception as exc:
                    rec.setdefault(call + ".error", []).append({"error": _first_line(exc)})
        out["solvent"][sym] = rec
    return out


def _has(value):
    """True if a reference property is given in the record."""
    return int(value.sta[0]) != 0 or value.val != 0.0


def select_subset(db_path):
    """The records of the committed subset of one database (09 §1.3). Deterministic."""
    engine = make_engine(db_path, "default")
    db = engine.database()
    if is_synthetic(db_path):  # every record of the synthetic fixtures
        return {"substances": sorted(db.getSubstancesList()), "reactions": sorted(db.getReactionsList())}

    groups = {}
    for sym in sorted(db.getSubstancesList()):
        s = db.getSubstance(sym)
        par = s.thermoParameters()
        key = (int(s.substanceClass()), int(s.aggregateState()),
               tuple(sorted([str(s.methodGenEOS()), str(s.method_T()), str(s.method_P())])),
               bool(s.reactionSymbol()), len(par.phase_transition_prop) > 0, len(par.Cp_coeff))
        groups.setdefault(key, []).append(sym)
    substances = set()
    for syms in groups.values():
        substances.update(syms[:FIRST_N])

    groups = {}
    for sym in sorted(db.getReactionsList()):
        r = db.getReaction(sym)
        ref = r.thermoReferenceProperties()
        key = (tuple(sorted([str(r.methodGenEOS()), str(r.method_T()), str(r.method_P())])),
               _has(ref.log_equilibrium_constant), _has(ref.reaction_volume),
               _has(ref.reaction_heat_capacity_cp))
        groups.setdefault(key, []).append(sym)
    reactions = set()
    for syms in groups.values():
        reactions.update(syms[:FIRST_N])

    # always included
    for sym in sorted(db.getSubstancesList()):
        s = db.getSubstance(sym)
        if _is_solvent(s):
            substances.add(sym)
    for sym in sorted(substances):  # selected substances defined by a reaction bring that reaction
        rsym = db.getSubstance(sym).reactionSymbol()
        if rsym and db.containsReaction(rsym):
            reactions.add(rsym)
    if db.containsSubstance("H+"):
        substances.add("H+")
    for rsym in list(reactions):
        for other in db.getReaction(rsym).reactants():
            symbol = other if isinstance(other, str) else other[0]
            if db.containsSubstance(symbol):
                substances.add(symbol)
    return {"substances": sorted(substances), "reactions": sorted(reactions)}


def _isclose(got, exp, rel_tol):
    if isinstance(exp, float) and math.isnan(exp):
        return isinstance(got, float) and math.isnan(got)
    if math.isinf(exp) or math.isinf(got):
        return got == exp
    return math.isclose(got, exp, rel_tol=rel_tol, abs_tol=rel_tol * max(abs(exp), 1.0))


def _compare_points(kind, record, prop, exp_list, got_list, grid, rel_tol, diffs):
    points = _grid(grid["T"], grid["P"])
    if len(exp_list) != len(got_list):
        diffs.append({"kind": kind, "record": record, "property": prop, "field": "length",
                      "T": None, "P": None, "expected": len(exp_list), "got": len(got_list), "rel": None})
        return
    for (t, p), e, g in zip(points, exp_list, got_list):
        if isinstance(e, dict) or isinstance(g, dict):
            if e != g:
                diffs.append({"kind": kind, "record": record, "property": prop, "field": "error",
                              "T": t, "P": p, "expected": e, "got": g, "rel": None})
            continue
        for name, ev, gv in zip(SCALAR_FIELDS, e, g):
            if name == "sta":
                same = ev == gv
            else:
                same = _isclose(gv, ev, rel_tol)
            if not same:
                rel = None
                if name != "sta" and ev != 0:
                    rel = abs(gv - ev) / abs(ev)
                diffs.append({"kind": kind, "record": record, "property": prop, "field": name,
                              "T": t, "P": p, "expected": ev, "got": gv, "rel": rel})


def compare(expected, got, rel_tol):
    """List of differences between two results of compute() (09 §1.2). Empty if they agree."""
    diffs = []
    if expected["grid"] != got["grid"]:
        return [{"kind": "grid", "record": "", "property": "", "field": "grid", "T": None, "P": None,
                 "expected": expected["grid"], "got": got["grid"], "rel": None}]
    for kind in ["substances", "reactions", "solvent"]:
        for record in sorted(set(expected[kind]) | set(got[kind])):
            if record not in got[kind] or record not in expected[kind]:
                diffs.append({"kind": kind, "record": record, "property": "", "field": "missing",
                              "T": None, "P": None, "expected": record in expected[kind],
                              "got": record in got[kind], "rel": None})
                continue
            e_rec, g_rec = expected[kind][record], got[kind][record]
            for prop in sorted(set(e_rec) | set(g_rec)):
                if prop not in e_rec or prop not in g_rec:
                    diffs.append({"kind": kind, "record": record, "property": prop, "field": "missing",
                                  "T": None, "P": None, "expected": prop in e_rec, "got": prop in g_rec,
                                  "rel": None})
                    continue
                _compare_points(kind, record, prop, e_rec[prop], g_rec[prop], expected["grid"], rel_tol, diffs)
    return diffs


def format_report(diffs, label="", max_report=20):
    """Text table of the first differences and the total count."""
    lines = []
    if label:
        lines.append(label)
    lines.append("kind record property field T P expected got rel")
    for d in diffs[:max_report]:
        lines.append(" ".join(str(d[k]) for k in
                              ["kind", "record", "property", "field", "T", "P", "expected", "got", "rel"]))
    lines.append("total differences: %d" % len(diffs))
    return "\n".join(lines)
