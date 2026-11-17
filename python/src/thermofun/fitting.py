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
        function = self.engine.parameterSensitivities if prop in SUBSTANCE_FIELDS else self.engine.reactionParameterSensitivities
        d = function(T, P, symbol, self.keys)[prop]
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
