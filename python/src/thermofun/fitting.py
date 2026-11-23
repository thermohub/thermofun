# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Fit parameters of records to measured properties, with the exact Jacobian of the engine.

    from thermofun import fitting
    problem = fitting.Problem(engine,
                              parameters=[("Calcite", "", "G"), ("Calcite", "CpPolynomial", "a0")],
                              observations=[fitting.Observation("Calcite", "Cp", 500.0, 1e5, 95.2, 0.5)])
    result = problem.solve()
    result.values, result.covariance
    result.database().save("calcite-fitted.yaml", tf.DataFormat.V2_YAML)

Equality constraints hold exactly at the solution:

    fitting.PropertyConstraint("Na+", "Cp", 298.15, 1e5, value)        # a property at T, P has a given value
    fitting.Relation({("Na+", "HKF88", "a4"): 1.0, ("Na+", "HKF88", "a2"): 4.134}, -27790 * 4.184)

A parameter that enters the model strongly nonlinearly (omega of HKF) can be searched in one dimension; the other
parameters are then solved for each of its values: Problem(..., search=("Na+", "HKF88", "omega")).

Without constraints and search the solver is scipy.optimize.least_squares (optional dependency: thermofun[fitting]);
with them it is a Gauss-Newton solver with the constraints in a KKT system (NumPy only). All numbers are in SI: V in
m3/mol, the parameters in the units of their handles.
"""

from dataclasses import dataclass, field
from types import SimpleNamespace

from thermofun.PyThermoFun import *  # noqa: F401,F403

SUBSTANCE_FIELDS = {"G": "gibbs_energy", "H": "enthalpy", "S": "entropy", "Cp": "heat_capacity_cp", "V": "volume",
                    "U": "internal_energy", "A": "helmholtz_energy"}
REACTION_FIELDS = {"logK": "log_equilibrium_constant", "lnK": "ln_equilibrium_constant", "dG": "reaction_gibbs_energy", "dH": "reaction_enthalpy",
                   "dS": "reaction_entropy", "dCp": "reaction_heat_capacity_cp", "dV": "reaction_volume"}


@dataclass
class Observation:
    """One measured value. property: G, H, S, Cp, V, U, A of a substance; logK, lnK, dG, dH, dS, dCp, dV of a reaction. uncertainty: its standard uncertainty, the weight in the fit."""
    symbol: str
    property: str
    T: float
    P: float
    value: float
    uncertainty: float = 1.0


SI_UNITS = {"G": "J/mol", "H": "J/mol", "U": "J/mol", "A": "J/mol", "S": "J/(mol*K)", "Cp": "J/(mol*K)", "V": "m3/mol",
            "dG": "J/mol", "dH": "J/mol", "dS": "J/(mol*K)", "dCp": "J/(mol*K)", "dV": "m3/mol"}


def _points(name, x, n):
    if isinstance(x, (int, float)):
        return [float(x)] * n
    x = [float(v) for v in x]
    if len(x) != n:
        raise ValueError(f"{name} has {len(x)} values; the observations need {n} (or one number for all).")
    return x


def observations(symbol, property, T, P, values, uncertainties=1.0, unit=None, T_unit="K"):
    """The observations of one property of one record at several points. T, P, uncertainties: a list as long as values,
    or one number for all. unit: the unit of values and uncertainties, for example "kJ/mol" (default SI). T_unit: "K" or
    "C". P in Pa; 0 is the saturation pressure of water."""
    _check_property(property, f"the observations of {symbol}")
    values = [float(v) for v in values]
    n = len(values)
    T, P, uncertainties = _points("T", T, n), _points("P", P, n), _points("uncertainties", uncertainties, n)
    if T_unit not in ("K", "C"):
        raise ValueError(f"T_unit is {T_unit}; use K or C.")
    shift = 273.15 if T_unit == "C" else 0.0
    factor = 1.0
    if unit:
        if property not in SI_UNITS:
            raise ValueError(f"{property} has no unit to convert to.")
        from thermofun import convert_units
        factor = convert_units(1.0, _unit(unit), _unit(SI_UNITS[property]))
    return [Observation(symbol, property, t + shift, p, v * factor, u * factor) for t, p, v, u in zip(T, P, values, uncertainties)]


def calculated(engine, symbol, property, T, P, uncertainty=1.0):
    """Observations made with an engine (SI): data with known parameters, to see a fit find them. T, P: lists of the
    same length, or one number for all; uncertainty: one number for all, in SI."""
    n = max(1 if isinstance(T, (int, float)) else len(T), 1 if isinstance(P, (int, float)) else len(P))
    T, P = _points("T", T, n), _points("P", P, n)
    problem = SimpleNamespace(engine=engine)
    values = [Problem._value(problem, symbol, property, t, p) for t, p in zip(T, P)]
    return observations(symbol, property, T, P, values, uncertainty)


@dataclass
class PropertyConstraint:
    """The property of a record at T, P equals value exactly. property: as in Observation.
    terms: {(symbol, method, parameter): coefficient}, added to the calculated property as coefficient * parameter
    (for example the difference of other Born functions at 25 C from those of the solvent, times omega)."""
    symbol: str
    property: str
    T: float
    P: float
    value: float
    terms: dict = field(default_factory=dict)


@dataclass
class Relation:
    """A linear relation among parameters, exact: sum of coefficient * parameter = value.
    terms: {(symbol, method, parameter): coefficient}; the values are in the units of the handles."""
    terms: dict
    value: float


class Result:
    """What a fit gives. values: by the name of the parameter. covariance: a matrix in the order of names. chi2: the
    sum of the squared weighted residuals. weighted_rms: sqrt(chi2 / sum of 1/uncertainty^2), a typical deviation in
    the unit of the observations. constraint_residuals: calculated minus required value of each constraint."""

    def __init__(self, problem, x, covariance, chi2, success, message, solver=None):
        self._problem = problem
        self._x = [float(v) for v in x]
        self.names = problem.names
        self.values = dict(zip(problem.names, self._x))
        self.covariance = covariance
        self.uncertainties = dict(zip(problem.names, [max(float(covariance[i][i]), 0.0) ** 0.5 for i in range(len(problem.names))]))
        self.chi2 = chi2
        self.degrees_of_freedom = len(problem.observations) + len(problem.constraints) - len(problem.names)
        weights = sum(1.0 / o.uncertainty ** 2 for o in problem.observations)
        self.weighted_rms = (chi2 / weights) ** 0.5
        self.constraint_residuals = problem.constraint_residuals(self._x)
        self.success = bool(success)
        self.message = str(message)
        self.solver = solver
        self.scipy = solver if solver is not None and hasattr(solver, "jac") else None

    def database(self):
        """A database whose records have the fitted values and their covariance. Saved in v2 format, it keeps them."""
        problem = self._problem
        n = len(problem.names)
        for handle, value in zip(problem.handles, self._x):
            handle.setValue(float(value))
        flat = [float(self.covariance[i][j]) for i in range(n) for j in range(n)]
        self.covariance_dropped = problem.engine.setParameterCovariance(problem.keys, flat)
        return problem.engine.commitParameters()


def _check_property(name, what):
    if name not in SUBSTANCE_FIELDS and name not in REACTION_FIELDS:
        raise ValueError(f"Unknown property {name} of {what}. Use one of {sorted(SUBSTANCE_FIELDS)} or {sorted(REACTION_FIELDS)}.")


class Problem:
    """A least squares problem: the parameters as (symbol, method, parameter), method "" for a reference value.
    constraints: PropertyConstraint and Relation objects, held exactly. search: one of the parameters, searched in one
    dimension while the others are solved for each of its values."""

    def __init__(self, engine, parameters, observations, constraints=(), search=None):
        if not parameters:
            raise ValueError("There are no parameters to fit.")
        constraints = list(constraints)
        if len(observations) + len(constraints) < len(parameters):
            raise ValueError(f"{len(observations)} observations and {len(constraints)} constraints cannot fix {len(parameters)} parameters.")
        for o in observations:
            _check_property(o.property, f"an observation of {o.symbol}")
            if not o.uncertainty > 0.0:
                raise ValueError(f"The uncertainty of an observation of {o.symbol} must be above 0.")
        for c in constraints:
            if isinstance(c, PropertyConstraint):
                _check_property(c.property, f"a constraint on {c.symbol}")
            elif not isinstance(c, Relation):
                raise ValueError(f"A constraint must be a PropertyConstraint or a Relation, not {type(c).__name__}.")
        self.engine = engine
        self.keys = [tuple(p) for p in parameters]
        self.handles = [engine.parameter(*p) for p in self.keys]
        self.names = [h.name() for h in self.handles]
        self.observations = list(observations)
        self.constraints = constraints
        self._other = {}  # handles of parameters named in constraints but not fitted: constant
        for c in constraints:
            for key in c.terms:
                key = tuple(key)
                if key not in self.keys and key not in self._other:
                    self._other[key] = engine.parameter(*key)
        self.search = None
        if search is not None:
            if tuple(search) not in self.keys:
                raise ValueError(f"The searched parameter {search} must be one of the fitted parameters.")
            self.search = self.keys.index(tuple(search))

    # ---- calculated values and derivatives
    def _value(self, symbol, prop, T, P):
        if prop in SUBSTANCE_FIELDS:
            return getattr(self.engine.thermoPropertiesSubstance(T, P, symbol), SUBSTANCE_FIELDS[prop]).val
        return getattr(self.engine.thermoPropertiesReaction(T, P, symbol), REACTION_FIELDS[prop]).val

    def _derivatives(self, symbol, prop, T, P):
        d = self.engine.parameterSensitivities(T, P, symbol, self.keys)[prop]
        return [d[name] for name in self.names]

    def _calculate(self, o):
        return self._value(o.symbol, o.property, o.T, o.P)

    def _set(self, x):
        for handle, value in zip(self.handles, x):
            if handle.value() != value:
                handle.setValue(float(value))

    def _parameter_value(self, key, x):
        key = tuple(key)
        return x[self.keys.index(key)] if key in self.keys else self._other[key].value()

    def residuals(self, x):
        """(calculated - measured) / uncertainty for each observation."""
        self._set(x)
        return [(self._calculate(o) - o.value) / o.uncertainty for o in self.observations]

    def jacobian(self, x):
        """The exact derivatives of the residuals by the parameters, a row for each observation."""
        self._set(x)
        return [[d / o.uncertainty for d in self._derivatives(o.symbol, o.property, o.T, o.P)] for o in self.observations]

    def constraint_residuals(self, x):
        """Calculated minus required value of each constraint."""
        return [r for r, _ in self._constraint_rows(x)]

    def _constraint_rows(self, x):
        self._set(x)
        rows = []
        for c in self.constraints:
            row = [0.0] * len(self.keys)
            if isinstance(c, PropertyConstraint):
                calculated = self._value(c.symbol, c.property, c.T, c.P)
                row = self._derivatives(c.symbol, c.property, c.T, c.P)
            else:
                calculated = 0.0
            for key, coefficient in c.terms.items():
                calculated += coefficient * self._parameter_value(key, x)
                if tuple(key) in self.keys:
                    row[self.keys.index(tuple(key))] += coefficient
            rows.append((calculated - c.value, row))
        return rows

    # ---- solvers
    def solve(self, **options):
        """Without constraints and search: scipy.optimize.least_squares with the exact Jacobian; options go to that
        function. The parameters are scaled by their start values, unless x_scale is given.
        With constraints or search: Gauss-Newton steps with the constraints in a KKT system (options x_scale,
        max_iterations = 50, xtol = 1e-10), and for search a one-dimensional search (options search_step = 1 % of the
        start value, search_tol = search_step / 100, max_evaluations = 500, search_fixed_step = False: True keeps
        the step as OptimB does)."""
        if self.constraints or self.search is not None:
            return self._solve_constrained(**options)
        try:
            import numpy
            from scipy.optimize import least_squares
        except ImportError as error:
            raise ImportError("thermofun.fitting needs SciPy: pip install thermofun[fitting]") from error
        start = [h.value() for h in self.handles]
        options.setdefault("x_scale", [abs(x) if x != 0.0 else 1.0 for x in start])
        solution = least_squares(self.residuals, start, jac=lambda x: numpy.array(self.jacobian(x)), **options)
        self._set(solution.x)
        chi2 = float(numpy.sum(numpy.array(self.residuals(solution.x)) ** 2))
        scale = numpy.array(options["x_scale"]) if not isinstance(options["x_scale"], str) else numpy.ones(len(start))
        covariance = self._covariance(solution.x, scale)
        return Result(self, solution.x, covariance, chi2, solution.success, solution.message, solution)

    def _covariance(self, x, scale):
        """(J^T J)^-1 on the scaled parameters, restricted to the directions the constraints leave free."""
        import numpy
        J = numpy.array(self.jacobian(x)) * scale
        n = len(self.keys)
        if self.constraints:
            A = numpy.array([row for _, row in self._constraint_rows(x)]) * scale
            _, s, vt = numpy.linalg.svd(A)
            rank = int(numpy.sum(s > s.max() * 1e-12)) if s.size else 0
            Z = vt[rank:].T
        else:
            Z = numpy.eye(n)
        if Z.shape[1] == 0:
            return numpy.zeros((n, n)).tolist()
        H = Z.T @ J.T @ J @ Z
        return (Z @ numpy.linalg.pinv(H) @ Z.T * numpy.outer(scale, scale)).tolist()

    def _kkt_step(self, x, active, scale):
        import numpy
        r = numpy.array(self.residuals(x))
        J = numpy.array(self.jacobian(x))[:, active] * scale[active]
        rows = self._constraint_rows(x)
        A = numpy.array([row for _, row in rows]).reshape(len(rows), len(self.keys))[:, active] * scale[active]
        c = numpy.array([value for value, _ in rows])
        k, m = int(active.sum()), len(rows)
        H = J.T @ J
        # each constraint row scaled to the size of H: the same equation, but a KKT matrix the solver can resolve
        size = numpy.sqrt(max(numpy.max(numpy.abs(numpy.diag(H))), 1e-300)) if k else 1.0
        norms = numpy.linalg.norm(A, axis=1) if m else numpy.zeros(0)
        keep = norms > 0.0     # a constraint on parameters that do not move here (the searched one) has no row
        A, c = A[keep] * (size / norms[keep])[:, None], c[keep] * (size / norms[keep])
        m = int(keep.sum())
        K = numpy.block([[H, A.T], [A, numpy.zeros((m, m))]])
        rhs = numpy.concatenate([-J.T @ r, -c])
        try:
            solution = numpy.linalg.solve(K, rhs)
        except numpy.linalg.LinAlgError:
            solution = numpy.linalg.lstsq(K, rhs, rcond=None)[0]
        step = numpy.zeros(len(self.keys))
        step[active] = solution[:k] * scale[active]
        return step

    def _gauss_newton(self, x, active, scale, max_iterations, xtol):
        import numpy
        x = numpy.array(x, float)
        for iteration in range(1, max_iterations + 1):
            step = self._kkt_step(x, active, scale)
            x = x + step
            if numpy.max(numpy.abs(step / scale)) < xtol:
                return x, True, iteration
        return x, False, max_iterations

    def _chi2(self, x):
        return float(sum(r * r for r in self.residuals(x)))

    def _solve_constrained(self, x_scale=None, max_iterations=50, xtol=1e-10, search_step=None, search_tol=None, max_evaluations=500,
                           search_fixed_step=False):
        import numpy
        start = numpy.array([h.value() for h in self.handles], float)
        scale = numpy.array(x_scale if x_scale is not None else [abs(v) if v != 0.0 else 1.0 for v in start], float)
        active = numpy.ones(len(self.keys), bool)
        if self.search is None:
            x, converged, iterations = self._gauss_newton(start, active, scale, max_iterations, xtol)
            message = f"converged in {iterations} iterations" if converged else f"not converged in {iterations} iterations"
            info = SimpleNamespace(iterations=iterations)
        else:
            x, converged, message, info = self._search(start, scale, max_iterations, xtol, search_step, search_tol, max_evaluations,
                                                       search_fixed_step)
        self._set(x)
        return Result(self, x, self._covariance(x, scale), self._chi2(x), converged, message, info)

    def _search(self, start, scale, max_iterations, xtol, step, tol, max_evaluations, fixed_step):
        """The search of OptimB (Shvarov 2015, Proc_415FD0): from the start value, steps of `step` downhill; at a
        minimum a parabola through three points; the step grows tenfold (up to 1000 times) on a plateau; stop when
        the parabola moves less than `tol`. The other parameters are solved for each value.
        OptimB keeps the step: its parabola then misses the minimum by an amount of the order of step^2 times the third
        derivative. Here the step shrinks to twice the last move (not below tol), unless fixed_step."""
        import numpy
        i = self.search
        inner = numpy.ones(len(self.keys), bool)
        inner[i] = False
        s = step if step is not None else (abs(start[i]) * 0.01 if start[i] != 0.0 else 1.0)
        tol = tol if tol is not None else s / 100.0
        s_max = s * 1000.0
        evaluations = [0]
        inner_ok = [True]

        def f(v, x0):
            if evaluations[0] >= max_evaluations:
                raise StopIteration
            evaluations[0] += 1
            x = numpy.array(x0, float)
            x[i] = v
            if inner.any():
                x, converged, _ = self._gauss_newton(x, inner, scale, max_iterations, xtol)
                inner_ok[0] = inner_ok[0] and converged
            return self._chi2(x), x

        v = start[i]
        steps = 0
        x0 = start
        try:
            f0, x0 = f(v, start)
            while True:
                fA, xA = f(v + s, x0)
                fB, xB = f(v - s, x0)
                while not (f0 <= fA and f0 <= fB):     # downhill in steps of s
                    if fA < fB:
                        v, f0, x0, fB, xB = v + s, fA, xA, f0, x0
                        fA, xA = f(v + s, x0)
                    else:
                        v, f0, x0, fA, xA = v - s, fB, xB, f0, x0
                        fB, xB = f(v - s, x0)
                curvature = (fA + fB - 2.0 * f0) / s ** 2
                if curvature <= 0.0:                      # flat: a larger step
                    if s >= s_max:
                        return x0, False, f"no minimum of the searched parameter found ({evaluations[0]} evaluations)", SimpleNamespace(evaluations=evaluations[0], steps=steps)
                    s *= 10.0
                    continue
                t = -(fA - fB) / (2.0 * s) / curvature
                t = max(-s, min(s, t))
                v += t
                steps += 1
                if not fixed_step:
                    s = max(tol, min(s, 2.0 * abs(t)))
                f0, x0 = f(v, x0)
                if abs(t) <= tol:
                    ok = inner_ok[0]
                    message = f"search converged: {steps} parabola steps, {evaluations[0]} evaluations"
                    if not ok:
                        message += "; an inner solve did not converge"
                    return x0, ok, message, SimpleNamespace(evaluations=evaluations[0], steps=steps)
        except StopIteration:
            return x0, False, f"search stopped after {max_evaluations} evaluations", SimpleNamespace(evaluations=evaluations[0], steps=steps)


# ---- fit templates and tasks -------------------------------------------------------------------------------------
# A fit template (fit_templates/<name>.yaml) says what can be fitted for one method and how: the parameters and their
# units, the constraints (each a switch), conventions, rules and presets. A task names a template, a record, its
# data and given values. Task(engine, task) turns both into one Problem.

import ast as _ast
import operator as _operator
import os as _os

TEMPLATE_FOLDER = _os.path.join(_os.path.dirname(__file__), "fit_templates")
_SI_OF_PROPERTY = {"G": "J/mol", "H": "J/mol", "S": "J/(mol*K)", "Cp": "J/(mol*K)", "V": "m3/mol", "U": "J/mol", "A": "J/mol",
                   "logK": "1", "lnK": "1", "dG": "J/mol", "dH": "J/mol", "dS": "J/(mol*K)", "dCp": "J/(mol*K)", "dV": "m3/mol"}
_OPERATORS = {_ast.Add: _operator.add, _ast.Sub: _operator.sub, _ast.Mult: _operator.mul, _ast.Div: _operator.truediv,
              _ast.Pow: _operator.pow}
_COMPARISONS = {_ast.Lt: _operator.lt, _ast.LtE: _operator.le, _ast.Gt: _operator.gt, _ast.GtE: _operator.ge}


def evaluate(expression, names):
    """The value of an expression of a template: numbers, names (dotted, as given.Cp), + - * / **, parentheses, and
    one comparison (< <= > >=) for the rules. Nothing else: no calls, no other attributes."""
    if isinstance(expression, (int, float)) and not isinstance(expression, bool):
        return float(expression)

    def name_of(node):
        if isinstance(node, _ast.Name):
            return node.id
        if isinstance(node, _ast.Attribute):
            return f"{name_of(node.value)}.{node.attr}"
        raise ValueError(f"Not allowed in an expression of a fit template: {expression!r}")

    def value(node):
        if isinstance(node, _ast.Constant) and isinstance(node.value, (int, float)) and not isinstance(node.value, bool):
            return float(node.value)
        if isinstance(node, _ast.BinOp) and type(node.op) in _OPERATORS:
            return _OPERATORS[type(node.op)](value(node.left), value(node.right))
        if isinstance(node, _ast.UnaryOp) and isinstance(node.op, (_ast.USub, _ast.UAdd)):
            v = value(node.operand)
            return -v if isinstance(node.op, _ast.USub) else v
        if isinstance(node, _ast.Compare) and len(node.ops) == 1 and type(node.ops[0]) in _COMPARISONS:
            return _COMPARISONS[type(node.ops[0])](value(node.left), value(node.comparators[0]))
        if isinstance(node, (_ast.Name, _ast.Attribute)):
            name = name_of(node)
            if name not in names:
                raise ValueError(f"Unknown name {name} in {expression!r}. Known: {', '.join(sorted(names))}.")
            return names[name]
        raise ValueError(f"Not allowed in an expression of a fit template: {expression!r}")

    return value(_ast.parse(str(expression).strip(), mode="eval").body)


def _read_document(source, key):
    """A dict, or the path of a YAML or JSON file; the part under `key` if the document has it."""
    if isinstance(source, dict):
        document = source
    else:
        with open(source, encoding="utf-8") as f:
            text = f.read()
        if str(source).endswith((".yaml", ".yml")):
            try:
                import yaml
            except ImportError as error:
                raise ImportError("Fit templates and tasks in YAML need PyYAML: pip install thermofun[fitting]") from error
            document = yaml.safe_load(text)
        else:
            import json
            document = json.loads(text)
    return document.get(key, document)


def template(name_or_path):
    """A fit template as a dict: a name of the catalog of ThermoFun (catalog()), a path, or a dict."""
    if isinstance(name_or_path, str) and not _os.path.exists(name_or_path):
        path = _os.path.join(TEMPLATE_FOLDER, f"{name_or_path}.yaml")
        if not _os.path.exists(path):
            raise ValueError(f"No fit template {name_or_path!r}. ThermoFun has: {', '.join(row['name'] for row in catalog())}.")
        name_or_path = path
    return _read_document(name_or_path, "fit_template")


def catalog(name=""):
    """The fit templates of ThermoFun (a list of rows, pandas.DataFrame(catalog()) is a table), or with a name one template:
    name, method, summary, presets, source (the path of its file)."""
    def row(file):
        path = _os.path.join(TEMPLATE_FOLDER, file)
        t = _read_document(path, "fit_template")
        return {"name": t["name"], "method": t.get("method", ""), "summary": " ".join(str(t.get("description", "")).split()),
                "presets": list(t.get("presets", {})), "source": path}
    files = sorted(f for f in _os.listdir(TEMPLATE_FOLDER) if f.endswith(".yaml")) if _os.path.isdir(TEMPLATE_FOLDER) else []
    if not name:
        return [row(f) for f in files]
    if f"{name}.yaml" not in files:
        raise ValueError(f"No fit template {name!r}. ThermoFun has: {', '.join(f[:-5] for f in files)}.")
    return row(f"{name}.yaml")


def _unit(text):
    return str(text).replace(".", "*")   # the units of the catalog write J/(mol.K); the converter wants J/(mol*K)


def _to_si(quantity, si_unit):
    """A number (SI) or a text "number unit" in SI."""
    from thermofun import convert_units
    if isinstance(quantity, (int, float)):
        return float(quantity)
    number, _, unit = str(quantity).strip().partition(" ")
    unit = unit.strip()
    return float(number) if not unit else convert_units(float(number), _unit(unit), _unit(si_unit))


class Task:
    """A fit set up from a template and a task (a dict or a YAML or JSON file under the key fit_task):

        template: HKF88                  # a name of catalog(), a path, or a dict
        record: UO2+2
        preset: psat_only                # optional: the free parameters and the constraints that are on
        free: [a1, a4, c1, c2, omega]    # optional: instead of those of the preset
        switches: {V_reference: off}     # optional: constraints on or off, after the preset
        conventions: {born_reference: SH88_fixed}
        given: {Cp: 10.04 cal/(mol*K), V: 6.1 cm3/mol}   # values the constraints use; a number is SI
        start: {omega: 144618}           # optional: start values, in the units of the template
        data:                            # the observations
          property: G                    # the default of the rows; a column "property" can change it
          unit: kJ/mol                   # of the values and the uncertainties
          columns: [T_C, P_bar, value, uncertainty]   # T_C or T_K; P_bar, P_MPa or P_Pa (0 = Psat); uncertainty or weight (= 1/uncertainty)
          rows: [[10, 0, -954.45, 0.5], ...]          # or file: data.csv (a header row with the column names)
        solve: {search_step: 1000}       # optional: options of Problem.solve, the search ones in the unit of the parameter

    Attributes: problem, warnings (from the rules of the template), free, constraints_on. solve() gives a Result;
    report(result) gives the values in the units of the template."""

    def __init__(self, engine, task):
        spec = _read_document(task, "fit_task")
        self.engine = engine
        self.spec = spec
        self.template = template(spec["template"])
        t = self.template
        self.record = spec["record"]
        self.method = t["method"]
        parameters = t["parameters"]
        preset = t.get("presets", {}).get(spec["preset"]) if spec.get("preset") else {}
        if preset is None:
            raise ValueError(f"No preset {spec['preset']!r} in the template {t['name']}. It has: {', '.join(t.get('presets', {}))}.")
        self.free = list(spec.get("free", preset.get("free", [])))
        if not self.free:
            raise ValueError("The task fits no parameters: give a preset or free.")
        for name in self.free:
            if name not in parameters:
                raise ValueError(f"{name} is not a parameter of the template {t['name']}. It has: {', '.join(parameters)}.")
        on = {name: False for name in t.get("constraints", {})}
        for name in preset.get("constraints", []):
            on[name] = True
        for name, state in spec.get("switches", {}).items():
            if name not in on:
                raise ValueError(f"No constraint {name} in the template {t['name']}. It has: {', '.join(on)}.")
            on[name] = state is True or str(state).lower() in ("on", "true", "yes")
        self.constraints_on = [name for name, state in on.items() if state]

        for name, value in spec.get("start", {}).items():
            self._handle(name).setValue(_to_si(f"{value} {parameters[name]['unit']}", self._si_unit(name)))
        self.observations = self._observations(spec["data"])
        self.names = self._names(spec)
        self.warnings = self._rules()
        constraints = [self._constraint(name) for name in self.constraints_on]
        search = None
        for name in self.free:
            if "search" in parameters[name]:
                search = self._key(name)
        self.solve_options = self._solve_options(spec.get("solve", {}), search)
        self.problem = Problem(engine, [self._key(n) for n in self.free], self.observations, constraints, search)

    # parameters
    def _key(self, name):
        method = self.template["parameters"][name].get("method", self.method)
        return (self.record, method, name)

    def _handle(self, name):
        return self.engine.parameter(*self._key(name))

    def _si_unit(self, name):
        if self.template["parameters"][name].get("method", self.method) == "":
            return _SI_OF_PROPERTY[name]
        from thermofun.PyThermoFun import methods
        spec = next(s for s in methods.catalog() if s["id"] == self.method)
        return next(p["si_unit"] for p in spec["parameters"] if p["name"] == name)

    def _from_template_unit(self, name, value):
        return _to_si(f"{value} {self.template['parameters'][name]['unit']}", self._si_unit(name))

    # data
    def _observations(self, data):
        if "file" in data:
            import csv
            with open(data["file"], newline="", encoding="utf-8") as f:
                rows = list(csv.reader(f))
            columns, rows = rows[0], rows[1:]
        else:
            columns, rows = data["columns"], data["rows"]
        columns = [c.strip() for c in columns]
        unit = data.get("unit")
        out = []
        for row in rows:
            r = dict(zip(columns, row))
            prop = str(r.get("property", data.get("property", "G"))).strip()
            T = float(r["T_C"]) + 273.15 if "T_C" in r else float(r["T_K"])
            P = float(r["P_bar"]) * 1e5 if "P_bar" in r else float(r["P_MPa"]) * 1e6 if "P_MPa" in r else float(r["P_Pa"])
            factor = _to_si(f"1 {unit}", _SI_OF_PROPERTY[prop]) if unit else 1.0
            if "uncertainty" in r:
                u = float(r["uncertainty"])
            elif "weight" in r:
                if float(r["weight"]) == 0.0:
                    continue          # weight 0: the point is not used (OptimB)
                u = 1.0 / float(r["weight"])
            else:
                u = 1.0
            out.append(Observation(str(r.get("symbol", self.record)).strip(), prop, T, P, float(r["value"]) * factor, u * factor))
        if not out:
            raise ValueError("The task has no data.")
        return out

    # names of the expressions
    def _names(self, spec):
        t = self.template
        Tr, Pr = 298.15, 1e5
        names = {"Tr": Tr, "Pr": Pr, "cal": 4.184}
        for prop, value in spec.get("given", {}).items():
            names[f"given.{prop}"] = _to_si(value, _SI_OF_PROPERTY.get(prop, "1"))
        try:
            born = self.engine.electroPropertiesSolvent(Tr, Pr, "H2O@")
            names.update({"solvent.bornQ": born.bornQ.val, "solvent.bornX": born.bornX.val, "solvent.bornY": born.bornY.val})
        except Exception:
            pass                      # no water: the expressions that need it fail with the list of names
        for convention, setup in t.get("conventions", {}).items():
            choice = spec.get("conventions", {}).get(convention, setup.get("default"))
            if choice not in setup.get("choices", {}):
                raise ValueError(f"No choice {choice!r} of {convention} in the template {t['name']}. It has: {', '.join(setup.get('choices', {}))}.")
            values = setup["choices"][choice] or {}
            if convention == "born_reference" and not values:
                values = {"Q": names.get("solvent.bornQ"), "X": names.get("solvent.bornX")}
            for k, v in values.items():
                if v is not None:
                    names[f"{convention}.{k}"] = float(v)
        self.conventions = {c: spec.get("conventions", {}).get(c, s.get("default")) for c, s in t.get("conventions", {}).items()}
        T = [o.T for o in self.observations]
        P = [o.P if o.P > 0.0 else max(Pr, self.engine.saturationPressure(o.T)) for o in self.observations]
        names["span_T"], names["span_P"] = max(T) - min(T), max(P) - min(P)
        return names

    def _constraint(self, name):
        c = self.template["constraints"][name]
        needs = c.get("needs_any_free")
        if needs and not any(n in self.free for n in needs):
            raise ValueError(f"The constraint {name} needs one of {', '.join(needs)} fitted.")
        terms = {self._key(p): evaluate(e, self.names) for p, e in c.get("terms", {}).items()}
        value = evaluate(c["value"], self.names)
        if "property" in c:
            T = evaluate(c.get("T", "Tr"), self.names)
            P = evaluate(c.get("P", "Pr"), self.names)
            return PropertyConstraint(self.record, c["property"], T, P, value, terms)
        relation = {self._key(p): evaluate(e, self.names) for p, e in c["relation"].items()}
        for key, coefficient in terms.items():
            relation[key] = relation.get(key, 0.0) + coefficient
        return Relation(relation, value)

    def _rules(self):
        warnings = []
        for rule in self.template.get("rules", []):
            if not evaluate(rule["when"], self.names):
                continue
            free = [n for n in rule["of"] if n in self.free]
            if len(free) > rule["max_free"] and not any(n in self.constraints_on for n in rule.get("unless_on", [])):
                warnings.append(f"{', '.join(free)} fitted together: {rule['message']}")
        return warnings

    def _solve_options(self, options, search):
        options = dict(options)
        if search is not None:
            setup = self.template["parameters"][search[2]]["search"]
            for option, key in (("search_step", "step"), ("search_tol", "tol")):
                value = options.get(option, setup.get(key))
                if value is not None:
                    options[option] = self._from_template_unit(search[2], value)
        return options

    def solve(self):
        """Solve the problem; then add to warnings the values outside the plausible ranges of the template."""
        result = self.problem.solve(**self.solve_options)
        for name, (low, high) in (self.template.get("plausible") or {}).items():
            if name in self.free:
                shown = self.report(result)[name]["value"]
                if not low <= shown <= high:
                    self.warnings.append(f"{name} = {shown:g} is outside the plausible range [{low:g}, {high:g}] of the template")
        return result

    def report(self, result):
        """The fitted values and their uncertainties in the units of the template, times display_scale (the
        convention of OptimB and SUPCRT: a1*10, a2*1e-2, a4*1e-4, c2*1e-4, omega*1e-5)."""
        out = {}
        for name in self.free:
            p = self.template["parameters"][name]
            per_si = 1.0 / self._from_template_unit(name, 1.0)
            scale = float(p.get("display_scale", 1.0))
            key = "/".join(k for k in self._key(name) if k)
            out[name] = {"value": result.values[key] * per_si * scale, "uncertainty": result.uncertainties[key] * per_si * scale,
                         "unit": p["unit"], "display_scale": scale}
        return out
