# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# The code of the pages runs, in a new folder for each page, and the links of all pages lead somewhere.

import pathlib
import re
import shutil

import pytest
import thermofun as tf

ROOT = pathlib.Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs"
# these have their own tests
OWN_TEST = {"add-data.md", "add-a-method-as-data.md", "fit-parameters.md"}
PAGES = sorted(p for folder in ("tutorials", "how-to", "reference", "explanation") for p in (DOCS / folder).glob("*.md") if p.name not in OWN_TEST)
BLOCK = re.compile(r"(?:<!-- file: (?P<file>[^ ]+) -->\n)?```(?P<kind>[a-z+]+)\n(?P<text>.*?)```", re.S)
LINK = re.compile(r"\]\((?!https?:|#)([^)#]+)(#[^)]*)?\)")


@pytest.mark.parametrize("page", PAGES, ids=lambda p: p.name)
def test_the_code_of_a_page_runs(page, tmp_path, monkeypatch):
    text = page.read_text()
    if "fitting" in text:
        pytest.importorskip("scipy")
    shutil.copy(ROOT / "Resources" / "databases" / "aq17-thermofun.json", tmp_path)
    shutil.copytree(ROOT / "examples" / "v2", tmp_path / "examples" / "v2")
    monkeypatch.chdir(tmp_path)
    names = {}
    for match in BLOCK.finditer(text):
        if match["file"]:
            (tmp_path / match["file"]).write_text(match["text"])
        elif match["kind"] == "python":
            exec(compile(match["text"], page.name, "exec"), names)


@pytest.mark.parametrize("page", sorted(DOCS.rglob("*.md")) + [ROOT / "README.md"], ids=lambda p: p.name)
def test_the_links_of_a_page_lead_somewhere(page):
    for target, _ in LINK.findall(page.read_text()):
        assert (page.parent / target).exists(), target


def test_the_method_page_list_is_there():
    assert (DOCS / "reference" / "methods" / "index.md").exists()


def test_the_small_example_runs():
    engine = tf.ThermoEngine(str(ROOT / "examples" / "v2" / "minimal.yaml"))
    assert engine.thermoPropertiesReaction(298.15, 1e5, "Halite-dissolution").reaction_gibbs_energy.val < 1e5
