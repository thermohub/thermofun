<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Validate data

**You need:** ThermoFun, and a database file.

1. Run the check from a shell. It prints each problem with its code, and how to correct it.

   ```
   python -m thermofun.validate my-database.yaml
   ```

2. Correct each error. [Diagnostics](../reference/diagnostics.md) lists all codes.

3. Or run it in Python:

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

```python
import thermofun as tf

database = tf.Database()
try:
    database = tf.Database("bad.yaml")
    problems = database.validate()
except RuntimeError as error:
    problems = str(error)
assert "V2-MTH-001" in str(problems) or any(p.code == "V2-MTH-001" for p in problems)
```

**Check:** a file in order has no errors.

```python
errors = [d for d in tf.Database("examples/v2/minimal.yaml").validate() if d.severity == tf.Severity.Error]
assert errors == []
```
