# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Converts a database file between the legacy format and the v2 format.

    python -m thermofun.convert INPUT OUTPUT [--to v2|legacy] [--format json|yaml] [--split] [--exact] [--report report.md]

INPUT is a legacy or v2 file or folder (JSON or YAML). The C++ readers and writers do the work. With --split the output is a
dataset folder with one file for each record (the layout of a ThermoHub dataset). A v2 file has short decimals, unless --exact is given.
"""

import argparse
import collections
import json
import os
import re
import sys

import thermofun as tf


def _write(text, path):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
        if not text.endswith("\n"):
            f.write("\n")


def _text(database, fmt, readable):
    return database.toYaml(readable) if fmt == "yaml" else database.toJson(readable)


def _safe(symbol):
    return re.sub(r"[^A-Za-z0-9_.+()-]", "_", symbol)


def _write_split(data, folder, fmt, readable):
    ext = ".yaml" if fmt == "yaml" else ".json"
    head = {"thermohub": data["thermohub"]}
    first = dict(head)
    first["thermodataset"] = data.get("thermodataset", {})
    # the project records are in the file of the dataset
    for section in ("methods", "parameter_sets", "data_sources"):
        if section in data:
            first[section] = data[section]
    _write(_text(tf.Database(json.dumps(first)), fmt, readable), os.path.join(folder, "thermodataset" + ext))
    for section in ("elements", "substances", "reactions"):
        for symbol, record in data.get(section, {}).items():
            one = tf.Database(json.dumps(dict(head, **{section: {symbol: record}})))
            _write(_text(one, fmt, readable), os.path.join(folder, section, _safe(symbol) + ext))


def report_text(database, input_path, to):
    """The migration report as Markdown."""
    load = list(database.loadDiagnostics()) + list(tf.legacyFileDiagnostics(input_path))
    check = list(database.validate())
    out = ["# Conversion report", "", "## Summary", "",
           "- input: `%s`" % input_path, "- output format: %s" % to,
           "- elements: %d, substances: %d, reactions: %d" % (database.numberOfElements(), database.numberOfSubstances(),
                                                            database.numberOfReactions()),
           "- methods: %d, parameter sets: %d, data sources: %d" % (len(database.methods()), len(database.parameterSets()),
                                                                   len(database.dataSources())), ""]

    def table(title, items, empty):
        out.extend(["## " + title, ""])
        if not items:
            out.extend([empty, ""])
            return
        counts = collections.OrderedDict()
        for d in items:
            entry = counts.setdefault(d.code, [0, []])
            entry[0] += 1
            if d.record and len(entry[1]) < 5 and d.record not in entry[1]:
                entry[1].append(d.record)
        out.extend(["| Code | Count | First records |", "|---|---|---|"])
        for code in sorted(counts):
            out.append("| %s | %d | %s |" % (code, counts[code][0], ", ".join(counts[code][1])))
        out.append("")

    table("Quirks applied", [d for d in load if d.code.startswith("LEG-QRK")], "None.")
    table("Unread legacy keys", [d for d in load if d.code in ("LEG-KEY-001", "LEG-KEY-002")], "None.")
    table("Unresolved references", [d for d in check if d.code.startswith("V2-REF")], "None.")
    table("Other problems of the data", [d for d in check if not d.code.startswith("V2-REF")], "None.")
    used = collections.Counter()
    for s in database.getSubstancesList():
        for m in database.getSubstance(s).record().methods:
            used[m.method] += 1
    for r in database.getReactionsList():
        for m in database.getReaction(r).record().methods:
            used[m.method] += 1
    out.extend(["## Methods used", "", "| Method | Records |", "|---|---|"])
    out.extend("| %s | %d |" % (m, n) for m, n in sorted(used.items()))
    out.extend(["", "## Uncertainty convention notice", "",
                "A legacy `error` is read as a standard uncertainty (coverage factor 1). The legacy format does not say if an error is a "
                "standard uncertainty, an expanded one or a confidence interval. Check the source of the data before you use the "
                "uncertainties.", ""])
    return "\n".join(out)


def convert_file(input, output, to="v2", fmt="json", split=False, report=None, exact=False):
    """Converts a file. Returns the Diagnostics of the load."""
    if to not in ("v2", "legacy"):
        raise ValueError("to must be v2 or legacy")
    database = tf.Database(str(input))
    if to == "legacy":
        if split or fmt == "yaml":
            raise ValueError("The legacy format is one JSON file")
        _write(database.toLegacyJson(True), str(output))
    elif split:
        _write_split(json.loads(database.toJson(not exact)), str(output), fmt, not exact)
    else:
        _write(_text(database, fmt, not exact), str(output))
    if report:
        _write(report_text(database, str(input), to), str(report))
    return database.loadDiagnostics()


def main(argv=None):
    parser = argparse.ArgumentParser(prog="python -m thermofun.convert")
    parser.add_argument("input")
    parser.add_argument("output")
    parser.add_argument("--to", choices=["v2", "legacy"], default="v2")
    parser.add_argument("--format", choices=["json", "yaml"], default="json", dest="fmt")
    parser.add_argument("--split", action="store_true", help="write a dataset folder with one file for each record")
    parser.add_argument("--exact", action="store_true", help="write each number so that it reads back to the same double")
    parser.add_argument("--report", help="write the migration report (Markdown)")
    args = parser.parse_args(argv)
    convert_file(args.input, args.output, args.to, args.fmt, args.split, args.report, args.exact)
    return 0


if __name__ == "__main__":
    sys.exit(main())
