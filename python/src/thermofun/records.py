# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Builders of v2 records (the C++ class RecordBuilder).

    from thermofun import records as rec

    quartz = (rec.substance("Quartz", formula="SiO2", aggregate_state="crystal", class_="component")
                 .standard_state(G=-856288, H=-910700, S=41.46, Cp=44.6, V="22.688 cm3/mol")
                 .method("CpIntegration", segments=[dict(t_min=298.15, t_max=844, a0=44.6, a1=0.0378, a2=-1.0e6)])
                 .method("MurnaghanHP98", alpha0=1.24e-5, kappa0="730 kbar"))
    problems = quartz.check()   # the reader and validate() on this record, now
    database.addSubstance(quartz.to_json())

A value is a number, a text "number unit", or a dictionary {value, unit, uncertainty}: the same forms as in a file.
The builders write ordinary v2 records; the reader checks them.
"""

from thermofun.PyThermoFun import RecordBuilder

substance = RecordBuilder.substance
reaction = RecordBuilder.reaction
element = RecordBuilder.element

__all__ = ["RecordBuilder", "substance", "reaction", "element"]
