# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Reports the records that can be calculated in two ways and the difference between the ways.

    python -m thermofun.consistency DATABASE [--TP 298.15,1e5 373.15,1e7 ...] [--threshold 100]

The report is information for the data owners. The exit code is 0 also if a record is not consistent.
"""

import argparse
import sys

import thermofun as tf


def report(database, TP=((298.15, 1e5),), threshold=100.0):
    engine = tf.ThermoEngine(database) if not isinstance(database, tf.ThermoEngine) else database
    options = tf.ConsistencyOptions()
    options.delta_G_threshold = threshold
    return engine.consistencyReport(list(TP), options)


def table(items):
    lines = ["| Record | Kind | T (K) | P (Pa) | delta G (J/mol) | delta logK | Flag |", "|---|---|---|---|---|---|---|"]
    for i in items:
        lines.append("| %s | %s | %g | %g | %.6g | %.6g | %s |" % (i.symbol, i.kind, i.T, i.P, i.delta_G, i.delta_logK, i.flag))
    return "\n".join(lines)


def main(argv=None):
    parser = argparse.ArgumentParser(prog="python -m thermofun.consistency")
    parser.add_argument("database")
    parser.add_argument("--TP", nargs="*", default=["298.15,1e5"], help="pairs T,P in K and Pa")
    parser.add_argument("--threshold", type=float, default=100.0, help="J/mol")
    args = parser.parse_args(argv)
    TP = [tuple(float(x) for x in pair.split(",")) for pair in args.TP]
    print(table(report(args.database, TP, args.threshold)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
