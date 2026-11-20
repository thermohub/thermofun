# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

import json
import pathlib

import thermofun as tf
from thermofun import records as rec

V2 = pathlib.Path(__file__).resolve().parent / "data" / "v2"


def fixture(name, kind, symbol):
    return json.loads((V2 / name).read_text())[kind][symbol]


def test_the_builder_gives_the_fixture_of_a_substance():
    wanted = fixture("hkf.json", "substances", "Na+")
    built = (rec.substance("Na+", name=wanted["name"], formula=wanted["formula"], charge=wanted["charge"],
                           class_=wanted["class"], aggregate_state=wanted["aggregate_state"],
                           reference_conditions=wanted["reference_conditions"])
             .standard_state(**wanted["standard_state_properties"]))
    for m in wanted["methods"]:
        parameters = m["parameters"]
        built.method(m["method"], implementation=m.get("implementation"), **parameters)
    assert built.to_dict() == dict(wanted, symbol="Na+")


def test_the_builder_gives_the_fixture_with_segments():
    wanted = fixture("minerals.json", "substances", "Quartz")
    built = rec.substance("Quartz", **{k: v for k, v in wanted.items() if k not in ("methods", "standard_state_properties")})
    built.standard_state(**wanted["standard_state_properties"])
    for m in wanted["methods"]:
        if "segments" in m:
            built.method(m["method"], segments=[dict(s["parameters"], t_min=s.get("t_min"), t_max=s.get("t_max")) if "t_min" in s else dict(s["parameters"])
                                                for s in m["segments"]])
        else:
            built.method(m["method"], **{k: v for k, v in m.items() if k != "method"}.get("parameters", {}))
    result = built.to_dict()
    assert result["methods"][0]["method"] == wanted["methods"][0]["method"]
    assert result["methods"][0]["segments"][0]["parameters"] == wanted["methods"][0]["segments"][0]["parameters"]


def test_values_with_units_and_the_record():
    quartz = (rec.substance("Q", formula="SiO2", aggregate_state="crystal", class_="component", charge=0)
              .standard_state(G=-856288, H=-910700, S=41.46, Cp=44.6, V="22.688 cm3/mol")
              .method("CpIntegration", segments=[dict(t_min=298.15, t_max=844, a0=44.6, a1=0.0378, a2=-1.0e6)])
              .method("MurnaghanHP98", alpha0=1.24e-5, kappa0="730 kbar"))
    record = quartz.record()
    assert abs(record.standard_state_properties[tf.PropertyName.V].value - 2.2688e-5) < 1e-12
    assert record.methods[1].parameters["kappa0"].value == 730e8
    db = tf.Database()
    db.addSubstance(quartz.to_json())
    assert db.containsSubstance("Q")


def test_reaction_and_element_builders():
    r = rec.reaction("R", stoichiometry={"A": 1, "B": -1}).properties(logK={"value": -1.0, "uncertainty": 0.1}, dH=1000)
    assert r.record().reaction_properties[tf.ReactionPropertyName.logK].uncertainty.standard == 0.1
    e = rec.element("Fe", number=26, molar_mass=55.845, class_="element")
    assert e.record().number == 26


def test_check_gives_no_problems_for_a_good_record():
    quartz = (rec.substance("Q", formula="SiO2", aggregate_state="crystal", class_="component", charge=0)
              .standard_state(G=-856288, H=-910700, S=41.46, Cp=44.6, V="22.688 cm3/mol")
              .method("ConstantCp"))
    assert quartz.check() == []


def test_check_shows_a_problem_at_the_line_that_made_the_record():
    bad = rec.substance("Q", formula="SiO2", aggregate_state="crystal", class_="component").method("NoSuchMethod")
    assert any(d.severity == tf.Severity.Error for d in bad.check())
    assert any("Pa" in d.message or "unit" in d.message.lower() for d in
               rec.substance("Q", formula="SiO2", aggregate_state="crystal", class_="component").standard_state(G="1 parsec", H=0, S=1, Cp=1, V=1e-5).method("ConstantCp").check())


def test_check_of_a_reaction_does_not_ask_for_its_species():
    assert [d for d in rec.reaction("R", equation="A = B").properties(logK=1.0).method("ConstantReactionProperties").check() if d.severity == tf.Severity.Error] == []
