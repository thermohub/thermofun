import thermofun as thermofun
import pytest as pytest
import unittest


class TestCpfT(unittest.TestCase):

    def setUp(self):
        self.engine = thermofun.ThermoEngine('pytests/Substances/Solids/Mo(s).json')
    
    def test_properties_substance(self):
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "Sulfur").gibbs_energy.val == pytest.approx(-818.39713, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "Sulfur").enthalpy.val == pytest.approx(577.23048, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "Sulfur").entropy.val == pytest.approx(33.65718, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(50+273.15, 1e5, "Sulfur").heat_capacity_cp.val == pytest.approx(23.41557, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e5, "Sulfur").gibbs_energy.val == pytest.approx(-2593.4817, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e5, "Sulfur").enthalpy.val == pytest.approx(2142.10468, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e5, "Sulfur").entropy.val == pytest.approx(38.098027, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(100+273.15, 1e5, "Sulfur").heat_capacity_cp.val == pytest.approx(25.940799, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(200+273.15, 1e5, "Sulfur").gibbs_energy.val == pytest.approx(-7096.92410, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(200+273.15, 1e5, "Sulfur").enthalpy.val == pytest.approx(6996.83946, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(200+273.15, 1e5, "Sulfur").entropy.val == pytest.approx(49.82448, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(200+273.15, 1e5, "Sulfur").heat_capacity_cp.val == pytest.approx(36.52631, 1e-5, 1e-14)