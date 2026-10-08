# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""A substance with the empirical Cp method but no Cp coefficients uses a constant Cp."""
import json
import math
import os

import pytest
import thermofun as tf

HERE = os.path.dirname(os.path.abspath(__file__))
DB = os.path.join(HERE, "test-thermoengine-thermofun.json")
FLT_MIN = 1.1754943508222875e-38  # the "empty" value GEMS writes for a missing number


def engine(tmp_path, cp_ref):
    with open(DB) as f:
        d = json.load(f)
    cal = next(s for s in d["substances"] if s["symbol"] == "Cal")
    cal["TPMethods"][0] = {"method": {"0": "cp_ft_equation"}}
    cal["sm_heat_capacity_p"]["values"] = [cp_ref]
    path = tmp_path / "db.json"
    path.write_text(json.dumps(d))
    return tf.ThermoEngine(str(path)), cal


def constant_cp(T, cp, G0, S0, H0, Tr=298.15):
    G = G0 - S0 * (T - Tr) - cp * (T * math.log(T / Tr) - (T - Tr))
    return G, H0 + cp * (T - Tr), S0 + cp * math.log(T / Tr)


@pytest.mark.parametrize("cp_ref", [81.87109375, FLT_MIN])
@pytest.mark.parametrize("T", [273.15, 298.15, 373.15])
def test_constant_cp_at_the_reference_value(tmp_path, cp_ref, T):
    eng, cal = engine(tmp_path, cp_ref)
    tps = eng.thermoPropertiesSubstance(T, 1e5, "Cal")
    ref = eng.thermoPropertiesSubstance(298.15, 1e5, "Cal")
    cp = 0.0 if cp_ref == FLT_MIN else cp_ref
    G, H, S = constant_cp(T, cp, ref.gibbs_energy.val, ref.entropy.val, ref.enthalpy.val)
    assert tps.heat_capacity_cp.val == pytest.approx(cp, abs=1e-12)
    assert tps.gibbs_energy.val == pytest.approx(G, rel=1e-12, abs=1e-8)
    assert tps.enthalpy.val == pytest.approx(H, rel=1e-12, abs=1e-8)
    assert tps.entropy.val == pytest.approx(S, rel=1e-12, abs=1e-10)
