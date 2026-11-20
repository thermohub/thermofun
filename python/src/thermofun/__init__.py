# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2025-2026 ThermoFun contributors

"""ThermoFun Python interface for thermodynamic calculations.

This module provides Python bindings for the ThermoFun library,
enabling thermodynamic property calculations for substances and reactions.
"""

from thermofun.PyThermoFun import *


def __getattr__(name):
    # tables is imported when it is first used, so that `python -m thermofun.tables` does not find it imported already
    # (Python then warns that the module may run twice)
    if name in ("tables", "table"):
        import importlib
        tables = importlib.import_module("thermofun.tables")
        return tables if name == "tables" else tables.table
    raise AttributeError(f"module 'thermofun' has no attribute {name!r}")
