# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Makes the expression of a symbolic (Tier-1) method record from its source. It needs SymPy.

    python -m thermofun.method_tool build  record.yaml [-o out.yaml]
    python -m thermofun.method_tool check  record.yaml
    python -m thermofun.method_tool new    cp

A file is one Method record, or a document with a `methods` mapping. `build` writes `expression`, `generator` and
`source_hash` next to `source`. `check` writes nothing: it exits with 1 if the expression is not the one the source gives,
if the source changed after the build, or if a test case of the record fails.

The source forms for a substance are `cp`, `cp_increment`, `volume`, `gibbs`, `gibbs_increment`, `shomate`
(the parameters A to G of the NIST WebBook) and `nasa7` (the parameters a1 to a7 of the NASA 7-term polynomials).
Each piece in T has its own parameter values in the substance.
The source forms for a reaction (`operates_on: reaction`) are `logk` (log K as a function of T and P), `drcp` (the heat
capacity of reaction as a function of T), `vant_hoff` (constant enthalpy of reaction, no expression) and `dg` (dG as written).
"""

import argparse
import hashlib
import json
import sys

FORMS = ("cp", "cp_increment", "volume", "gibbs", "gibbs_increment", "shomate", "nasa7", "sgte", "magnetic_ihj")
REACTION_FORMS = ("logk", "drcp", "vant_hoff", "dg")
FIXED = ("T", "P", "Tr", "Pr", "R", "Gr", "Hr", "Sr", "Cpr", "Vr")
FIXED_REACTION = ("T", "P", "Tr", "Pr", "R", "dGr", "dHr", "dSr", "dCpr", "dVr")


class BuildError(Exception):
    """The record cannot be built: the message says why."""


def _sympy():
    try:
        import sympy
    except ImportError as error:   # pragma: no cover
        raise BuildError("SymPy is needed: pip install sympy") from error
    return sympy


def _generator():
    try:
        import thermofun
        return "thermofun.method_tool build " + getattr(thermofun, "__version__", "")
    except ImportError:   # pragma: no cover
        return "thermofun.method_tool build"


def source_hash(source):
    """The hash of the source block, as written in `source_hash`."""
    text = json.dumps(source, sort_keys=True, separators=(",", ":"))
    return "sha256:" + hashlib.sha256(text.encode()).hexdigest()


def _of_reaction(record):
    return record.get("operates_on") == "reaction"


def _symbols(record):
    sympy = _sympy()
    fixed = FIXED_REACTION if _of_reaction(record) else FIXED
    names = list(fixed) + [str(p) for p in (record.get("parameter_definitions") or {})]
    positive = ("T", "P", "Tr", "Pr")
    return {n: sympy.Symbol(n, positive=True) if n in positive else sympy.Symbol(n, real=True) for n in names}


def _parse(text, symbols):
    sympy = _sympy()
    try:
        expr = sympy.sympify(text, locals=dict(symbols), rational=False)
    except (sympy.SympifyError, SyntaxError, TypeError) as error:
        raise BuildError(f"the expression '{text}' does not parse: {error}") from error
    unknown = sorted(str(s) for s in expr.free_symbols if str(s) not in symbols)
    if unknown:
        raise BuildError(f"the expression '{text}' uses names that are not variables or parameters: {', '.join(unknown)}")
    return expr


def _integrate(expr, variable, lower, upper):
    sympy = _sympy()
    dummy = sympy.Symbol("u_", positive=True)
    result = sympy.integrate(expr.subs(variable, dummy), (dummy, lower, upper))
    if result.has(sympy.Integral):
        raise BuildError(f"SymPy cannot integrate {expr}. Write the source in the form gibbs or gibbs_increment, "
                         "or the method needs a coded (Tier 2) implementation.")
    return result


def _shomate(symbols, record):
    """G = H - T S of the Shomate equations of the NIST WebBook (t = T / 1000, H in kJ/mol, F and G as published).

    H is the enthalpy of formation at T, so the record has the convention assigned_enthalpy."""
    sympy = _sympy()
    missing = [n for n in "ABCDEFG" if n not in (record.get("parameter_definitions") or {})]
    if missing:
        raise BuildError(f"the form shomate needs the parameters A to G; missing: {', '.join(missing)}")
    A, B, C, D, E, F, G = (symbols[n] for n in "ABCDEFG")
    T = symbols["T"]
    t = T / 1000
    H = 1000 * (A * t + B * t**2 / 2 + C * t**3 / 3 + D * t**4 / 4 - E / t + F)
    S = A * sympy.log(t) + B * t + C * t**2 / 2 + D * t**3 / 3 - E / (2 * t**2) + G
    return H - T * S


def _nasa7(symbols, record):
    """G = H - T S of the NASA 7-term polynomials: H/RT = a1 + a2 T/2 + a3 T^2/3 + a4 T^3/4 + a5 T^4/5 + a6/T,
    S/R = a1 ln T + a2 T + a3 T^2/2 + a4 T^3/3 + a5 T^4/4 + a7. H is on the assigned-enthalpy scale."""
    sympy = _sympy()
    missing = [f"a{i}" for i in range(1, 8) if f"a{i}" not in (record.get("parameter_definitions") or {})]
    if missing:
        raise BuildError(f"the form nasa7 needs the parameters a1 to a7; missing: {', '.join(missing)}")
    a1, a2, a3, a4, a5, a6, a7 = (symbols[f"a{i}"] for i in range(1, 8))
    T, R = symbols["T"], symbols["R"]
    H = R * (a1 * T + a2 * T**2 / 2 + a3 * T**3 / 3 + a4 * T**4 / 4 + a5 * T**5 / 5 + a6)
    S = R * (a1 * sympy.log(T) + a2 * T + a3 * T**2 / 2 + a4 * T**3 / 3 + a5 * T**4 / 4 + a7)
    return H - T * S


def _need(record, names, form):
    missing = [n for n in names if n not in (record.get("parameter_definitions") or {})]
    if missing:
        raise BuildError(f"the form {form} needs the parameters {', '.join(names)}; missing: {', '.join(missing)}")


def _sgte(symbols, record):
    """G - H(SER) of the SGTE unary data: a + b T + c T ln T + d T^2 + e T^3 + f/T + g T^7 + h/T^9 (one set for each piece in T)."""
    sympy = _sympy()
    _need(record, "abcdefgh", "sgte")
    a, b, c, d, e, f, g, h = (symbols[n] for n in "abcdefgh")
    T = symbols["T"]
    return a + b * T + c * T * sympy.log(T) + d * T**2 + e * T**3 + f / T + g * T**7 + h / T**9


def _magnetic_ihj(symbols, record):
    """The magnetic term of Inden, Hillert and Jarl: R T ln(beta + 1) g(tau), tau = T / Tc.
    A list of two expressions: for tau < 1 and for tau >= 1. The substance gives two pieces in T with the same parameters."""
    sympy = _sympy()
    _need(record, ("Tc", "beta", "p"), "magnetic_ihj")
    Tc, beta, p = symbols["Tc"], symbols["beta"], symbols["p"]
    T, R = symbols["T"], symbols["R"]
    tau = T / Tc
    A = sympy.Rational(518, 1125) + sympy.Rational(11692, 15975) * (1 / p - 1)
    low = 1 - (sympy.Rational(79, 140) / (p * tau) + sympy.Rational(474, 497) * (1 / p - 1)
               * (tau**3 / 6 + tau**9 / 135 + tau**15 / 600)) / A
    high = -(tau**-5 / 10 + tau**-15 / 315 + tau**-25 / 1500) / A
    prefactor = R * T * sympy.log(beta + 1)
    return [prefactor * low, prefactor * high]


def _reaction_dg(record, symbols):
    """The pair ("dG", expression of dG) for the source of a method of a reaction."""
    sympy = _sympy()
    source = record.get("source") or {}
    form = source.get("form")
    if form not in REACTION_FORMS:
        raise BuildError(f"the source form '{form}' is not for a reaction. Use one of: {', '.join(REACTION_FORMS)}")
    T, Tr = symbols["T"], symbols["Tr"]
    if form == "vant_hoff":
        return "dG", symbols["dHr"] - T * (symbols["dHr"] - symbols["dGr"]) / Tr
    if "expression" not in source:
        raise BuildError("the source needs an expression")
    expr = _parse(source["expression"], symbols)
    if form == "dg":
        return "dG", expr
    if form == "logk":
        return "dG", -symbols["R"] * T * sympy.log(10) * expr
    dH = _integrate(expr, T, Tr, T)
    dS = _integrate(expr / T, T, Tr, T)
    return "dG", symbols["dGr"] - symbols["dSr"] * (T - Tr) + dH - T * dS


def _gibbs(record, symbols):
    """The pair (key, expression of G or of the increment of G) for the source of the record."""
    sympy = _sympy()
    if _of_reaction(record):
        return _reaction_dg(record, symbols)
    source = record.get("source") or {}
    form = source.get("form")
    if "segments" in source:
        raise BuildError("pieces in T (source.segments) are not supported yet")
    if form not in FORMS:
        raise BuildError(f"the source form '{form}' is not supported. Use one of: {', '.join(FORMS)}")
    if form == "shomate":
        return "G", _shomate(symbols, record)
    if form == "nasa7":
        return "G", _nasa7(symbols, record)
    if form == "sgte":
        return "G", _sgte(symbols, record)
    if form == "magnetic_ihj":
        return "G_increment", _magnetic_ihj(symbols, record)
    if "expression" not in source:
        raise BuildError("the source needs an expression")
    expr = _parse(source["expression"], symbols)
    T, P, Tr, Pr = (symbols[n] for n in ("T", "P", "Tr", "Pr"))
    if form == "cp":
        dH = _integrate(expr, T, Tr, T)
        dS = _integrate(expr / T, T, Tr, T)
        return "G", symbols["Gr"] - symbols["Sr"] * (T - Tr) + dH - T * dS
    if form == "cp_increment":
        dH = _integrate(expr, T, Tr, T)
        dS = _integrate(expr / T, T, Tr, T)
        return "G_increment", dH - T * dS
    if form == "volume":
        return "G_increment", _integrate(expr, P, Pr, P)
    return ("G" if form == "gibbs" else "G_increment"), expr


def _numbers(record, symbols, case):
    values = {"dGr": 0.0, "dHr": 0.0, "dSr": 0.0, "dCpr": 0.0, "dVr": 0.0, "T": 298.15, "P": 1e5, "Tr": 298.15, "Pr": 1e5, "R": 8.31451, "Gr": 0.0, "Hr": 0.0, "Sr": 0.0, "Cpr": 0.0, "Vr": 0.0}
    values.update(case.get("reference") or {})
    values.update({"T": case.get("T", values["T"]), "P": case.get("P", values["P"])})
    values.update(case.get("parameters") or {})
    return {symbols[n]: v for n, v in values.items() if n in symbols}


def properties(expression, record, case):
    """G, H, S, Cp and V (SI) of an expression of G for the values of a test case."""
    sympy = _sympy()
    symbols = _symbols(record)
    g = _parse(expression, symbols)
    T, P = symbols["T"], symbols["P"]
    s = -sympy.diff(g, T)
    out = {"G": g, "S": s, "V": sympy.diff(g, P), "Cp": -T * sympy.diff(g, T, 2), "H": g + T * s}
    if _of_reaction(record):
        out = {"d" + name: value for name, value in out.items()}
        out["logK"] = -g / (symbols["R"] * T * sympy.log(10))
    values = _numbers(record, symbols, case)
    return {k: float(v.subs(values)) for k, v in out.items()}


def _simplified(expr, symbols, record):
    """simplify only if the value does not change, also when it is evaluated with ordinary floating-point numbers"""
    sympy = _sympy()
    try:
        # simplify joins logarithms: log(T**a/Tr**a) overflows for a large a, as a*log(T) does not
        simple = sympy.expand_log(sympy.simplify(expr), force=True)
    except Exception:   # pragma: no cover
        return expr
    names = list(symbols)
    f_old = sympy.lambdify([symbols[n] for n in names], expr, "math")
    f_new = sympy.lambdify([symbols[n] for n in names], simple, "math")
    for T, value in ((350.0, 1.5), (900.0, 1.5), (2500.0, 1.5), (900.0, 300.0)):
        point = {"T": T, "P": 2e7, "Tr": 298.15, "Pr": 1e5, "R": 8.31451, "Gr": -1e6, "Hr": -1.1e6, "Sr": 80.0, "Cpr": 90.0, "Vr": 3e-5}
        args = [point.get(n, point.get(n[1:], value)) for n in names]  # dGr as Gr
        try:
            a, b = f_old(*args), f_new(*args)
        except (OverflowError, ValueError, ZeroDivisionError):
            return expr
        if not (abs(a - b) <= 1e-12 * max(1.0, abs(a))):
            return expr
    return simple


def build(record):
    """A copy of the Method record with `expression`, `generator` and `source_hash` made from `source`."""
    sympy = _sympy()
    if record.get("kind") != "symbolic":
        raise BuildError("the record is not a symbolic method (kind: symbolic)")
    symbols = _symbols(record)
    key, g = _gibbs(record, symbols)
    out = dict(record)
    if isinstance(g, list):   # one expression for each piece in T
        out["expression"] = [{key: sympy.sstr(_simplified(x, symbols, record))} for x in g]
    else:
        out["expression"] = {key: sympy.sstr(_simplified(g, symbols, record))}
    out["generator"] = _generator()
    out["source_hash"] = source_hash(record["source"])
    return out


def _pieces(expression):
    """The expressions of a record as a list of mappings: one for a method without pieces"""
    return expression if isinstance(expression, list) else [expression or {}]


def check(record):
    """The problems of a record: a list of messages, empty if it is in order."""
    problems = []
    try:
        built = build(record)
    except BuildError as error:
        return [str(error)]
    if record.get("source_hash") and record["source_hash"] != built["source_hash"]:
        problems.append("the source changed after the expression was made (V2-SYM-003): run build")
    written, made = _pieces(record.get("expression")), _pieces(built["expression"])
    key = next(iter(made[0]))
    if len(written) != len(made) or any(key not in w for w in written):
        problems.append(f"the record has not the expression for {key} of each piece: run build")
        return problems
    # the written expression and the one of the source must give the same numbers
    point = {"T": 450.0, "P": 3e7, "reference": {"Gr": -1e6, "Hr": -1.1e6, "Sr": 80.0, "Cpr": 90.0, "Vr": 3e-5, "dGr": -1e4, "dHr": -2e4, "dSr": 30.0, "dCpr": 90.0, "dVr": 3e-5},
             "parameters": {n: 1.5 for n in (record.get("parameter_definitions") or {})}}
    for i, (w, m) in enumerate(zip(written, made)):
        a, b = properties(w[key], record, point), properties(m[key], record, point)
        for name in a:
            if abs(a[name] - b[name]) > 1e-9 * max(1.0, abs(b[name])):
                problems.append(f"the expression of piece {i} and the source differ in {name}")
    # the test cases, with the generated form; a case of a method with pieces names its piece
    for i, case in enumerate(record.get("tests") or []):
        got = properties(written[case.get("piece", 0)][key], record, case)
        tolerance = case.get("relative_tolerance", 1e-9)
        for name, want in (case.get("expected") or {}).items():
            if name in got and abs(got[name] - want) > tolerance * max(abs(want), 1e-30):
                problems.append(f"test case {i}: {name} is {got[name]:.10g}, expected {want:.10g} (V2-SYM-004)")
    return problems


_TEMPLATES = {
    "cp": ("a + b*T + c/T**2", {"a": "J/(mol.K)", "b": "J/(mol.K2)", "c": "J.K/mol"}),
    "cp_increment": ("a + b*T", {"a": "J/(mol.K)", "b": "J/(mol.K2)"}),
    "volume": ("v0*(1 - k*(P - Pr))", {"v0": "m3/mol", "k": "1/Pa"}),
    "gibbs": ("Gr - Sr*(T - Tr)", {}),
    "gibbs_increment": ("dv*(P - Pr)", {"dv": "m3/mol"}),
    "shomate": ("", {n: "1" for n in "ABCDEFG"}),
    "nasa7": ("", {f"a{i}": "1" for i in range(1, 8)}),
    "sgte": ("", {n: "1" for n in "abcdefgh"}),
    "magnetic_ihj": ("", {"Tc": "K", "beta": "1", "p": "1"}),
    "logk": ("A0 + A1*T + A2/T + A3*log(T)/log(10) + A4/T**2", {"A0": "1", "A1": "1/K", "A2": "K", "A3": "1", "A4": "K2"}),
    "drcp": ("a + b*T", {"a": "J/(mol.K)", "b": "J/(mol.K2)"}),
    "vant_hoff": ("", {}),
    "dg": ("dGr - dSr*(T - Tr)", {}),
}


def new(form):
    """A template Method record for a source form."""
    if form not in _TEMPLATES:
        raise BuildError(f"the source form '{form}' is not supported. Use one of: {', '.join(FORMS + REACTION_FORMS)}")
    expression, parameters = _TEMPLATES[form]
    if form in REACTION_FORMS:
        return {"kind": "symbolic", "tier": 1, "operates_on": "reaction", "contributes_to": ["logK", "dG", "dH", "dS", "dCp", "dV"],
                "parameter_definitions": {k: {"unit": v} for k, v in parameters.items()},
                "source": {"form": form, "expression": expression} if expression else {"form": form}}
    source = {"form": form} if form in ("shomate", "nasa7", "sgte", "magnetic_ihj") else {"form": form, "expression": expression}
    return {"kind": "symbolic", "tier": 1, "operates_on": "substance",
            "contributes_to": ["G", "H", "S", "Cp"] + (["V"] if form in ("volume", "gibbs_increment") else []),
            "parameter_definitions": {k: {"unit": v} for k, v in parameters.items()},
            "source": source}


def _load(path):
    with open(path) as f:
        text = f.read()
    if path.endswith((".yaml", ".yml")):
        import yaml
        return yaml.safe_load(text), "yaml"
    return json.loads(text), "json"


def _dump(data, kind):
    if kind == "yaml":
        import yaml
        return yaml.safe_dump(data, sort_keys=False)
    return json.dumps(data, indent=2) + "\n"


def _records(document):
    """(key, record) of the symbolic Method records of a file: the record itself, or the `methods` of a document."""
    if "methods" in document and isinstance(document["methods"], dict):
        return [(k, v) for k, v in document["methods"].items() if isinstance(v, dict) and v.get("kind") == "symbolic"]
    return [(None, document)]


def main(argv=None):
    parser = argparse.ArgumentParser(prog="python -m thermofun.method_tool", description="Authoring of symbolic methods")
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ("build", "check"):
        p = sub.add_parser(name)
        p.add_argument("file")
        if name == "build":
            p.add_argument("-o", "--output")
    p = sub.add_parser("new")
    p.add_argument("form", choices=FORMS + REACTION_FORMS)
    args = parser.parse_args(argv)

    try:
        if args.command == "new":
            sys.stdout.write(_dump(new(args.form), "yaml"))
            return 0
        document, kind = _load(args.file)
        records = _records(document)
        if args.command == "check":
            bad = 0
            for key, record in records:
                for problem in check(record):
                    print(f"{key or args.file}: {problem}")
                    bad += 1
            return 1 if bad else 0
        for key, record in records:
            built = build(record)
            if key is None:
                document = built
            else:
                document["methods"][key] = built
        sys.stdout.write("") if args.output else None
        with open(args.output or args.file, "w") as f:
            f.write(_dump(document, kind))
        return 0
    except BuildError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
