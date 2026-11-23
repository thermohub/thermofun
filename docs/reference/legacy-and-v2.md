<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Two formats

ThermoFun reads two formats into the same records. The v2 format is the documented one. It is YAML or JSON, with methods and parameters by name, and units.

The legacy format is the JSON of the earlier versions. It has numbered methods and parameters by position. ThermoFun keeps it, so that existing files and programs work.

A record read from a legacy file and written back keeps its text if nothing changed.

| Version | What changes |
|---|---|
| 1.0 | v2 is the documented format. The old data calls run through `Compat/`, and are marked as deprecated. |
| 1.x | Reading a legacy file logs a message that names `python -m thermofun.convert`. |
| 2.0 | `Compat/` goes. The legacy reader and writer stay while legacy files are in use. |
