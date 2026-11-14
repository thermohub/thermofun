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
import re
import sys

import thermofun as tf

DATA = "Resources/databases"
# examples from published sources, written by hand and checked by pytests/test_method_examples.py
EXAMPLES = "docs/reference/method-examples.yaml"


def published_examples(path=EXAMPLES):
    """For each method id: the hand-written example of docs/reference/method-examples.yaml."""
    if not os.path.exists(path):
        return {}
    import yaml  # only to generate the pages
    with open(path, encoding="utf-8") as f:
        return {x["method"]: x for x in yaml.safe_load(f)}

# the points tried for the calculation shown under an example: the first two that work are shown
POINTS = [(298.15, 1e5), (373.15, 1e7), (573.15, 1e8), (773.15, 1e8)]
HEADER = "thermohub: 2.0.0-draft.1\n"
# a water for an example that needs one and does not give it (as in pytests/test_method_examples.py)
WATER = """substances:
  H2O@:
    formula: H2O@
    class: water_solvent
    aggregate_state: aqueous
    standard_state_properties: {G: -237183.0, H: -285881.0, S: 69.923, Cp: 75.36053, V: {value: 1.8068397, unit: J/bar}}
    methods:
      - method: WaterIAPWS95
      - method: JohnsonNorton91
"""
SUBSTANCE_COLUMNS = (("G", "gibbs_energy"), ("H", "enthalpy"), ("S", "entropy"), ("Cp", "heat_capacity_cp"), ("V", "volume"))
REACTION_COLUMNS = (("logK", "log_equilibrium_constant"), ("dG", "reaction_gibbs_energy"), ("dH", "reaction_enthalpy"),
                    ("dS", "reaction_entropy"), ("dCp", "reaction_heat_capacity_cp"))
_engines = {}


def _engine(key, make):
    if key not in _engines:
        _engines[key] = make()
    return _engines[key]


def published_engine(example):
    """The engine of a published example, with a water if it has none."""
    def make():
        if "H2O@:" in example["record"]:
            return tf.ThermoEngine(tf.Database(HEADER + example["record"]))
        database = tf.Database(HEADER + WATER)
        database.appendData(HEADER + example["record"])
        return tf.ThermoEngine(database)
    return _engine(("published", example["method"]), make)


def calculated(engine, reaction, symbol, points):
    """Rows (T, P, values) for the first two points that give finite numbers."""
    import math
    columns = REACTION_COLUMNS if reaction else SUBSTANCE_COLUMNS
    rows = []
    for T, P in points:
        try:
            result = engine.thermoPropertiesReaction(T, P, symbol) if reaction else engine.thermoPropertiesSubstance(T, P, symbol)
            values = [getattr(result, attribute).val for _, attribute in columns]
        except Exception:
            continue
        if all(math.isfinite(v) for v in values):
            rows.append((T, P, values))
        if len(rows) == 2:
            break
    return rows


def calculation_lines(source, reaction, symbol, rows, note=None):
    """The code that calculates the example, and what it prints."""
    if not rows:
        return []
    columns = REACTION_COLUMNS if reaction else SUBSTANCE_COLUMNS
    call = "thermoPropertiesReaction" if reaction else "thermoPropertiesSubstance"
    points = ", ".join("(%s, %s)" % (_number(T), _number(P)) for T, P, _ in rows)
    lines = ["Calculation, with the record as above:", "", "```python", "import thermofun as tf", "", source,
             "for T, P in [%s]:  # K, Pa" % points,
             "    p = engine.%s(T, P, \"%s\")" % (call, symbol),
             "    print(T, P, %s)" % ", ".join("p.%s.val" % a for _, a in columns), "```", ""]
    if note:
        lines += [note, ""]
    units = "J/mol, J/mol, J/(mol K), J/(mol K), m3/mol" if not reaction else "no unit, J/mol, J/mol, J/(mol K), J/(mol K)"
    lines += ["Output, as `T  P  %s` (%s):" % ("  ".join(n for n, _ in columns), units), "", "```text"]
    for T, P, values in rows:
        lines.append("%s  %s  %s" % (_number(T), _number(P), "  ".join("%.5g" % v for v in values)))
    lines += ["```", ""]
    return lines


def _pressure(P):
    return "saturation" if P == 0 else "%s bar" % _number(P / 1e5)


def published_lines(example):
    lines = ["From %s. No bundled database has this method yet." % example["source"], ""]
    if example.get("note"):
        lines += [example["note"].strip(), ""]
    lines += ["```yaml"] + example["record"].rstrip("\n").split("\n") + ["```", ""]
    test = "`pytests/test_method_examples.py`"
    if example.get("checks"):
        lines += ["Published values, checked by %s:" % test, "", "| Record | T, K | P | Property | Published | Tolerance |", "|---|---|---|---|---|---|"]
        for c in example["checks"]:
            lines.append("| %s | %s | %s | %s | %s | %s |" % (c["symbol"], _number(c["T"]), _pressure(c["P"]), c["property"], _number(c["value"]), _number(c["tolerance"])))
        lines.append("")
    else:
        points = ", ".join("%s K and %s" % (_number(c["T"]), _pressure(c["P"])) for c in example.get("calculates", []))
        lines += ["%s checks that it calculates, at %s. There is no published value here to compare with." % (test, points), ""]
    import yaml
    symbol = (example.get("checks") or example.get("calculates"))[0]["symbol"]
    reaction = symbol not in (yaml.safe_load(example["record"]).get("substances") or {})
    given = [(c["T"], c["P"]) for c in example.get("checks", []) + example.get("calculates", [])]
    wanted = [(T, P) for T, P in dict.fromkeys(given) if P != 0] or POINTS
    rows = calculated(published_engine(example), reaction, symbol, wanted)
    note = None if "H2O@:" in example["record"] else "The record has no water: the calculation adds `H2O@` with `WaterIAPWS95`, as the test does."
    lines += calculation_lines('engine = tf.ThermoEngine("record.yaml")  # the record above, in a file', reaction, symbol, rows, note)
    return lines
NOTICE = "Generated. Do not edit by hand: run `python -m thermofun.docs generate docs/reference`."


def _cell(text):
    return str(text).replace("|", "\\|")


def _number(value):
    return "%.6g" % value


def _plain(value):
    return value["value"] if isinstance(value, dict) else value


def examples(data=DATA):
    """For each method id: (database file, section, symbol, application, has values, record). The first record with parameters, in a fixed order."""
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
                        found[key] = (name, section, symbol, application, bool(has_values), document[section][symbol])
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


def _ranges(names):
    """a1, a2, a3 as a1 to a3."""
    out, run = [], []
    def close():
        if len(run) > 2:
            out.append("`%s` to `%s`" % (run[0][0], run[-1][0]))
        else:
            out.extend("`%s`" % r[0] for r in run)
        run.clear()
    for name in names:
        match = re.fullmatch(r"(.*?)(\d+)", name)
        step = (name, match.group(1), int(match.group(2))) if match else (name, None, None)
        if run and not (match and run[-1][1] == step[1] and run[-1][2] + 1 == step[2]):
            close()
        run.append(step)
    close()
    return out


def bundled_calculation(example, data):
    name, section, symbol = example[0], example[1], example[2]
    path = os.path.join(data, name)
    engine = _engine(("bundled", path), lambda: tf.ThermoEngine(path))
    rows = calculated(engine, section == "reactions", symbol, POINTS)
    return calculation_lines('engine = tf.ThermoEngine("%s")' % name, section == "reactions", symbol, rows)


def method_page(spec, example, same=(), published=None, data=DATA):
    lines = ["# " + spec["id"], "", spec["title"] + ".", "", spec["summary"], ""]
    for other in same:
        lines += ["The id `%s` is the same method: the same calculation and parameters, under a name of the legacy files (%s). "
                  "It has no page of its own." % (other["id"], ", ".join("`%s`" % n for n in other["legacy_names"]) or "no legacy code"), ""]
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
    own = [p for p in spec["parameters"] if not p["constant"] and p["used"]]
    unused = [p["name"] for p in spec["parameters"] if not p["constant"] and not p["used"]]
    built_in = [p["name"] for p in spec["parameters"] if p["constant"]]
    if own:
        lines += ["## Parameters", "", "| Name | Unit of the source | SI unit | Required | Default | Description |", "|---|---|---|---|---|---|"]
        for p in own:
            default = "" if p["default"] is None else _number(p["default"])
            lines.append("| %s | %s | %s | %s | %s | %s |" % (p["name"], _cell(p["source_unit"]), _cell(p["si_unit"]), "yes" if p["required"] else "no", default, _cell(p["description"])))
        lines.append("")
    if unused:
        lines += ["## Parameters", "", "None. The method reads only the reference values of the reaction, and makes its terms of log K from them "
                  "(see above).", "",
                  "Not read: %s. Legacy files write these coefficients of log K(T) and dCp(T) with every method of log K, as GEMS calculated "
                  "them; a file can keep them, and they change nothing." % ", ".join(_ranges(unused)), ""]
    if built_in:
        lines += ["## Built-in constants", "", "A record does not give these: ThermoFun reads them from `ThermoFun/Methods/ParameterSets`. "
                  "A record can replace one like a parameter, inline or with a parameter set of the same method, unless the text below says it cannot.", ""]
        for s in spec["constant_sets"]:
            if s["implementation"]:
                lines += ["Implementation `%s`." % s["implementation"], ""]
            if not s["default"]:
                lines += ["Variant `%s`: a record chooses it with `parameter_set`." % s["symbol"], ""]
            lines += ["Units of the values: %s. They are not converted." % s["formula_units"], "", s["description"], "", "Source: %s." % s["reference"], ""]
        lines += ["Names: %s." % ", ".join(_ranges(built_in)), ""]
    lines += ["## Example", ""]
    if published and not example:
        lines += published_lines(published)
    elif example and unused:
        # the method reads the reference values of the reaction, not the coefficients that the legacy file gave with it
        values = example[5].get("reaction_properties", {})
        shown = ["  %s: %s" % (n, _number(_plain(values[n]))) for n in ("logK", "dG", "dH", "dS", "dCp") if n in values]
        lines += ["From %s in `%s`: the reference values it reads, and the method." % (example[2], example[0]), "", "```yaml", "reaction_properties:"]
        lines += shown + ["methods:", "  - method: " + spec["id"], "```", ""]
        lines += bundled_calculation(example, data)
    elif example:
        lines += ["From %s in `%s`." % (example[2], example[0]), "", "```yaml"] + example_lines(spec, example) + ["```", ""]
        lines += bundled_calculation(example, data)
    else:
        lines += ["No bundled record uses this method.", ""]
    return "\n".join(lines)


def _key(spec):
    return ("substances" if spec["operates_on"] == "substance" else "reactions", spec["id"])


def groups(specs):
    """The groups in the order of the catalog, each with its methods sorted by id."""
    out = {}
    for s in specs:
        if not s["group"]:
            raise ValueError("method %s has no group in the catalog" % s["id"])
        out.setdefault(s["group"], []).append(s)
    return {g: sorted(members, key=lambda s: s["id"]) for g, members in out.items()}


def group_file(group):
    """The page of a group: the anchor MyST gives its heading, for example reactions-log-k."""
    return re.sub(r"[^a-z0-9 -]", "", group.lower()).strip().replace(" ", "-")


def _table(members):
    lines = ["| Method | Title | Kind | Classes | Implementations |", "|---|---|---|---|---|"]
    for s in members:
        lines.append("| [%s](%s.md) | %s | %s | %s | %s |" % (s["id"], s["id"], _cell(s["title"]), "base" if s["base"] else "correction",
                                                         ", ".join(s["classes"]) or "any", ", ".join(s["implementations"])))
    return lines


def index_page(specs):
    """All methods, one section for each group; the side bar of the site goes through the page of each group."""
    lines = ["# Methods", "", NOTICE, ""]
    for g, members in groups(specs).items():
        lines += ["## " + g, ""] + _table(members) + [""]
    # the pages of the site (Sphinx) are those of a toctree; hidden: the tables above are the list
    lines += ["```{toctree}", ":hidden:", ""] + [group_file(g) for g in groups(specs)] + ["```", ""]
    return "\n".join(lines)


def group_page(group, members):
    lines = ["# " + group, "", NOTICE, ""] + _table(members)
    lines += ["", "```{toctree}", ":hidden:", ""] + [s["id"] for s in members] + ["```", ""]
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


def _type(node):
    """The type of a schema node, in words."""
    if not isinstance(node, dict) or not node:
        return "any"
    if "$ref" in node:
        name = node["$ref"].split("/")[-1]
        return "[%s](#%s)" % (name, name)
    if "allOf" in node:   # a file of one record: record_type and the record
        parts = [n for n in node["allOf"] if "$ref" in n]
        const = [n["properties"]["record_type"]["const"] for n in node["allOf"] if "properties" in n and "record_type" in n["properties"]]
        return "record_type `%s` and %s" % (const[0], _type(parts[0])) if const and parts else "all of the parts"
    if "const" in node:
        return "`%s`" % node["const"]
    if "enum" in node:
        return ", ".join("`%s`" % v for v in node["enum"])
    if "oneOf" in node:
        return " or ".join(_type(n) for n in node["oneOf"])
    t = node.get("type", "any")
    if t == "array" and "items" in node:
        return "list of " + _type(node["items"])
    if t == "object" and isinstance(node.get("additionalProperties"), dict):
        return "mapping to " + _type(node["additionalProperties"])
    return " or ".join(t) if isinstance(t, list) else t


def schema_page():
    from thermofun import schema
    s = schema.schema()
    defs = s["$defs"]
    order = ["document", "single_record", "thermodataset", "element", "substance", "reaction", "method_application", "quantity",
             "uncertainty", "convention", "reference_conditions", "applicability", "units", "covariances", "parameters",
             "method_definition", "parameter_set", "data_source"]
    lines = ["# The JSON Schema of the v2 format", "", NOTICE, "",
             "The schema is `ThermoFun/IO/V2/thermohub.schema.json` (JSON Schema 2020-12), also installed with the headers. "
             "A file that is valid there is read without a warning. A key that starts with `_` is allowed and ignored. "
             "[The v2 format](format-v2.md) and [Add data](../how-to/add-data.md) explain the fields.", ""]
    for name in order + sorted(set(defs) - set(order)):
        node = defs[name]
        lines += ["## %s" % name, ""]
        if node.get("description"):
            lines += [node["description"], ""]
        if "properties" in node:
            required = set(node.get("required", []))
            lines += ["| Key | Type | Required | Note |", "|---|---|---|---|"]
            for key, sub_node in node["properties"].items():
                lines.append("| `%s` | %s | %s | %s |" % (key, _cell(_type(sub_node)), "yes" if key in required else "",
                                                          _cell(sub_node.get("description", "") if isinstance(sub_node, dict) else "")))
            lines.append("")
        elif "oneOf" in node:
            lines += ["One of: " + "; ".join(_cell(_type(n)) if "properties" not in n else
                                            "an object with " + ", ".join("`%s`" % k for k in n["properties"]) for n in node["oneOf"]), ""]
        else:
            lines += ["Type: " + _cell(_type(node)), ""]
    return "\n".join(lines)


def pages(data=DATA):
    """The files as a dictionary: path relative to the folder of the reference -> text."""
    catalog = list(tf.methods.catalog())  # the order of the catalog gives the order of the groups
    specs = [s for s in catalog if not s["same_as"]]  # another name of a method: a note on the page of that method
    found = examples(data)
    written = published_examples()
    out = {"methods/index.md": index_page(specs), "diagnostics.md": diagnostics_page(), "engine-preferences.md": preferences_page(),
           "format-v2-schema.md": schema_page()}
    for s in specs:
        out["methods/%s.md" % s["id"]] = method_page(s, found.get(_key(s)), [o for o in catalog if o["same_as"] == s["id"]], written.get(s["id"]), data)
    for g, members in groups(specs).items():
        out["methods/%s.md" % group_file(g)] = group_page(g, members)
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
