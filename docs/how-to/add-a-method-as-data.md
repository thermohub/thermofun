# Add a method as data

A closed-form model can be a Method record with an expression. No C++ change is needed.
The code in this page is run by `pytests/test_howto_docs.py`. It needs SymPy and a build with `TFUN_USE_SYMENGINE=ON`.

The engine runs the forms that give the Gibbs energy of a substance:

- `cp`: the heat capacity Cp(T). It makes a base method.
- `cp_increment`: a term ΔCp(T). It adds to another method.
- `volume`: the volume V(T, P). It adds to another method.
- `gibbs` and `gibbs_increment`: G(T, P) or ΔG(T, P) written directly.
- `shomate`: the Shomate equations of the NIST WebBook. The parameters A to G are given for each piece in T of the substance.
- `nasa7`: the NASA 7-term polynomials. The parameters a1 to a7 are given for each piece in T of the substance.

The forms for reactions, and pieces in T, are not supported yet.

```python
import pytest
import thermofun as tf

pytest.importorskip("sympy")
if not tf.symbolicAvailable():
    pytest.skip("built without SymEngine")
```

## Step 1. Write the source

The source is the model as in the paper. The names T, P, Tr, Pr, R, Gr, Hr, Sr, Cpr and Vr are known.
Declare each parameter with its SI unit. `python -m thermofun.method_tool new cp` prints a template.

```python
import json
from thermofun import method_tool as methods

record = {
    "kind": "symbolic", "tier": 1, "operates_on": "substance", "contributes_to": ["G", "H", "S", "Cp"],
    "parameter_definitions": {"a": {"unit": "J/(mol.K)"}, "b": {"unit": "J/(mol.K2)"}, "c": {"unit": "J.K/mol"}},
    "source": {"form": "cp", "expression": "a + b*T + c/T**2"},
}
```

## Step 2. Build the expression

`build` integrates the source with SymPy. It writes `expression`, `generator` and `source_hash`.
From a shell the command is `python -m thermofun.method_tool build record.yaml`.

```python
built = methods.build(record)
print(built["expression"]["G"])
assert "G" in built["expression"]
```

## Step 3. Add a test case from the paper

A test case gives T, P, the reference values, the parameters and the expected values.
`check` writes nothing. It compares the expression with the source and runs the test cases.

```python
built["tests"] = [{"T": 500.0, "P": 1e5,
                   "reference": {"Gr": -1.2e6, "Hr": -1.1731665e6, "Sr": 90.0, "Cpr": 80.0},
                   "parameters": {"a": 80.0, "b": 0.012, "c": -1.1e6},
                   "expected": {"Cp": 80.0 + 0.012 * 500.0 - 1.1e6 / 500.0**2}, "relative_tolerance": 1e-9}]
assert methods.check(built) == []
```

## Step 4. Use the method

A substance names the method and gives the parameters. The Method record goes into the `methods` section of the file.

```python
substance = {
    "formula": "CaCO3", "charge": 0, "class": "component", "aggregate_state": "crystal",
    "reference_conditions": {"T": 298.15, "P": 1e5},
    "standard_state_properties": {"G": -1.2e6, "H": -1.1731665e6, "S": 90.0, "Cp": 80.0, "V": 3e-5},
    "methods": [{"method": "MaierKelleyCp", "parameters": {"a": 80.0, "b": 0.012, "c": -1.1e6}}],
}
document = {"thermohub": "2.0.0-draft.1", "methods": {"MaierKelleyCp": built}, "substances": {"X": substance}}
engine = tf.ThermoEngine(tf.Database(json.dumps(document)))
props = engine.thermoPropertiesSubstance(500.0, 1e5, "X")
print(props.heat_capacity_cp.val)
assert abs(props.heat_capacity_cp.val - (80.0 + 0.012 * 500.0 - 1.1e6 / 500.0**2)) < 1e-9
```

The engine differentiates the expression. S is −∂G/∂T, V is ∂G/∂P and Cp is −T ∂²G/∂T².
Several methods on one record give the same result in any order.

## Step 5. Share the method

Examples are in `examples/v2/methods/`. Open a pull request with the Method record to `thermohub-data/methods/definitions/`.
A reviewer checks the source against the paper. The checks `V2-SYM-001`, `002` and `006` run when the file is read.
