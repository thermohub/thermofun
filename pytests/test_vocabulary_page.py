# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""docs/reference/vocabulary.yaml (decisions 0030-0032): the allowed words it lists are those of the library, and each word of
the glossary has a meaning and a source. The pages it gives are checked by test_tools (docs.check)."""

import pathlib
import re

import pytest
import yaml

import thermofun as tf

ROOT = pathlib.Path(__file__).resolve().parent.parent
WORDS = yaml.safe_load((ROOT / "docs" / "reference" / "vocabulary.yaml").read_text())
VOCABULARY_CPP = (ROOT / "ThermoFun" / "Model" / "Vocabulary.cpp").read_text()
FIELDS = [f for s in WORDS["allowed_words"]["sections"] for f in s["fields"] if "check" in f]


def listed(field):
    return set(re.findall(r"`([^`]+)`", field["words"]))


@pytest.mark.parametrize("field", FIELDS, ids=lambda f: f["field"])
def test_the_allowed_words_are_those_of_the_library(field):
    check = field["check"]
    if "options" in check:
        words = {w for w in tf.optionWords()[check["options"]] if " " not in w}
        assert words <= listed(field), (field["field"], words - listed(field))
    else:
        # the words of class and aggregate_state are in the table of Model/Vocabulary.cpp
        for word in listed(field):
            assert f'"{word}"' in VOCABULARY_CPP, (field["field"], word)


def test_each_word_of_the_glossary_has_a_meaning_and_a_source_once():
    words = [w["word"] for w in WORDS["glossary"]["words"]]
    assert len(words) == len(set(words))
    for w in WORDS["glossary"]["words"]:
        assert w["meaning"].strip() and w["source"].strip(), w["word"]


@pytest.mark.parametrize("word", ["Route", "Contribution", "Terms", "Variables", "Conversion", "Batch calculation", "Batch setup", "Batch template",
                                  "Result table", "Provenance", "Model parameter", "Water anchor", "Electron", "Hydrogen ion", "Standard state"])
def test_the_words_of_the_decisions_are_there(word):
    assert word in [w["word"] for w in WORDS["glossary"]["words"]]
