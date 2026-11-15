# Add a method as data

A closed-form model can be a Method record with an expression. No C++ change is needed.

```{admonition} For people who add models
:class: note
You do not need this page to use the methods that ThermoFun has: they are in the [list of methods](../reference/methods/index.md). This page is for adding a new equation from a paper. It needs SymPy and a build with `TFUN_USE_SYMENGINE=ON`.
```

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
if not tf.methods.symbolicAvailable():
    pytest.skip("built without SymEngine")
```

## Step 1. Write the source

The source is the model as in the paper: here the heat capacity Cp(T) = a + b T + c/T² of Maier and Kelley.
`"form": "cp"` says that the expression is Cp. It is all you write: the record does not say how G, H and S follow from Cp,
because that is the same for every Cp, and Step 2 does it. The names T, P, Tr, Pr, R, Gr, Hr, Sr, Cpr and Vr are known
(Gr, Hr, Sr, Cpr and Vr are the reference values of the record that uses the method). Declare each parameter with its SI unit.
`python -m thermofun.method_tool new cp` prints a template.

```python
from thermofun import method_tool as methods

record = {
    "kind": "symbolic", "tier": 1, "operates_on": "substance", "contributes_to": ["G", "H", "S", "Cp"],
    "parameter_definitions": {"a": {"unit": "J/(mol.K)"}, "b": {"unit": "J/(mol.K2)"}, "c": {"unit": "J.K/mol"}},
    "source": {"form": "cp", "expression": "a + b*T + c/T**2"},
}
```

## Step 2. Build the expression

`build` turns the source into G, with SymPy, once. For the form `cp` it integrates from the reference temperature Tr to T:

- ΔH = ∫ Cp dT and ΔS = ∫ Cp/T dT;
- G = Gr - Sr (T - Tr) + ΔH - T ΔS.

It writes this G into the record, under `expression`, with `generator` and `source_hash`. From a shell the command is
`python -m thermofun.method_tool build record.yaml`.

```python
built = methods.build(record)
print(built["expression"]["G"])
```

The record now holds G written out (for example `examples/v2/methods/maier-kelley-cp.yaml`, the key `expression` at the end).
The engine reads only this G, never the form: S = -∂G/∂T, H = G + T S, Cp = -T ∂²G/∂T² and V = ∂G/∂P, with exact
derivatives. So the Cp that the engine gives back is the Cp of the source, and S starts at Sr. The other forms work the same
way: `cp_increment` and `volume` are integrated to a ΔG, and `gibbs` and `gibbs_increment` are G or ΔG as written.
`source_hash` ties the G to the source: if one changes without the other, the checks V2-SYM-001 and 002 report it when the file
is read.

<!-- check
```python
assert "G" in built["expression"]
```
-->

## Step 3. Add a test case from the paper

A test case gives T, P, the reference values, the parameters and the expected values.
`check` writes nothing. It compares the expression with the source and runs the test cases.

```python
built["tests"] = [{"T": 500.0, "P": 1e5,
                   "reference": {"Gr": -1.2e6, "Hr": -1.1731665e6, "Sr": 90.0, "Cpr": 80.0},
                   "parameters": {"a": 80.0, "b": 0.012, "c": -1.1e6},
                   "expected": {"Cp": 80.0 + 0.012 * 500.0 - 1.1e6 / 500.0**2}, "relative_tolerance": 1e-9}]
print(methods.check(built))  # prints []
```

<!-- check
```python
assert methods.check(built) == []
```
-->

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
engine = tf.ThermoEngine(tf.Database(document))
props = engine.thermoPropertiesSubstance(500.0, 1e5, "X")
print(props.heat_capacity_cp.val)
```

<!-- check
```python
assert abs(props.heat_capacity_cp.val - (80.0 + 0.012 * 500.0 - 1.1e6 / 500.0**2)) < 1e-9
```
-->

The engine differentiates the expression. S is −∂G/∂T, V is ∂G/∂P and Cp is −T ∂²G/∂T².
Several methods on one record give the same result in any order.

(the-same-method-in-c-and-as-data)=
## The same method in C++ and as data

A method can be written in C++ ([Add a method in C++](add-a-cpp-method.md)) or as data, as on this page. Python code cannot be a method (a calculation calls a method many times, for each T, P and derivative); in Python you write the method as data. Here is one method both ways: it adds a (T - Tr) to G.

In C++, you write G and each property that follows from it:

```cpp
template <typename Scalar>
auto myMethod(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in, const MethodParameters<>& p) -> void
{
    const Scalar a = p.value(0, in);
    state.G += a * (in.T - in.reference.T);   // dG = a (T - Tr)
    state.S -= a;                             // dS = -d(dG)/dT
    state.H -= a * in.reference.T;            // dH = dG + T dS
}
```

As data, you write only G. The engine takes S, H, Cp and V from it, so they cannot disagree:

```python
shift = methods.build({
    "kind": "symbolic", "tier": 1, "operates_on": "substance", "contributes_to": ["G", "H", "S"],
    "parameter_definitions": {"a": {"unit": "J/(mol.K)"}},
    "source": {"form": "gibbs_increment", "expression": "a*(T - Tr)"},
    "tests": [{"T": 400.0, "P": 1e5, "parameters": {"a": 2.0},
               "expected": {"G": 2.0 * (400.0 - 298.15), "S": -2.0, "H": -2.0 * 298.15}, "relative_tolerance": 1e-12}],
})
print(shift["expression"])   # prints {'G_increment': 'a*(T - Tr)'}
print(methods.check(shift))  # prints []: the test case agrees
```

A record uses it after its base method, like any increment:

```python
mineral = {"formula": "CaCO3", "class": "component", "aggregate_state": "crystal",
           "standard_state_properties": {"G": -1.2e6, "H": -1.1731665e6, "S": 90.0, "Cp": 80.0, "V": 3e-5}}
with_shift = tf.Database({"methods": {"EntropyShift": shift},
                          "substances": {"X": {**mineral, "methods": [{"method": "ConstantCp"}, {"method": "EntropyShift", "parameters": {"a": 2.0}}]}}})
without = tf.Database({"substances": {"X": {**mineral, "methods": [{"method": "ConstantCp"}]}}})

x = tf.ThermoEngine(with_shift).thermoPropertiesSubstance(400.0, 1e5, "X")
y = tf.ThermoEngine(without).thermoPropertiesSubstance(400.0, 1e5, "X")
print(x.gibbs_energy.val - y.gibbs_energy.val)   # a (T - Tr) = 203.7 J/mol
print(x.entropy.val - y.entropy.val)             # -a = -2.0 J/(mol K)
```

<!-- check
```python
assert methods.check(shift) == []
assert abs((x.gibbs_energy.val - y.gibbs_energy.val) - 2.0 * (400.0 - 298.15)) < 1e-8
assert abs((x.entropy.val - y.entropy.val) + 2.0) < 1e-10
assert abs((x.enthalpy.val - y.enthalpy.val) + 2.0 * 298.15) < 1e-8
```
-->

## Step 5. Share the method

Examples are in `examples/v2/methods/`. Open a pull request with the Method record to `thermohub-data/methods/definitions/`.
A reviewer checks the source against the paper. The checks `V2-SYM-001`, `002` and `006` run when the file is read.

## Expert: a method of a reaction

This section is for model authors who need a method for a reaction, for example a new equation of log K. You can skip it.

A method of a reaction has `operates_on: reaction`. Its expression gives dG in J/mol. All other properties come from it: dS = -d(dG)/dT, dH = dG + T dS, dCp = -T d2(dG)/dT2, dV = d(dG)/dP, and log K = -dG/(R T ln 10).

The names T, P, Tr, Pr and R are known, and the reference values of the reaction are dGr, dHr, dSr, dCpr and dVr. In an expression, `log` is the natural logarithm.

Write what the paper gives. The tool makes the expression of dG:

| Form | The paper gives | The source | The record gives | The tool writes for dG |
|---|---|---|---|---|
| `logk` | log K = a + b/T | `a + b/T` | | `-R*T*log(10)*(a + b/T)` |
| `logk` | log K = A0 + A1 T + A2/T + A3 log10 T + A4/T2 | `A0 + A1*T + A2/T + A3*log(T)/log(10) + A4/T**2` | | the same, times `-R*T*log(10)` |
| `vant_hoff` | log K and dH at 25 degC, with constant dH | no expression | `logK`, `dH` | `dHr - T*(dHr - dGr)/Tr` |
| `drcp` | constant dCp | `dc` | `logK`, and `dS` or `dH` | `dGr - dSr*(T - Tr) + dc*(T - Tr - T*log(T/Tr))` |
| `drcp` | dCp = a + b T | `a + b*T` | `logK`, and `dS` or `dH` | `dGr - dSr*(T - Tr) + a*(T - Tr - T*log(T/Tr)) - b*(T - Tr)**2/2` |
| `dg` | dG(T, P) | the expression | what it uses | the expression |

An example: log K = a + b/T. The method is made as for a substance, with one test case from the paper (at 400 K, log K is 0.75):

```python
from thermofun import method_tool as tool

logk_method = tool.build({
    "kind": "symbolic", "tier": 1, "operates_on": "reaction",
    "parameter_definitions": {"a": {"unit": "1"}, "b": {"unit": "K"}},
    "source": {"form": "logk", "expression": "a + b/T"},
    "tests": [{"T": 400.0, "P": 1e5, "parameters": {"a": -3.0, "b": 1500.0}, "expected": {"logK": 0.75}}],
})
print(tool.check(logk_method))  # prints []
```

The reaction names the method and gives a and b. A small database holds the method, the reaction and its two substances:

```python
substances = {
    "Y": {"formula": "Ca", "class": "component", "standard_state_properties": {"G": -1000.0, "H": -900.0, "S": 10.0, "Cp": 10.0, "V": 1e-5}},
    "Z": {"formula": "Ca", "class": "component", "defining_reaction": "R"},
}
reactions = {"R": {"equation": "Y = Z", "methods": [{"method": "MyLogK", "parameters": {"a": -3.0, "b": 1500.0}}]}}

database = tf.Database({"thermohub": "2.0.0-draft.1", "methods": {"MyLogK": logk_method}, "substances": substances, "reactions": reactions})
engine = tf.ThermoEngine(database)
print(engine.thermoPropertiesReaction(400.0, 1e5, "R").log_equilibrium_constant.val)  # prints 0.75 (to 1e-10)
```

<!-- check
```python
assert tool.check(logk_method) == []
assert abs(engine.thermoPropertiesReaction(400.0, 1e5, "R").log_equilibrium_constant.val - 0.75) < 1e-10
```
-->

`python -m thermofun.method_tool new logk` prints a template for a form. Examples are in `examples/v2/methods/`.

**The reference values.** A value that the record does not give comes from those it gives: dG from log K, and dS or dH from dG = dH - Tr dS. If the expression uses a value that the record cannot give, the calculation stops with an error that names it.

## What the checks find

`validate` and `python -m thermofun.method_tool check` run the checks of a method.

| Code | What it finds |
|---|---|
| `V2-SYM-001`, `002`, `006` | An unknown name, an expression that cannot be read, or one that is too large. |
| `V2-SYM-003` | The source changed after the expression was made. |
| `V2-SYM-004` | A test case of the method fails. |
| `V2-SYM-005` | The expression does not give an energy per mole. |

`V2-SYM-005` calculates the expression with the unit of mass doubled, so that each value with kg in its unit changes. G must double. It finds a wrong power of an energy, for example `a*b*(T - Tr)` with `a` and `b` both in J/(mol K).

It does not find:

- A wrong unit of temperature, length or time. `a*T` and `a*T**2` both pass. Published forms use `ln T` and `T/1000`, so a check on kelvin would refuse correct methods.
- A wrong coefficient with a correct unit.
- A parameter whose unit ThermoFun cannot read: the check does not run for it.

**So give each method a test case from the paper.** A wrong power of T gives a wrong number, and `V2-SYM-004` finds it.
