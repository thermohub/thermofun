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

Needs SciPy (optional dependency: thermofun[fitting]). All numbers are in SI: V in m3/mol.
"""

from dataclasses import dataclass

from thermofun.PyThermoFun import *  # noqa: F401,F403

SUBSTANCE_FIELDS = {"G": "gibbs_energy", "H": "enthalpy", "S": "entropy", "Cp": "heat_capacity_cp", "V": "volume",
                    "U": "internal_energy", "A": "helmholtz_energy"}
REACTION_FIELDS = {"logK": "log_equilibrium_constant", "lnK": "ln_equilibrium_constant", "dG": "reaction_gibbs_energy", "dH": "reaction_enthalpy",
                   "dS": "reaction_entropy", "dCp": "reaction_heat_capacity_cp", "dV": "reaction_volume"}
_TO_SI = {"V": 1e-5, "dV": 1e-5}  # the engine gives J/bar


@dataclass
class Observation:
    """One measured value. property: G, H, S, Cp, V, U, A of a substance; logK, lnK, dG, dH, dS, dCp, dV of a reaction. uncertainty: its standard uncertainty, the weight in the fit."""
    symbol: str
    property: str
    T: float
    P: float
    value: float
    uncertainty: float = 1.0


class Result:
    """What a fit gives. values: by the name of the parameter. covariance: a matrix in the order of names. chi2: the sum of the squared weighted residuals."""

    def __init__(self, problem, solution, covariance, chi2):
        self._problem = problem
        self.names = problem.names
        self.values = dict(zip(problem.names, [float(x) for x in solution.x]))
        self.covariance = covariance
        self.uncertainties = dict(zip(problem.names, [float(covariance[i][i]) ** 0.5 for i in range(len(problem.names))]))
        self.chi2 = chi2
        self.degrees_of_freedom = len(problem.observations) - len(problem.names)
        self.success = bool(solution.success)
        self.message = str(solution.message)
        self.scipy = solution

    def database(self):
        """A database whose records have the fitted values and their covariance. Saved in v2 format, it keeps them."""
        problem = self._problem
        n = len(problem.names)
        for handle, value in zip(problem.handles, self.scipy.x):
            handle.setValue(float(value))
        flat = [float(self.covariance[i][j]) for i in range(n) for j in range(n)]
        self.covariance_dropped = problem.engine.setParameterCovariance(problem.keys, flat)
        return problem.engine.commitParameters()


class Problem:
    """A least squares problem: the parameters as (symbol, method, parameter), method "" for a reference value."""

    def __init__(self, engine, parameters, observations):
        if not parameters:
            raise ValueError("There are no parameters to fit.")
        if len(observations) < len(parameters):
            raise ValueError(f"{len(observations)} observations cannot fix {len(parameters)} parameters.")
        for o in observations:
            if o.property not in SUBSTANCE_FIELDS and o.property not in REACTION_FIELDS:
                raise ValueError(f"Unknown property {o.property}. Use one of {sorted(SUBSTANCE_FIELDS)} or {sorted(REACTION_FIELDS)}.")
            if not o.uncertainty > 0.0:
                raise ValueError(f"The uncertainty of an observation of {o.symbol} must be above 0.")
        self.engine = engine
        self.keys = [tuple(p) for p in parameters]
        self.handles = [engine.parameter(*p) for p in self.keys]
        self.names = [h.name() for h in self.handles]
        self.observations = list(observations)

    def _calculate(self, o):
        if o.property in SUBSTANCE_FIELDS:
            return getattr(self.engine.thermoPropertiesSubstance(o.T, o.P, o.symbol), SUBSTANCE_FIELDS[o.property]).val * _TO_SI.get(o.property, 1.0)
        return getattr(self.engine.thermoPropertiesReaction(o.T, o.P, o.symbol), REACTION_FIELDS[o.property]).val * _TO_SI.get(o.property, 1.0)

    def _set(self, x):
        for handle, value in zip(self.handles, x):
            if handle.value() != value:
                handle.setValue(float(value))

    def residuals(self, x):
        """(calculated - measured) / uncertainty for each observation."""
        self._set(x)
        return [(self._calculate(o) - o.value) / o.uncertainty for o in self.observations]

    def jacobian(self, x):
        """The exact derivatives of the residuals by the parameters, a row for each observation."""
        self._set(x)
        rows = []
        for o in self.observations:
            function = self.engine.parameterSensitivities if o.property in SUBSTANCE_FIELDS else self.engine.reactionParameterSensitivities
            d = function(o.T, o.P, o.symbol, self.keys)[o.property]
            rows.append([d[name] / o.uncertainty for name in self.names])
        return rows

    def solve(self, **options):
        """Solve with scipy.optimize.least_squares and the exact Jacobian. options go to that function. The parameters are scaled by their start values, unless x_scale is given."""
        try:
            import numpy
            from scipy.optimize import least_squares
        except ImportError as error:
            raise ImportError("thermofun.fitting needs SciPy: pip install thermofun[fitting]") from error
        start = [h.value() for h in self.handles]
        options.setdefault("x_scale", [abs(x) if x != 0.0 else 1.0 for x in start])
        solution = least_squares(self.residuals, start, jac=lambda x: numpy.array(self.jacobian(x)), **options)
        self._set(solution.x)
        J = numpy.array(self.jacobian(solution.x))
        chi2 = float(numpy.sum(numpy.array(self.residuals(solution.x)) ** 2))
        # the scales keep the matrix well conditioned
        scale = numpy.array(options["x_scale"]) if not isinstance(options["x_scale"], str) else numpy.ones(len(start))
        scaled = J * scale
        covariance = numpy.linalg.pinv(scaled.T @ scaled) * numpy.outer(scale, scale)
        return Result(self, solution, covariance.tolist(), chi2)
