# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2021-2026 ThermoFun contributors

import thermofun as thermofun
import pytest as pytest
import unittest


class TestCpfT(unittest.TestCase):

    def setUp(self):
        self.engine = thermofun.ThermoEngine('pytests/Substances/Gases/mines16-thermofun.json')
        # these numbers are of the state at the reference pressure, with the residual terms of H and S
        self.engine.preferences.apply_pressure_correction_to_gas_props = False
        self.engine.preferences.cancel_fugacity_coefficient_in_gas_props = True
    
    def test_properties_substance(self):
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "O2").gibbs_energy.val == pytest.approx(-5159.744184316695, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "O2").enthalpy.val == pytest.approx(727.35314, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "O2").entropy.val == pytest.approx(207.53899, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "O2").heat_capacity_cp.val == pytest.approx(29.5102266, 1e-5, 1e-14)  # with the residual Cp (was 29.46353)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e8, "O2").gibbs_energy.val == pytest.approx(-15646.55883, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e8, "O2").enthalpy.val == pytest.approx(797.39243, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e8, "O2").entropy.val == pytest.approx(206.5307979, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e8, "O2").heat_capacity_cp.val == pytest.approx(38.9017101, 1e-5, 1e-14)  # was 29.8257057
        assert self.engine.thermoPropertiesSubstance(700+273.15, 1e9, "O2").gibbs_energy.val == pytest.approx(-153207.61977, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(700+273.15, 1e9, "O2").enthalpy.val == pytest.approx(39375.342722, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(700+273.15, 1e9, "O2").entropy.val == pytest.approx(241.08859, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(700+273.15, 1e9, "O2").heat_capacity_cp.val == pytest.approx(38.1507522, 1e-5, 1e-14)  # was 34.726123

    def test_the_implementation_gems_has_no_residual_cp(self):
        """The values before 1.0, which are those of GEMS."""
        prefs = self.engine.preferences
        prefs.method_implementation = {"PRSV86": "gems"}
        self.engine.setPreferences(prefs)
        for T, P, cp in [(50+273.15, 1e5, 29.46353), (100+273.15, 1e8, 29.8257057), (700+273.15, 1e9, 34.726123)]:
            assert self.engine.thermoPropertiesSubstance(T, P, "O2").heat_capacity_cp.val == pytest.approx(cp, 1e-5, 1e-14)
