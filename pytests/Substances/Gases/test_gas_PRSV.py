import thermofun as thermofun
import pytest as pytest
import unittest


class TestCpfT(unittest.TestCase):

    def setUp(self):
        self.engine = thermofun.ThermoEngine('pytests/Substances/Gases/mines16-thermofun.json')
    
    def test_properties_substance(self):
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "O2").gibbs_energy.val == pytest.approx(-5159.744184316695, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "O2").enthalpy.val == pytest.approx(727.35314, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "O2").entropy.val == pytest.approx(207.53899, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "O2").heat_capacity_cp.val == pytest.approx(29.46353, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e8, "O2").gibbs_energy.val == pytest.approx(-15646.55883, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e8, "O2").enthalpy.val == pytest.approx(797.39243, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e8, "O2").entropy.val == pytest.approx(206.5307979, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e8, "O2").heat_capacity_cp.val == pytest.approx(29.8257057, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(700+273.15, 1e9, "O2").gibbs_energy.val == pytest.approx(-153207.61977, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(700+273.15, 1e9, "O2").enthalpy.val == pytest.approx(39375.342722, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(700+273.15, 1e9, "O2").entropy.val == pytest.approx(241.08859, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(700+273.15, 1e9, "O2").heat_capacity_cp.val == pytest.approx(34.726123, 1e-5, 1e-14)