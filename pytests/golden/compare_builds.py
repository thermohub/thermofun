# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Compare two builds of ThermoFun on every record of every database.

    python pytests/golden/compare_builds.py --base .golden-baseline/install [--new PREFIX]

--base and --new are install prefixes (see build_baseline.sh). --new defaults to the active environment.
Both builds compute the same questions with golden_common.py of the working tree.
The relative tolerance is 1e-12. The exit code is 1 if any value, error or status differs.
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

REL_TOL = 1e-12
CHUNK = 150  # records per work item, to bound the memory


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
        diffs, files = [], sorted(p.name for p in (tmp / "base").iterdir())
        if files != sorted(p.name for p in (tmp / "new").iterdir()):
            print("the two builds produced different work items")
            return 2
        for name in files:
            with gzip.open(tmp / "base" / name, "rt") as f:
                expected = json.load(f)
            with gzip.open(tmp / "new" / name, "rt") as f:
                got = json.load(f)
            for d in gc.compare(expected, got, REL_TOL):
                d["database"], d["variant"] = expected["database"], expected["variant"]
                diffs.append(d)
        print(gc.format_report(diffs, "compared %d work items" % len(files), args.max_report))
        return 1 if diffs else 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
