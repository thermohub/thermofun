# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2025-2026 ThermoFun contributors

"""ThermoFun Python interface for thermodynamic calculations.

This module provides Python bindings for the ThermoFun library,
enabling thermodynamic property calculations for substances and reactions.
"""

from thermofun.PyThermoFun import *


def __getattr__(name):
    # batch is imported when it is first used, so that `python -m thermofun.batch` does not find it imported already
    # (Python then warns that the module may run twice). tf.batch is the module and the function: tf.batch(setup) runs it.
    if name in ("batch", "BatchSetup", "Results", "templates", "template"):
        import importlib
        module = importlib.import_module("thermofun.batch")
        return module if name == "batch" else getattr(module, name)
    raise AttributeError(f"module 'thermofun' has no attribute {name!r}")
