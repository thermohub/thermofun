# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Writes the generated reference pages.

    python -m thermofun.docs generate docs/reference [--check] [--data Resources/databases]

The pages are methods/index.md, one page for each method, diagnostics.md and engine-preferences.md. The examples come from the
records of the bundled databases. With --check nothing is written: the command compares the files and exits with 1 if one differs.
"""

import argparse
import difflib
import json
import os
import sys

import thermofun as tf

DATA = "Resources/databases"
NOTICE = "Generated. Do not edit by hand: run `python -m thermofun.docs generate docs/reference`."


def _cell(text):
    return str(text).replace("|", "\\|")


def _number(value):
    return "%.6g" % value


def _plain(value):
    return value["value"] if isinstance(value, dict) else value


def examples(data=DATA):
    """For each method id: (database file, section, symbol, application). The first record with parameters, in a fixed order."""
    found = {}
    if not os.path.isdir(data):
        return found
    for name in sorted(os.listdir(data)):
        if not name.endswith(".json"):
            continue
        document = json.loads(tf.Database(os.path.join(data, name)).toJson(True))
        for section in ("substances", "reactions"):
            for symbol in sorted(document.get(section, {})):
                for application in document[section][symbol].get("methods", []):
                    method = application["method"]
                    has_values = application.get("parameters") or application.get("segments")
                    key = (section, method)
                    if key not in found or (has_values and not found[key][4]):
                        found[key] = (name, section, symbol, application, bool(has_values))
    return found


def example_lines(spec, example):
    """The example of a method as lines of YAML, an item of the list `methods`, as the writer gives it: in the units of the source."""
    application = example[3]
    lines = ["- method: " + spec["id"]]

    def block(values, indent):
        return ["%s%s: %s" % (indent, name, _number(_plain(value))) for name, value in values.items() if isinstance(_plain(value), (int, float))]

    if application.get("units"):
        lines.append("  units: {%s}" % ", ".join("%s: %s" % item for item in application["units"].items()))
    if application.get("segments"):
        lines.append("  segments:")
        for segment in application["segments"][:1]:
            first = True
            for key in ("t_min", "t_max"):
                if key in segment:
                    lines.append("%s%s: %s" % ("    - " if first else "      ", key, _number(_plain(segment[key]))))
                    first = False
            lines.append(("    - " if first else "      ") + "parameters:")
            lines.extend(block(segment.get("parameters", {}), "        "))
    elif application.get("parameters"):
        values = block(application["parameters"], "    ")
        if values:
            lines.append("  parameters:")
            lines.extend(values)
    return lines


def method_page(spec, example):
    lines = ["# " + spec["id"], "", spec["title"] + ".", "", spec["summary"], ""]
    if spec["citation"]:
        lines += ["Source: " + spec["citation"] + ".", ""]
    needs = ", ".join(n for n, on in (("the solvent", spec["needs_solvent"]),) if on) or "nothing from outside its record"
    lines += ["| | |", "|---|---|",
              "| Works on | %s |" % spec["operates_on"],
              "| Kind | %s |" % ("base method" if spec["base"] else "correction"),
              "| Classes | %s |" % (", ".join(spec["classes"]) or "any"),
              "| Needs | %s |" % needs,
              "| Temperature segments | %s |" % ("yes" if spec["piecewise"] else "no"),
              "| Has code | %s |" % ("yes" if spec["implemented"] else "no: the name is known, and the method does nothing")]
    if spec["native_convention"]:
        lines.append("| Convention of its results | %s |" % spec["native_convention"])
    if spec["legacy_names"]:
        lines.append("| Legacy names | %s |" % ", ".join("`%s`" % n for n in spec["legacy_names"]))
    if spec["aliases"]:
        lines.append("| Other names | %s |" % ", ".join("`%s`" % n for n in spec["aliases"]))
    lines.append("")
    if len(spec["implementations"]) > 1:
        lines += ["## Implementations", "",
                  "%s. The default is `%s`. Use another only to reproduce the results of that code." %
                  (", ".join("`%s`" % i for i in spec["implementations"]), spec["implementations"][0]), ""]
    if spec["parameters"]:
        lines += ["## Parameters", "", "| Name | Unit of the source | SI unit | Required | Default | Description |", "|---|---|---|---|---|---|"]
        for p in spec["parameters"]:
            default = "" if p["default"] is None else _number(p["default"])
            lines.append("| %s | %s | %s | %s | %s | %s |" % (p["name"], _cell(p["source_unit"]), _cell(p["si_unit"]), "yes" if p["required"] else "no", default, _cell(p["description"])))
        lines.append("")
    lines += ["## Example", ""]
    if example:
        lines += ["From %s in `%s`." % (example[2], example[0]), "", "```yaml"] + example_lines(spec, example) + ["```", ""]
    else:
        lines += ["No bundled record uses this method.", ""]
    return "\n".join(lines)


def _key(spec):
    return ("substances" if spec["operates_on"] == "substance" else "reactions", spec["id"])


def index_page(specs):
    lines = ["# Methods", "", NOTICE, "", "| Method | Title | Kind | Works on | Classes | Implementations |", "|---|---|---|---|---|---|"]
    for s in specs:
        lines.append("| [%s](%s.md) | %s | %s | %s | %s | %s |" % (s["id"], s["id"], _cell(s["title"]), "base" if s["base"] else "correction", s["operates_on"],
                                                                   ", ".join(s["classes"]) or "any", ", ".join(s["implementations"])))
    lines.append("")
    return "\n".join(lines)


def diagnostics_page():
    lines = ["# Diagnostics", "", NOTICE, "", "| Code | Severity | Meaning | What to do |", "|---|---|---|---|"]
    for d in tf.diagnosticCatalog():
        lines.append("| %s | %s | %s | %s |" % (d.code, str(d.severity).split(".")[-1], _cell(d.meaning), _cell(d.fix)))
    lines.append("")
    return "\n".join(lines)


def _options(cls, words):
    lines = ["| Option | Default | Valid words | Description |", "|---|---|---|---|"]
    default = cls()
    for name in sorted(n for n in dir(default) if not n.startswith("_")):
        attribute = getattr(cls, name)
        value = getattr(default, name)
        if callable(value):
            continue
        shown = str(value).split(".")[-1] if name in words else repr(value)
        doc = (attribute.__doc__ or "").strip().splitlines()
        lines.append("| %s | %s | %s | %s |" % (name, _cell(shown), ", ".join(words.get(name, [])), _cell(doc[0] if doc else "")))
    return lines


def preferences_page():
    words = tf.optionWords()
    return "\n".join(["# Options of the engine", "", NOTICE, "", "## EnginePreferences", ""] + _options(tf.EnginePreferences, words) +
                     ["", "## EngineConventions", ""] + _options(tf.EngineConventions, words) + [""])


def pages(data=DATA):
    """The files as a dictionary: path relative to the folder of the reference -> text."""
    specs = sorted(tf.methods.catalog(), key=lambda s: (s["operates_on"], s["id"]))
    found = examples(data)
    out = {"methods/index.md": index_page(specs), "diagnostics.md": diagnostics_page(), "engine-preferences.md": preferences_page()}
    for s in specs:
        out["methods/%s.md" % s["id"]] = method_page(s, found.get(_key(s)))
    return out


def generate(folder, data=DATA):
    for path, text in pages(data).items():
        full = os.path.join(folder, path)
        os.makedirs(os.path.dirname(full), exist_ok=True)
        with open(full, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)


def check(folder, data=DATA):
    """The list of problems: a missing, a different or an unknown file."""
    wanted = pages(data)
    problems = []
    for path, text in wanted.items():
        full = os.path.join(folder, path)
        if not os.path.exists(full):
            problems.append("missing: " + path)
            continue
        with open(full, encoding="utf-8", newline="") as f:
            have = f.read()
        if have != text:
            diff = difflib.unified_diff(have.splitlines(), text.splitlines(), "committed/" + path, "generated/" + path, lineterm="", n=1)
            problems.append("different: %s\n%s" % (path, "\n".join(list(diff)[:12])))
    methods = os.path.join(folder, "methods")
    if os.path.isdir(methods):
        for name in sorted(os.listdir(methods)):
            if "methods/" + name not in wanted:
                problems.append("unknown file: methods/" + name)
    return problems


def main(argv=None):
    parser = argparse.ArgumentParser(prog="python -m thermofun.docs")
    parser.add_argument("command", choices=["generate"])
    parser.add_argument("folder")
    parser.add_argument("--check", action="store_true", help="do not write; exit with 1 if a file is not up to date")
    parser.add_argument("--data", default=DATA, help="the folder of the bundled databases, for the examples")
    args = parser.parse_args(argv)
    if args.check:
        problems = check(args.folder, args.data)
        for p in problems:
            print(p)
        return 1 if problems else 0
    generate(args.folder, args.data)
    return 0


if __name__ == "__main__":
    sys.exit(main())
