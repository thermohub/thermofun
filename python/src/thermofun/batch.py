# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Batch calculations (decision 0032): many records at many conditions at once. The Python face of
ThermoFun/Batch/BatchSetup.h; the work is in the library.

    python -m thermofun.batch setup.yaml
    python -m thermofun.batch --list
    python -m thermofun.batch --template blank

In Python:

    results = tf.batch("setup.yaml")
    results = tf.batch(template="species-properties", database="aq17-thermofun.json", substances=["Ca+2", "Calcite"])
    setup = tf.template("blank")
    setup.set("/database", "aq17-thermofun.json")
    results = setup.run()
    results["species"].to_text("md")

A setup has four blocks: database and engine (with what), conditions (where: T and P), results (what: one entry for each
result table) and output (how to give the results). See docs/reference/batch.md.
"""

import argparse
import json
import os
import sys
import types

from thermofun import PyThermoFun as _tf

BatchSetupError = _tf.BatchSetupError

# keywords of tf.batch and tf.template: where each goes in the setup
_EACH_RESULT = ("substances", "reactions", "properties", "compare", "compare_difference")
_BLOCKS = {"conditions": ("T", "P", "points", "vary_first"),
           "output": ("file", "file_format", "number_format", "digits", "units", "layout", "grid_across", "rows_by", "csv_separator", "undefined", "provenance")}


class Results(dict):
    """The result tables of a batch calculation, by name: each a ResultTable."""

    def write(self):
        """Writes each result table that has output.file. Gives the paths."""
        return [t.write() for t in self.values() if t.output]

    @property
    def problems(self):
        """What could not be calculated: (result, record, T, P, message)."""
        return [(name, *p) for name, t in self.items() for p in t.problems]


def _database_of(database):
    return database.database() if isinstance(database, _tf.ThermoEngine) else database


class BatchSetup:
    """A batch setup: a dict, a YAML file, or a template, with its values set by path. It prints as YAML."""

    def __init__(self, core):
        self.core = core

    def set(self, path, value):
        """Sets a value of the setup by its path, for example set("/results/0/name", "species") or set("/conditions/T", [25, 100]).
        The path "/database" also takes a Database or a ThermoEngine."""
        if path == "/database" and not isinstance(value, (str, os.PathLike)):
            self.core.setDatabase(_database_of(value))
        else:
            self.core.setJson(path, json.dumps(value, default=os.fspath))
        return self

    def get(self, path):
        """The value at a path, as Python objects."""
        return json.loads(self.core.getJson(path))

    def fill(self, **settings):
        """Sets several values by name: database, substances, reactions, properties, compare, compare_difference (each result),
        T, P, points, vary_first (conditions), engine, and the keys of output (file, layout, digits, units, ...)."""
        for key, value in settings.items():
            if key == "database":
                self.set("/database", value)
            elif key in _EACH_RESULT:
                for i in range(len(self.get("/results"))):
                    self.set(f"/results/{i}/{key}", value)
            elif key == "engine":
                for name, part in value.items():
                    self.set(f"/engine/{name}", part)
            else:
                block = next((b for b, keys in _BLOCKS.items() if key in keys), None)
                if block is None:
                    raise BatchSetupError(f"tf.batch has no keyword {key}. Use one of: database, engine, {', '.join(_EACH_RESULT)}, "
                                          + ", ".join(k for keys in _BLOCKS.values() for k in keys) + ", or set(path, value).")
                self.set(f"/{block}/{key}", value)
                # a key at the top is for every result: the same key of a result gives way
                for i, result in enumerate(self.get("/results") if "results" in self.get("") else []):
                    if key in result.get(block, {}):
                        del result[block][key]
                        self.set(f"/results/{i}/{block}", result[block])
        return self

    def use(self, name):
        """Puts a template under the settings: what it says replaces what the setup says."""
        self.core.useTemplate(name)
        return self

    def property(self, name, result="", **settings):
        """Changes a property in all results that have it, for example property("G", unit="J/mol", digits=1)."""
        self.core.changeProperty(name, json.dumps(settings), result)
        return self

    def add_property(self, name, result="", **settings):
        """Adds a property to all results, or to one."""
        self.core.addPropertyFromJson(json.dumps({"name": name, **settings}), result)
        return self

    def remove_property(self, name, result=""):
        """Removes a property."""
        self.core.removeProperty(name, result)
        return self

    def batch(self, result=""):
        """One result as a calculation of ThermoBatch, the 0.x batch API. Its .run() gives the Output, and its .write() writes the file."""
        return self.core.batch(result)

    def apply_to(self, batch, result=""):
        """Sets the units, digits and preferences of a ThermoBatch that you have, from one result."""
        self.core.applyTo(batch, result)
        return batch

    def to_yaml(self):
        """The setup as YAML text."""
        return self.core.toYaml()

    def __str__(self):
        return self.to_yaml()

    def __repr__(self):
        return "<BatchSetup>\n" + self.to_yaml()

    def save(self, path):
        """Writes the setup to a YAML file. Gives the path."""
        self.core.save(os.fspath(path))
        return path

    def run(self, database=None):
        """Calculates all results. Gives Results."""
        if database is not None:
            self.set("/database", database)
        return Results((t.name, t) for t in self.core.runCore().tables())


def load(setup):
    """A BatchSetup from a path, YAML text or a dict."""
    if isinstance(setup, BatchSetup):
        return setup
    if isinstance(setup, dict):
        plain = {k: v for k, v in setup.items() if k != "database" or isinstance(v, (str, os.PathLike))}
        made = BatchSetup(_tf.BatchSetupCore.fromText(json.dumps(plain, default=os.fspath)))
        if "database" in setup and "database" not in plain:
            made.set("/database", setup["database"])
        return made
    text = os.fspath(setup)
    return BatchSetup(_tf.BatchSetupCore.fromFile(text) if os.path.isfile(text) else _tf.BatchSetupCore.fromText(text))


def templates():
    """The batch templates, each with what it makes."""
    return dict(_tf.BatchSetupCore.templates())


def template_source(name):
    """Where a template comes from: the path of its file, or "library"."""
    return _tf.BatchSetupCore.templateSource(name)


def template_folders():
    """The folders of THERMOFUN_TEMPLATE_PATH."""
    return _tf.BatchSetupCore.templateFolders()


def select_substances(database, rule):
    """The substances that a rule chooses, for example {"class": "aqueous_solute", "elements": ["Ca"]}."""
    return _tf.selectSubstances(_database_of(database), rule)


def select_reactions(database, rule):
    """The reactions that a rule chooses, for example {"involving": ["Calcite"]}."""
    return _tf.selectReactions(_database_of(database), rule)


def template(name, **settings):
    """A BatchSetup from a template or a list of templates, with the keywords of fill (database, substances, T, ...)."""
    names = [name] if isinstance(name, str) else list(name)
    made = BatchSetup(_tf.BatchSetupCore.fromTemplate(names[0]))
    for other in names[1:]:
        made.use(other)
    return made.fill(**settings)


def batch(setup=None, template=None, **settings):
    """Runs a batch calculation: a setup (a path, YAML text, a dict or a BatchSetup), or a template with the keywords of fill.
    Gives Results."""
    made = globals()["template"](template) if template is not None else load(setup)
    return made.fill(**settings).run()


def main(argv=None):
    parser = argparse.ArgumentParser(prog="python -m thermofun.batch", description="Runs a batch calculation from a setup.")
    parser.add_argument("setup", nargs="?", help="a YAML file")
    parser.add_argument("--template", help="print this template")
    parser.add_argument("--list", action="store_true", help="list the templates")
    args = parser.parse_args(argv)
    try:
        if args.list:
            for name, what in templates().items():
                print(f"{name}: {what}")
            return 0
        if args.template:
            sys.stdout.write(_tf.BatchSetupCore.templateText(args.template))
            return 0
        if not args.setup:
            parser.error("give a setup, --template or --list")
        results = load(args.setup).run()
        for path in results.write():
            print("wrote", path)
        for name, record, T, P, message in results.problems:
            print(f"{name}: {record} at T = {T}, P = {P}: {message}", file=sys.stderr)
        return 1 if results.problems else 0
    except (BatchSetupError, OSError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


class _CallableModule(types.ModuleType):
    """tf.batch is this module and the function batch(): `import thermofun.batch` puts the module where the function is."""

    def __call__(self, *args, **kwargs):
        return batch(*args, **kwargs)


sys.modules[__name__].__class__ = _CallableModule

if __name__ == "__main__":
    sys.exit(main())
