<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Writing rules

The pages follow the parts of ASD-STE100 Simplified Technical English that matter here.

1. One word has one meaning. Use the words of the [glossary](../glossary.md).
2. Write short sentences: at most 20 words in a step, at most 25 in a description.
3. Write one instruction in a sentence, in the imperative.
4. Use the active voice and the present tense.
5. Give the unit each time that a number has one.
6. Use lists for steps. Number them when the order counts.
7. Write at most six sentences in a paragraph, on one topic.
8. Do not use idioms or humour. Explain an abbreviation at its first use.
9. Write method ids, parameters and file names in `code`.

Each how-to starts with **You need:** and ends with **Check:**. Each tutorial is one task for 15 minutes, and says what the reader learns.

The code of tutorials and how-tos runs in `pytests/test_docs_pages.py`, and asserts its result. A block after `<!-- file: name -->` is written to that file first.

A message in the code says what is wrong. Its fix says what to do. Each is one sentence.
