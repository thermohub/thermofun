# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# psinagra-12-07 crashed the released library (0.6.x) with a segmentation fault: the record Pd has cp_ft_equation with T limits
# and no temperature intervals, and the old Cp integration indexed interval -1 (fixed on master in PR #87; the v2 CpPolynomial
# does not have the bug). The calculation runs in a child process, so a crash fails this test instead of stopping pytest.

import pathlib
import subprocess
import sys

DB = pathlib.Path(__file__).parent.parent / "Resources" / "databases" / "psinagra-12-07-thermofun.json"

SCRIPT = f"""
import thermofun as tf
engine = tf.ThermoEngine({str(DB)!r})
substances = engine.database().getSubstancesList()
for symbol in substances:
    for T, P in ((298.15, 1e5), (473.15, 1e7)):
        engine.thermoPropertiesSubstance(T, P, symbol)
for symbol in engine.database().getReactionsList():
    engine.thermoPropertiesReaction(473.15, 1e7, symbol)
print(len(substances), engine.thermoPropertiesSubstance(298.15, 1e5, "Pd").heat_capacity_cp.val)
"""


def test_every_record_of_psinagra_calculates_without_a_crash():
    done = subprocess.run([sys.executable, "-c", SCRIPT], capture_output=True, text=True, timeout=600)
    assert done.returncode == 0, (done.returncode, done.stderr[-2000:])
    count, cp = done.stdout.split()
    assert int(count) == 690
    assert abs(float(cp) - (23.974000930786 + 0.0059329001232982 * 298.15 - 35564 / 298.15**2)) < 1e-6   # Cp of Pd from its coefficients
