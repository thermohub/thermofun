# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Check that the layers of ThermoFun include only what they may include.

    python tests/layers/check_includes.py

The rules are those of the plan (02 section 1). A file in a layer folder of ThermoFun/ may include headers of the
layers listed for it, the standard library and the external libraries allowed for it. Known violations of old
files are listed in known_violations.txt with the step that removes them; any other violation, or a listed one
that is gone, makes the exit code 1.
"""

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC = ROOT / "ThermoFun"
KNOWN = pathlib.Path(__file__).resolve().parent / "known_violations.txt"

LAYERS = ["Common", "Model", "Conventions", "Methods", "Engine", "IO", "Compat"]
CATALOG = {"Methods/MethodCatalog.h", "Methods/MethodSpec.h"}  # the method specs, which IO and Compat may read

# layer -> layers it may include (its own layer always)
ALLOWED = {
    "Common": set(),
    "Model": {"Common"},
    "Conventions": {"Model", "Common"},
    "Methods": {"Model", "Conventions", "Common"},
    "Engine": {"Methods", "Model", "Conventions", "Common"},
    "IO": {"Model", "Common", "catalog"},
    "IO/Legacy": {"IO", "Model", "Common", "catalog", "legacy-enums"},
    "Compat": {"Model", "Common", "catalog", "legacy-enums", "public", "IO/Legacy"},
}
JSON_YAML = re.compile(r"^(nlohmann/|ryml|c4/|rapidyaml)")
INCLUDE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.M)


def layer_of(path):
    """The layer of a file of ThermoFun/ (None for the public classes and other folders)."""
    parts = path.relative_to(SRC).parts
    if len(parts) < 2 or parts[0] not in LAYERS:
        return None
    if parts[0] == "IO" and parts[1] == "Legacy":
        return "IO/Legacy"
    return parts[0]


def target_of(include, source):
    """What an include names: a layer, "catalog", "legacy-enums", "public", "json-yaml", or None (outside)."""
    candidates = [source.parent / include, SRC / include]
    if include.startswith("ThermoFun/"):
        candidates.append(ROOT / include)
    for c in candidates:
        if c.is_file():
            c = c.resolve()
            if SRC not in c.parents:
                return None
            rel = c.relative_to(SRC).as_posix()
            if rel in CATALOG:
                return "catalog"
            if rel == "GlobalVariables.h":
                return "legacy-enums"
            lay = layer_of(c)
            return lay if lay else "public"
    if JSON_YAML.match(include):
        return "json-yaml"
    return None


def violations():
    found = []
    for path in sorted(SRC.rglob("*")):
        if path.suffix not in (".h", ".hpp", ".cpp") or not path.is_file():
            continue
        lay = layer_of(path)
        if lay is None:
            continue
        rel = path.relative_to(ROOT).as_posix()
        for include in INCLUDE.findall(path.read_text(encoding="utf-8", errors="replace")):
            target = target_of(include, path)
            if target is None:
                continue
            if target == "json-yaml":
                # json and yaml only in the .cpp files and detail/ headers of IO
                ok = lay in ("IO", "IO/Legacy") and (path.suffix == ".cpp" or "detail" in path.parts)
            elif target == lay or (lay == "IO/Legacy" and target == "IO"):
                ok = True
            elif target == "catalog" and lay in ("Methods", "Engine"):
                ok = True  # the specs are part of the layer Methods
            else:
                ok = target in ALLOWED[lay] or (target in LAYERS and target in ALLOWED[lay])
            if not ok:
                found.append("%s: %s" % (rel, include))
    return found


def main():
    known = set()
    if KNOWN.exists():
        for line in KNOWN.read_text().splitlines():
            line = line.split("#", 1)[0].strip()
            if line:
                known.add(line)
    found = violations()
    new = [v for v in found if v not in known]
    gone = sorted(known - set(found))
    for v in new:
        print("not allowed: " + v)
    for v in gone:
        print("fixed, remove from known_violations.txt: " + v)
    print("layer check: %d known violations, %d new, %d fixed" % (len(known), len(new), len(gone)))
    return 1 if new or gone else 0


if __name__ == "__main__":
    sys.exit(main())
