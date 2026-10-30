# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Write iapws-check.json: water from an independent code, for test_water_methods.py.

    pip install --target <folder> --no-deps iapws
    PYTHONPATH=<folder> python pytests/data/sources/make_iapws_check.py

The code is jjgomera/iapws (github.com/jjgomera/iapws): IAPWS-95 and the dielectric constant of IAPWS R8-97.
The tests do not need this package: they read the file.
"""

import json
import pathlib

import iapws

M = 18.015268e-3  # kg/mol, as IAPWS-95
T_GRID = [273.16, 298.15, 323.15, 373.15, 473.15, 573.15, 673.15, 773.15, 873.15, 1073.15, 1273.0]
P_GRID = [1e5, 1e6, 1e7, 2.5e7, 1e8, 5e8, 1e9]  # Pa


def row(T, P):
    w = iapws.IAPWS95(T=T, P=P / 1e6)
    out = {"T": T, "P": P, "phase": "gas" if w.phase in ("Vapour", "Supercritical vapour") else "liquid", "rho": w.rho,
           "cp": w.cp * 1e3 * M, "s": w.s * 1e3 * M, "h": w.h * 1e3 * M}  # kg/m3, J/(mol K), J/(mol K), J/mol
    if T <= 873.15:  # the range of R8-97
        out["epsilon"] = iapws._iapws._Dielectric(w.rho, T)
    return out


def main():
    rows = [row(T, P) for T in T_GRID for P in P_GRID]
    # the differences of s and h from this point do not depend on the reference of the energies
    reference = row(298.15, 1e5)
    doc = {"source": "jjgomera/iapws %s: IAPWS95 and _Dielectric. T K, P Pa, rho kg/m3, cp and s J/(mol K), h J/mol; "
                     "s and h in the reference of IAPWS-95." % iapws.__version__,
           "reference": reference, "rows": rows}
    path = pathlib.Path(__file__).resolve().parent / "iapws-check.json"
    path.write_text(json.dumps(doc, indent=1) + "\n")


if __name__ == "__main__":
    main()
