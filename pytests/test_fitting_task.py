# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

# Fit tasks set up from a fit template (python/src/thermofun/fit_templates/HKF88.yaml): the task gives the same fit as
# the problem written by hand in test_fitting_hkf.py; presets, switches, conventions, rules and the expressions.

import copy
import json
import os

import pytest

fitting = pytest.importorskip("thermofun.fitting")
pytest.importorskip("numpy")

import test_fitting_hkf as hkf  # noqa: E402  (the UO2+2 example of OptimB)

CAL = hkf.CAL


def task(**changes):
    t = {"template": "HKF88", "record": "UO2+2", "preset": "psat_only",
         "conventions": {"born_reference": "SH88_fixed"},
         "given": {"Cp": "10.04 cal/(mol*K)", "V": "6.1 cm3/mol"},
         "data": {"property": "G", "unit": "kJ/mol", "columns": ["T_C", "P_bar", "weight", "value"],
                  "rows": [[t, 0, w, g] for t, w, g in hkf.DATA]}}
    t.update(changes)
    return t


def test_the_templates_of_thermofun():
    row = fitting.catalog("HKF88")
    assert [r["name"] for r in fitting.catalog()] == ["HKF88"]
    assert row["method"] == "HKF88" and row["presets"] == ["psat_only", "wide_TP"] and row["summary"].startswith("HKF parameters")
    assert row["source"] == os.path.join(fitting.TEMPLATE_FOLDER, "HKF88.yaml")
    template = fitting.template("HKF88")
    assert template["method"] == "HKF88" and "a2_a4_SH88" in template["constraints"]
    assert fitting.template(row["source"]) == template
    with pytest.raises(ValueError, match="No fit template 'NoSuchMethod'. ThermoFun has: HKF88"):
        fitting.template("NoSuchMethod")
    with pytest.raises(ValueError, match="No fit template"):
        fitting.catalog("NoSuchMethod")


def test_a_task_gives_the_fit_written_by_hand():
    by_hand = hkf.problem(hkf.engine()).solve(search_step=1000 * CAL, search_tol=0.01 * CAL)
    t = fitting.Task(hkf.engine(), task())
    assert t.free == ["a1", "a4", "c1", "c2", "omega"]
    assert t.constraints_on == ["Cp_reference", "V_reference", "a2_a4_SH88"]
    assert t.warnings == []
    result = t.solve()
    assert result.success, result.message
    for name, value in by_hand.values.items():
        assert result.values[name] == pytest.approx(value, rel=1e-7), name   # the inputs differ in the last digit (6.1 cm3/mol)
    assert max(abs(r) for r in result.constraint_residuals) < 1e-6
    shown = t.report(result)
    assert shown["omega"]["value"] == pytest.approx(result.values["UO2+2/HKF88/omega"] / CAL * 1e-5, rel=1e-12)
    assert shown["a1"]["value"] == pytest.approx(hkf.OPTIMB["a1"] * 10, rel=2e-3)        # a1*10 as in OptimB
    assert shown["omega"]["unit"] == "cal/mol" and shown["omega"]["display_scale"] == 1e-5


def test_the_born_convention_changes_the_constraints():
    solvent = fitting.Task(hkf.engine(), task(conventions={"born_reference": "solvent"})).solve()
    fixed = fitting.Task(hkf.engine(), task()).solve()
    # with the water of the engine at 25 C, terms are 0: Cp and V of ThermoFun itself are held
    assert abs(solvent.values["UO2+2/HKF88/omega"] - fixed.values["UO2+2/HKF88/omega"]) > 100.0
    e = hkf.engine()
    t = fitting.Task(e, task(conventions={"born_reference": "solvent"}))
    t.solve()
    r = e.thermoPropertiesSubstance(298.15, 1e5, "UO2+2")
    assert r.heat_capacity_cp.val == pytest.approx(10.04 * CAL, rel=1e-9)
    assert r.volume.val == pytest.approx(6.1e-6, rel=1e-9)


def test_switches_and_free_change_the_preset():
    t = fitting.Task(hkf.engine(), task(switches={"V_reference": "off", "c2_from_Cp_SSH97": "on"}))
    assert t.constraints_on == ["Cp_reference", "a2_a4_SH88", "c2_from_Cp_SSH97"]
    result = t.solve()
    c2 = result.values["UO2+2/HKF88/c2"]
    assert c2 == pytest.approx(2037 * 10.04 * CAL - 30346 * CAL, rel=1e-9)
    t = fitting.Task(hkf.engine(), task(preset=None, free=["c1", "omega"], switches={"Cp_reference": True}))
    assert t.free == ["c1", "omega"] and t.constraints_on == ["Cp_reference"]


def test_the_rules_of_the_template_warn():
    # along Psat only, a1-a4 together are not fixed by the data, unless V or the a2-a4 relation holds them
    t = fitting.Task(hkf.engine(), task(preset=None, free=["a1", "a2", "a3", "a4", "omega"]))
    assert len(t.warnings) == 1 and t.warnings[0].startswith("a1, a2, a3, a4 fitted together")
    t = fitting.Task(hkf.engine(), task(preset=None, free=["a1", "a2", "a3", "a4", "omega"], switches={"V_reference": "on"}))
    assert t.warnings == []


def test_a_task_from_files(tmp_path):
    yaml = pytest.importorskip("yaml")
    (tmp_path / "uo2.csv").write_text("T_C,P_bar,value,uncertainty\n" + "".join(f"{t},0,{g},{1 / w}\n" for t, w, g in hkf.DATA))
    spec = task(data={"property": "G", "unit": "kJ/mol", "file": str(tmp_path / "uo2.csv")})
    (tmp_path / "task.yaml").write_text(yaml.safe_dump({"fit_task": spec}))
    from_file = fitting.Task(hkf.engine(), str(tmp_path / "task.yaml")).solve()
    in_memory = fitting.Task(hkf.engine(), task()).solve()
    for name, value in in_memory.values.items():
        assert from_file.values[name] == pytest.approx(value, rel=1e-9)
    (tmp_path / "task.json").write_text(json.dumps({"fit_task": spec}))
    assert fitting.Task(hkf.engine(), str(tmp_path / "task.json")).free == ["a1", "a4", "c1", "c2", "omega"]


def test_a_point_of_weight_0_is_left_out():
    rows = [[t, 0, w, g] for t, w, g in hkf.DATA] + [[350, 0, 0, -999.0]]
    t = fitting.Task(hkf.engine(), task(data={"property": "G", "unit": "kJ/mol", "columns": ["T_C", "P_bar", "weight", "value"], "rows": rows}))
    assert len(t.observations) == len(hkf.DATA)


def test_start_values_in_the_units_of_the_template():
    e = hkf.engine()
    fitting.Task(e, task(start={"omega": 100000, "a1": 0.25}))
    assert e.parameter("UO2+2", "HKF88", "omega").value() == pytest.approx(100000 * CAL)
    assert e.parameter("UO2+2", "HKF88", "a1").value() == pytest.approx(0.25 * CAL / 1e5)


def test_plausible_ranges_warn_after_the_fit():
    template = copy.deepcopy(fitting.template("HKF88"))
    template["plausible"] = {"omega": [2.0, 3.0]}            # display units: omega*1e-5 cal/mol
    t = fitting.Task(hkf.engine(), task(template=template))
    t.solve()
    assert any(w.startswith("omega = 0.9") and "plausible range [2, 3]" in w for w in t.warnings)


def test_wrong_tasks():
    with pytest.raises(ValueError, match="No preset"):
        fitting.Task(hkf.engine(), task(preset="nope"))
    with pytest.raises(ValueError, match="No constraint"):
        fitting.Task(hkf.engine(), task(switches={"nope": "on"}))
    with pytest.raises(ValueError, match="not a parameter"):
        fitting.Task(hkf.engine(), task(preset=None, free=["b1"]))
    with pytest.raises(ValueError, match="needs one of c1, c2, omega"):
        fitting.Task(hkf.engine(), task(preset=None, free=["a1", "a4"], switches={"Cp_reference": "on"}))
    with pytest.raises(ValueError, match="Unknown name given.Cp"):
        fitting.Task(hkf.engine(), task(given={"V": 6.1e-6}))
    with pytest.raises(ValueError, match="No choice"):
        fitting.Task(hkf.engine(), task(conventions={"born_reference": "HK74"}))


def test_the_expressions_allow_only_arithmetic():
    names = {"Tr": 298.15, "given.Cp": 42.0}
    assert fitting.evaluate("-Tr*(1 - 2)", names) == pytest.approx(298.15)
    assert fitting.evaluate("2037*given.Cp - 30346*4.184", names) == pytest.approx(2037 * 42.0 - 30346 * 4.184)
    assert fitting.evaluate("2**3", {}) == 8.0
    assert fitting.evaluate("Tr < 300", names) is True
    assert fitting.evaluate(7, {}) == 7.0
    for bad in ("__import__('os')", "Tr.__class__", "[1, 2]", "lambda: 1", "True"):
        with pytest.raises(ValueError):
            fitting.evaluate(bad, names)
