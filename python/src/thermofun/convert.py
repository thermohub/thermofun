# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Converts a database file between the legacy format and the v2 format.

    python -m thermofun.convert INPUT OUTPUT [--to v2|legacy] [--format json|yaml] [--split] [--readable] [--report report.md]

INPUT is a legacy or v2 file or folder (JSON or YAML). The C++ library does the work (convertFile). With --split the output is a
dataset folder with one file for each record (the layout of a ThermoHub dataset). A v2 file has each number so that it reads
back to the same double, so a conversion does not change a result. With --readable the numbers are short decimals (for example
-56.484 for the -56.484001159668 of a GEMS export): the values change a little, and the report says how many.
"""

import argparse
import sys

import thermofun as tf


def convert_file(input, output, to="v2", fmt="json", split=False, report=None, exact=None, readable=False):
    """Converts a file (the C++ convertFile). Returns the Diagnostics of the load. The numbers are exact unless readable is
    True (exact is kept for the scripts that give it: exact=False asks for readable numbers)."""
    if exact is False:
        readable = True
    if to not in ("v2", "legacy"):
        raise ValueError("to must be v2 or legacy")
    if fmt not in ("json", "yaml"):
        raise ValueError("fmt must be json or yaml")
    return tf.convertFile(str(input), str(output), tf.DataFormat.V2 if to == "v2" else tf.DataFormat.Legacy,
                          yaml=fmt == "yaml", split=split, readable=readable, report=str(report) if report else "")


def main(argv=None):
    parser = argparse.ArgumentParser(prog="python -m thermofun.convert")
    parser.add_argument("input")
    parser.add_argument("output")
    parser.add_argument("--to", choices=["v2", "legacy"], default="v2")
    parser.add_argument("--format", choices=["json", "yaml"], default="json", dest="fmt")
    parser.add_argument("--split", action="store_true", help="write a dataset folder with one file for each record")
    parser.add_argument("--readable", action="store_true", help="write short decimals: the values change a little (the report says how many)")
    parser.add_argument("--exact", action="store_true", help="the default now: each number reads back to the same double (kept for old scripts)")
    parser.add_argument("--report", help="write the migration report (Markdown)")
    args = parser.parse_args(argv)
    convert_file(args.input, args.output, args.to, args.fmt, args.split, args.report, readable=args.readable and not args.exact)
    return 0


if __name__ == "__main__":
    sys.exit(main())
