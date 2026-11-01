# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Tables of properties: the Python face of ThermoFun/Batch/Tables.h. The work is in the library.

    python -m thermofun.tables setup.yaml
    python -m thermofun.tables --list
    python -m thermofun.tables --template species-properties

In Python:

    results = tf.table("setup.yaml")
    results = tf.table(template="species-properties", database="aq17-thermofun.json", records=["Ca+2", "Calcite"])
    setup = tf.tables.template("species-properties", database=engine, records=["Ca+2"])
    setup.column("G", unit="J/mol", digits=1)
    results = setup.run()

See docs/how-to/make-tables.md.
"""

import argparse
import json
import os
import sys

from thermofun import PyThermoFun as _tf

SetupError = _tf.TableSetupError


class Results(dict):
    """The tables of a setup, by name."""

    def write(self):
        """Writes each table that has an output. Gives the paths."""
        return [t.write() for t in self.values() if t.output]

    @property
    def problems(self):
        """What could not be calculated: (table, record, T, P, message)."""
        return [(name, *p) for name, t in self.items() for p in t.problems]


def _database_of(database):
    return database.database() if isinstance(database, _tf.ThermoEngine) else database


class Setup:
    """A setup of tables, with its settings as functions. `table=` names one table; without it they change all tables."""

    def __init__(self, core):
        self.core = core

    def set(self, table="", **settings):
        """Sets anything of a table: records, reactions, columns, layout, digits, format, output, T, P, options, compare, ..."""
        for key, value in settings.items():
            if key == "database":
                self.core.setDatabase(_database_of(value) if not isinstance(value, (str, os.PathLike)) else os.fspath(value))
            else:
                self.core.setJson(key, json.dumps(value), table)
        return self

    def column(self, name, table="", **settings):
        """Changes a column in all tables that have it, for example column("G", unit="J/mol", digits=1)."""
        self.core.changeColumn(name, json.dumps(settings), table)
        return self

    def add_column(self, name, table="", **settings):
        """Adds a column."""
        self.core.addColumnFromJson(json.dumps({"property": name, **settings}), table)
        return self

    def remove_column(self, name, table=""):
        """Removes a column."""
        self.core.removeColumn(name, table)
        return self

    def to_yaml(self):
        """The setup as YAML text."""
        return self.core.toYaml()

    def save(self, path):
        """Writes the setup to a YAML file. Gives the path."""
        self.core.save(path)
        return path

    def run(self, database=None):
        """All tables, as Results."""
        if database is not None:
            self.set(database=database)
        return Results((t.name, t) for t in self.core.runCore().tables())


def load(setup):
    """A Setup from a path, YAML text or a dict."""
    if isinstance(setup, Setup):
        return setup
    if isinstance(setup, dict):
        plain = {k: v for k, v in setup.items() if k != "database" or isinstance(v, (str, os.PathLike))}
        made = Setup(_tf.TableSetup.fromText(json.dumps(plain, default=os.fspath)))
        if "database" in setup and "database" not in plain:
            made.set(database=setup["database"])
        return made
    text = os.fspath(setup)
    return Setup(_tf.TableSetup.fromFile(text) if os.path.isfile(text) else _tf.TableSetup.fromText(text))


def templates():
    """The names of the templates, each with what it makes."""
    return dict(_tf.TableSetup.templates())


def template(name, database=None, **settings):
    """A Setup from a template, with your records and any setting of Setup.set."""
    made = Setup(_tf.TableSetup.fromTemplate(name))
    if database is not None:
        made.set(database=database)
    return made.set(**settings) if settings else made


def run(setup, database=None):
    """All tables of a setup, as Results."""
    return load(setup).run(database)


def table(setup=None, template=None, database=None, **settings):
    """Tables from a setup, or from a template with your records. Gives Results."""
    made = globals()["template"](template) if template is not None else load(setup)
    if settings:
        made.set(**settings)
    return made.run(database)


def main(argv=None):
    parser = argparse.ArgumentParser(prog="python -m thermofun.tables", description="Makes tables of properties from a setup.")
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
            sys.stdout.write(_tf.TableSetup.templateText(args.template))
            return 0
        if not args.setup:
            parser.error("give a setup, --template or --list")
        results = run(args.setup)
        for path in results.write():
            print("wrote", path)
        for name, record, T, P, message in results.problems:
            print(f"{name}: {record} at T = {T}, P = {P}: {message}", file=sys.stderr)
        return 1 if results.problems else 0
    except (SetupError, OSError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
