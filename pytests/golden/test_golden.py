# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Recompute the subset of every reference file and compare it with the stored results."""

import gzip
import json
import pathlib
import platform
import sys

import pytest

import golden_common as gc

DATA = pathlib.Path(__file__).resolve().parent / "data"
FILES = sorted(DATA.glob("*.json.gz"))
# The references are made on Linux x86-64. Another processor or mathematical library (macOS arm64: fused multiply-add,
# another libm) gives up to 2.2e-10 at 273.15 K and 5 kbar, where the density of water is found by iteration.
REFERENCE_PLATFORM = sys.platform.startswith("linux") and platform.machine() in ("x86_64", "AMD64")
REL_TOL = 1e-10 if REFERENCE_PLATFORM else 1e-9


@pytest.mark.parametrize("path", FILES, ids=[p.name for p in FILES])
def test_golden(path):
    with gzip.open(path, "rt") as f:
        expected = json.load(f)
    symbols = {"substances": list(expected["substances"]), "reactions": list(expected["reactions"])}
    got = gc.compute(gc.ROOT / expected["database"], expected["variant"],
                     expected["grid"]["T"], expected["grid"]["P"], symbols)
    diffs = gc.compare(expected, got, REL_TOL)
    assert not diffs, gc.format_report(diffs, path.name)


def test_reference_files_exist():
    assert FILES, "no reference files in pytests/golden/data; run make_references.py"
