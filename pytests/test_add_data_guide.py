# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""The guide docs/how-to/add-data.md: every example runs, and its method tables are those of the code.

To write the tables again: python pytests/test_add_data_guide.py --write
"""

import os
import re
import sys

import pytest
import thermofun as tf

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GUIDE = os.path.join(ROOT, "docs", "how-to", "add-data.md")
BEGIN, END = "<!-- methods begin -->", "<!-- methods end -->"


def read_guide():
    with open(GUIDE) as f:
        return f.read()


def cell(text):
    return text.replace("|", "\\|")


def parameter_text(method):
    parts = []
    for p in method["parameters"]:
        unit = "" if p["si_unit"] == "1" else " [" + p["si_unit"] + "]"
        parts.append("`" + p["name"] + "`" + unit + ("" if p["required"] else " (optional)"))
    return "<br>".join(parts) if parts else "none"


def table(methods):
    lines = ["| Id | What it is | Parameters, with the SI unit | Implementations |", "|---|---|---|---|"]
    for m in methods:
        implementations = ", ".join("`" + i + "`" for i in m["implementations"]) or "one"
        title = m["title"] + (" (needs the solvent)" if m["needs_solvent"] else "")
        lines.append(f"| `{m['id']}` | {cell(title)} | {cell(parameter_text(m))} | {implementations} |")
    return lines


def method_tables():
    catalog = [m for m in tf.methods.catalog() if m["implemented"]]
    groups = [
        ("Base methods of a substance", lambda m: m["operates_on"] == "substance" and m["base"] and not m["water"]),
        ("Corrections of a substance", lambda m: m["operates_on"] == "substance" and not m["base"] and not m["dielectric"]),
        ("Equations of state of water", lambda m: m["water"]),
        ("Dielectric models of water", lambda m: m["dielectric"]),
        ("Base methods of a reaction", lambda m: m["operates_on"] == "reaction" and m["base"]),
        ("Other methods of a reaction", lambda m: m["operates_on"] == "reaction" and not m["base"]),
    ]
    lines = ["These tables come from the code. A parameter without a unit has no unit. A method can have aliases: old ids stay readable.", ""]
    for title, wanted in groups:
        lines += ["### " + title, ""] + table([m for m in catalog if wanted(m)]) + [""]
    return "\n".join(lines)


def with_tables(text):
    start, end = text.index(BEGIN) + len(BEGIN), text.index(END)
    return text[:start] + "\n" + method_tables() + "\n" + text[end:]


def test_the_method_tables_are_those_of_the_code():
    text = read_guide()
    assert text == with_tables(text), "run: python pytests/test_add_data_guide.py --write"
    for m in tf.methods.catalog():
        if m["implemented"]:
            assert "`" + m["id"] + "`" in text


def test_the_method_names_in_the_text_exist():
    text = read_guide()
    start = text.index(BEGIN)
    known = {m["id"] for m in tf.methods.catalog() if m["implemented"]}
    prose = text[:start]
    named = set(re.findall(r"`((?:Cp|Log[K]|Water|Reaction|Landau|Murnaghan|Tait|Birch|Volume|Constant|Ideal|PRSV|Peng|Soave|Sterner|Churakov|Cork|HKF|Solute|Anderson|Akinfiev|Johnson|Sverjensky|Fernandez|NASA|TP|Dolejs|Marshall|Ryzhenko)[A-Za-z0-9]+)`", prose))
    assert named and named <= known, sorted(named - known)


def blocks():
    """The code blocks in order: (kind, text, file). A YAML block with a comment before it is that file."""
    pattern = re.compile(r"(?:<!-- file: (?P<file>[^ ]+) -->\n)?```(?P<kind>[a-z]+)\n(?P<text>.*?)```", re.S)
    return [(m["kind"], m["text"], m["file"]) for m in pattern.finditer(read_guide())]


def test_the_examples_run(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    namespace = {}
    ran = 0
    for kind, text, name in blocks():
        if kind == "yaml":
            assert name, text[:60]
            (tmp_path / name).write_text(text)
        elif kind == "python":
            exec(compile(text, "add-data.md", "exec"), namespace)
            ran += 1
    assert ran >= 10


def test_the_aqueous_example_gives_the_properties_of_the_legacy_record(tmp_path, monkeypatch):
    legacy = tf.ThermoEngine(os.path.join(ROOT, "pytests", "test-aq17-gem-lma-thermofun.json"))
    want = legacy.thermoPropertiesSubstance(373.15, 1e6, "Ca+2")
    monkeypatch.chdir(tmp_path)
    text = next(t for k, t, n in blocks() if n == "aqueous.yaml")
    (tmp_path / "aqueous.yaml").write_text(text)
    got = tf.ThermoEngine("aqueous.yaml").thermoPropertiesSubstance(373.15, 1e6, "Ca+2")
    # the example has the parameters and the reference values of the record, rounded to about 5 digits
    assert got.gibbs_energy.val == pytest.approx(want.gibbs_energy.val, rel=1e-4)
    assert got.entropy.val == pytest.approx(want.entropy.val, rel=1e-2)


if __name__ == "__main__" and "--write" in sys.argv:
    with open(GUIDE) as f:
        written = with_tables(f.read())
    with open(GUIDE, "w") as f:
        f.write(written)
