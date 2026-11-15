# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The code of the guide to methods as data runs.

import pathlib
import re
import shutil

import pytest

pytest.importorskip("sympy")

import thermofun as tf  # noqa: E402

from docs_outputs import expected_output, run_console, run_python  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parent.parent
GUIDE = ROOT / "docs" / "how-to" / "add-a-method-as-data.md"


def with_examples(folder):
    shutil.copytree(ROOT / "examples" / "v2", folder / "examples" / "v2")


@pytest.mark.skipif(not tf.methods.symbolicAvailable(), reason="built without SymEngine")
def test_the_code_of_the_guide_runs(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    with_examples(tmp_path)
    names = {"tf": tf}
    text = GUIDE.read_text()
    for match in re.finditer(r"```(python|console)\n(.*?)```", text, re.S):
        kind, block = match.group(1), match.group(2)
        if "pytest." in block:  # the note at the top
            continue
        if kind == "console":
            run_console(block, GUIDE.name, tmp_path)
        else:
            run_python(block, names, GUIDE.name, expected_output(text, match.end()))


def test_the_commands_and_outputs_of_the_tool_without_the_engine(tmp_path, monkeypatch):
    """The shell commands of method_tool need SymPy only: they run in every build."""
    monkeypatch.chdir(tmp_path)
    with_examples(tmp_path)
    text = GUIDE.read_text()
    for block in re.findall(r"```console\n(.*?)```", text, re.S):
        run_console(block, GUIDE.name, tmp_path)
