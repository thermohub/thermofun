# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Write the golden reference results of the committed subset.

The first run selects the records of every database and writes data/subset.json.
Later runs read that file. Use --reselect to select again.
Regenerate the references only in a change that says so and explains each changed number.
"""

import argparse
import gzip
import json
import pathlib
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import golden_common as gc  # noqa: E402

DATA = pathlib.Path(__file__).resolve().parent / "data"
SUBSET_FILE = DATA / "subset.json"


def commit_sha():
    try:
        return subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=gc.ROOT, text=True).strip()
    except Exception:
        return "unknown"


def load_or_select(dbs, reselect):
    subset = {}
    if SUBSET_FILE.exists() and not reselect:
        subset = json.loads(SUBSET_FILE.read_text())
        if all(gc.db_name(db) in subset for db in dbs):
            return subset
    for db in dbs:  # selects again, or only the databases that have no selection yet
        if gc.db_name(db) in subset:
            continue
        subset[gc.db_name(db)] = gc.select_subset(db)
        print("selected %s: %d substances, %d reactions" %
              (gc.db_name(db), len(subset[gc.db_name(db)]["substances"]), len(subset[gc.db_name(db)]["reactions"])))
    SUBSET_FILE.write_text(json.dumps(subset, indent=1, sort_keys=True) + "\n")
    return subset


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reselect", action="store_true", help="select the subset again")
    parser.add_argument("--new-only", action="store_true",
                        help="skip a database and variant whose data file exists already")
    parser.add_argument("--variant", action="append", help="only these variants")
    args = parser.parse_args()

    DATA.mkdir(exist_ok=True)
    dbs = gc.databases()
    subset = load_or_select(dbs, args.reselect)
    sha = commit_sha()
    for db in dbs:
        for variant in args.variant or gc.variants_for(db):
            path = DATA / ("%s__%s.json.gz" % (gc.db_name(db), variant))
            if args.new_only and path.exists():
                continue
            result = gc.compute(db, variant, gc.SUBSET_T, gc.SUBSET_P, subset[gc.db_name(db)])
            result["thermofun_commit"] = sha
            result["database"] = gc._rel(db)
            result["variant"] = variant
            text = json.dumps(result, separators=(",", ":"))
            path = DATA / ("%s__%s.json.gz" % (gc.db_name(db), variant))
            with gzip.GzipFile(path, "wb", mtime=0) as f:
                f.write(text.encode())
            print("%-60s %8d bytes" % (path.name, path.stat().st_size), flush=True)
    total = sum(f.stat().st_size for f in DATA.glob("*.json.gz"))
    print("total size of the data files: %.2f MB" % (total / 1e6))
    if total > 3e6:
        print("above 3 MB: apply the size reductions of pytests/golden/README.md")


if __name__ == "__main__":
    main()
