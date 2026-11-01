# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The code of the guide to methods as data runs.

import pathlib
import re

import pytest

pytest.importorskip("sympy")

import thermofun as tf  # noqa: E402

GUIDE = pathlib.Path(__file__).resolve().parent.parent / "docs" / "how-to" / "add-a-method-as-data.md"


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
def test_the_code_of_the_guide_runs(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    names = {"tf": tf}
    for block in re.findall(r"```python\n(.*?)```", GUIDE.read_text(), re.S):
        if "pytest." in block:  # the note at the top
            continue
        exec(block, names)
