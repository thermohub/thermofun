# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Compare two builds of ThermoFun on every record of every database.

    python pytests/golden/compare_builds.py --base .golden-baseline/install [--new PREFIX]

--base and --new are install prefixes (see build_baseline.sh). --new defaults to the active environment.
Both builds compute the same questions with golden_common.py of the working tree.
The relative tolerance is 1e-10, as in test_golden.py, and the values of the energies have a margin of two ulps of
P*V of the record (golden_common.py). The differences between 1e-12 and 1e-10 are counted and shown: they are the
rounding of the parameters between the units of a model and SI, where large terms nearly cancel. Their count has a limit
in expected_changes.json. The exit code is 1 if any value, error or status differs, except in the records
listed in expected_changes.json (wrong in the baseline, each with its reason).
"""

import argparse
import gzip
import json
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile

HERE = pathlib.Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import golden_common as gc  # noqa: E402

REL_TOL = 1e-10
ROUNDING_TOL = 1e-12  # differences above this are counted, and only those above REL_TOL fail
CHUNK = 150  # records per work item, to bound the memory


def expected_changes():
    """The (database, variant, kind, record) keys whose differences are expected. The value is None for any difference
    of the record, or (property, max_rel) if the entry accepts only small differences of one property."""
    with open(HERE / "expected_changes.json") as f:
        changes = json.load(f)["changes"]
    return {(c["database"], c["variant"], c["kind"], r): (c["property"], c["max_rel"]) if "property" in c else None
            for c in changes for r in c["records"]}


def is_expected(d, expected):
    key = (d["database"], d["variant"], d["kind"], d["record"])
    if key not in expected:
        return False
    limit = expected[key]
    return limit is None or (d["property"] == limit[0] and d["rel"] is not None and d["rel"] <= limit[1])


def rounding_limit():
    """The largest accepted number of differences between ROUNDING_TOL and REL_TOL."""
    with open(HERE / "expected_changes.json") as f:
        return json.load(f)["rounding_limit"]["count"]


def exit_code(differences, rounding, limit):
    """1 if a value, error or status differs above REL_TOL, or if the rounding differences are more than the limit."""
    return 1 if differences or rounding > limit else 0


def site_packages(prefix):
    found = sorted(pathlib.Path(prefix).glob("lib*/python*/site-packages"))
    if not found:
        raise SystemExit("no site-packages in " + str(prefix))
    return [str(p) for p in found]


def environment(prefix):
    """Environment of a worker for the build in prefix (None: the active environment)."""
    env = dict(os.environ)
    if prefix is None:
        return env
    paths = site_packages(prefix) + [str(HERE)]
    env["PYTHONPATH"] = os.pathsep.join(paths)
    libs = [str(p) for p in sorted(pathlib.Path(prefix).glob("lib*")) if p.is_dir()]
    var = "DYLD_LIBRARY_PATH" if sys.platform == "darwin" else "LD_LIBRARY_PATH"
    env[var] = os.pathsep.join(libs + ([env[var]] if env.get(var) else []))
    # A conda compiler writes the environment's lib folder first into the RPATH of the module, so a ThermoFun
    # installed in the active environment would be loaded instead of the one in prefix. Preload the right one.
    if sys.platform.startswith("linux"):
        own = [str(p) for lib in libs for p in sorted(pathlib.Path(lib).glob("libThermoFun.so"))]
        if own:
            env["LD_PRELOAD"] = os.pathsep.join(own[:1] + ([env["LD_PRELOAD"]] if env.get("LD_PRELOAD") else []))
    return env


def jobs(variants, only_db):
    """Work items (database path, variant, symbols) in a fixed order."""
    import thermofun
    out = []
    for db in gc.databases():
        if only_db and gc.db_name(db) not in only_db:
            continue
        engine = gc.make_engine(db, "default")
        subst = sorted(engine.database().getSubstancesList())
        reacs = sorted(engine.database().getReactionsList())
        for variant in gc.full_variants_for(db):
            if variants and variant not in variants:
                continue
            for i in range(0, max(len(subst), len(reacs), 1), CHUNK):
                out.append((db, variant, {"substances": subst[i:i + CHUNK], "reactions": reacs[i:i + CHUNK]}))
    del thermofun
    return out


def job_name(index, db, variant):
    return "%05d__%s__%s.json.gz" % (index, gc.db_name(db), variant)


def worker(outdir, variants, only_db):
    """Compute all work items and write one file for each."""
    outdir = pathlib.Path(outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    for index, (db, variant, symbols) in enumerate(jobs(variants, only_db)):
        result = gc.compute(db, variant, gc.FULL_T, gc.FULL_P, symbols)
        result["database"] = gc.db_name(db)
        result["variant"] = variant
        with gzip.open(outdir / job_name(index, db, variant), "wt") as f:
            json.dump(result, f, separators=(",", ":"))


def start_worker(prefix, outdir, variants, only_db):
    cmd = [sys.executable, str(HERE / "compare_builds.py"), "--worker", str(outdir)]
    for v in variants or []:
        cmd += ["--variant", v]
    for d in only_db or []:
        cmd += ["--db", d]
    return subprocess.Popen(cmd, env=environment(prefix))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--base", help="install prefix of the baseline build")
    parser.add_argument("--new", help="install prefix of the new build (default: the active environment)")
    parser.add_argument("--variant", action="append", help="only these variants")
    parser.add_argument("--db", action="append", help="only these database names")
    parser.add_argument("--max-report", type=int, default=20)
    parser.add_argument("--worker", help=argparse.SUPPRESS)
    args = parser.parse_args()

    if args.worker:
        worker(args.worker, args.variant, args.db)
        return 0
    if not args.base:
        parser.error("--base is required")

    tmp = pathlib.Path(tempfile.mkdtemp(prefix="golden-compare-"))
    try:
        procs = [start_worker(args.base, tmp / "base", args.variant, args.db),
                 start_worker(args.new, tmp / "new", args.variant, args.db)]
        codes = [p.wait() for p in procs]  # wait for both before the temporary folder is removed
        if any(code != 0 for code in codes):
            print("a worker failed")
            return 2
        diffs, accepted, rounding, files = [], 0, 0, sorted(p.name for p in (tmp / "base").iterdir())
        expected = expected_changes()
        if files != sorted(p.name for p in (tmp / "new").iterdir()):
            print("the two builds produced different work items")
            return 2
        for name in files:
            with gzip.open(tmp / "base" / name, "rt") as f:
                base = json.load(f)
            with gzip.open(tmp / "new" / name, "rt") as f:
                got = json.load(f)
            for d in gc.compare(base, got, REL_TOL):
                d["database"], d["variant"] = base["database"], base["variant"]
                if is_expected(d, expected):
                    accepted += 1
                else:
                    diffs.append(d)
            for d in gc.compare(base, got, ROUNDING_TOL):
                d["database"], d["variant"] = base["database"], base["variant"]
                rounding += not is_expected(d, expected)
        print(gc.format_report(diffs, "compared %d work items" % len(files), args.max_report))
        print("accepted differences (expected_changes.json): %d" % accepted)
        rounding -= len(diffs)
        limit = rounding_limit()
        print("differences between %g and %g (rounding): %d, limit %d" % (ROUNDING_TOL, REL_TOL, rounding, limit))
        if rounding > limit:
            print("more rounding differences than the limit in expected_changes.json: look for a new source of them")
        return exit_code(diffs, rounding, limit)
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
