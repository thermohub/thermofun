<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Validate data

**You need:** ThermoFun, and a database file.

The tool `thermofun.validate` reads a file, and prints each problem with its code, the record, the place in the file and
how to correct it. The last line is a summary. The exit code is 1 if there is an error, so a script can stop on it.
Each code is explained in [Diagnostics](../reference/diagnostics.md).

## 1. A file in order

```console
$ python -m thermofun.validate minimal.yaml
minimal.yaml: no problems
```

## 2. A file with problems

A load refuses only what makes the file unusable. The other problems are found when the whole database is checked. Here
two minerals of aq17 miss parameters of their volume method:

```console
$ python -m thermofun.validate aq17-thermofun.json
error V2-MTH-004 Boehmite /substances/Boehmite/methods/1: The method MurnaghanHP98 needs the parameter alpha0 (thermal expansion at the reference state, in 1/K). The method works on G, H, S, Cp, V and cannot calculate without it. The page of the method (reference/methods/MurnaghanHP98) lists its parameters. Add the parameter, or give a `parameter_set` that has it.
error V2-MTH-004 Boehmite /substances/Boehmite/methods/1: The method MurnaghanHP98 needs the parameter kappa0 (bulk modulus at 298 K, in kbar). The method works on G, H, S, Cp, V and cannot calculate without it. The page of the method (reference/methods/MurnaghanHP98) lists its parameters. Add the parameter, or give a `parameter_set` that has it.
error V2-MTH-004 Gibbsite /substances/Gibbsite/methods/1: The method MurnaghanHP98 needs the parameter alpha0 (thermal expansion at the reference state, in 1/K). The method works on G, H, S, Cp, V and cannot calculate without it. The page of the method (reference/methods/MurnaghanHP98) lists its parameters. Add the parameter, or give a `parameter_set` that has it.
error V2-MTH-004 Gibbsite /substances/Gibbsite/methods/1: The method MurnaghanHP98 needs the parameter kappa0 (bulk modulus at 298 K, in kbar). The method works on G, H, S, Cp, V and cannot calculate without it. The page of the method (reference/methods/MurnaghanHP98) lists its parameters. Add the parameter, or give a `parameter_set` that has it.
aq17-thermofun.json: 4 errors, 0 warnings
```

Each line has the severity, the code, the record, and the path of the value in the record (`methods/1` is the second
method).

## 3. A file that cannot be read

This file names a method that does not exist:

<!-- file: bad.yaml -->
```yaml
thermohub: 2.0.0-draft.1
substances:
  X:
    formula: Ca
    class: component
    standard_state_properties: {G: -1000.0}
    methods:
      - method: NoSuchMethod
```

The line and the column of the problem come first:

```console
$ python -m thermofun.validate bad.yaml
bad.yaml:8:9: error V2-MTH-001 X /substances/X/methods/0/method: Unknown method NoSuchMethod Use a method id from the method reference; the message names the nearest one.
bad.yaml: the file cannot be read
```

## 4. The same in Python

`validate()` gives the problems of a database that could be read, and `loadDiagnostics()` the warnings of the reading.
Each problem has `severity`, `code`, `record`, `path` and `message`.

```python
import thermofun as tf

problems = tf.Database("aq17-thermofun.json").validate()
for problem in problems.all()[:2]:
    print(problem.severity, problem.code, problem.record, problem.path)
print(problems.ok())
```

Output:

```text
Severity.Error V2-MTH-004 Boehmite /substances/Boehmite/methods/1
Severity.Error V2-MTH-004 Boehmite /substances/Boehmite/methods/1
False
```

A file that cannot be read raises an error with the same text:

```python
try:
    tf.Database("bad.yaml")
except RuntimeError as error:
    print("V2-MTH-001" in str(error))
```

Output:

```text
True
```
