# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Reports the problems of a database.

    python -m thermofun.validate INPUT

The exit code is 1 if there is an error.
"""

import argparse
import os
import sys

import thermofun as tf


def _reason(error, path):
    """The diagnostics in the message of a file that cannot be read, without the frame of the message and the source line."""
    lines = []
    for line in str(error).splitlines():
        line = line.strip().strip("*").strip()
        if not line or line.startswith("Location:") or line.startswith("Error: The file cannot be read"):
            continue
        if line.startswith("Reason:"):
            line = line[len("Reason:"):].strip()
        lines.append(line.replace(os.path.abspath(path), path))
    return lines


def main(argv=None):
    parser = argparse.ArgumentParser(prog="python -m thermofun.validate",
                                     description="Report the problems of a database: one line for each, then a summary.")
    parser.add_argument("input")
    args = parser.parse_args(argv)
    try:
        database = tf.Database(args.input)
    except Exception as error:   # the reader refuses the file; its message has the diagnostics
        for line in _reason(error, args.input):
            print(line)
        print(f"{args.input}: the file cannot be read")
        return 1
    # what the reading reported (warnings, notes) and what only the whole database shows
    problems = list(database.loadDiagnostics().all()) + list(database.validate().all())
    for problem in problems:
        print(str(problem).replace(os.path.abspath(args.input), args.input))
    errors = sum(1 for p in problems if p.severity == tf.Severity.Error)
    warnings = sum(1 for p in problems if p.severity == tf.Severity.Warning)
    count = lambda n, word: f"{n} {word}" + ("" if n == 1 else "s")
    print(f"{args.input}: " + (count(errors, "error") + ", " + count(warnings, "warning") if problems else "no problems"))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
