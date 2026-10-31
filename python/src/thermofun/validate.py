# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Reports the problems of a database.

    python -m thermofun.validate INPUT

The exit code is 1 if there is an error.
"""

import argparse
import sys

import thermofun as tf


def main(argv=None):
    parser = argparse.ArgumentParser(prog="python -m thermofun.validate")
    parser.add_argument("input")
    args = parser.parse_args(argv)
    try:
        database = tf.Database(args.input)
    except Exception as error:   # the reader refuses the file; its message has the diagnostics
        print(error)
        return 1
    problems = database.validate()
    print(problems)
    return 0 if problems.ok() else 1


if __name__ == "__main__":
    sys.exit(main())
