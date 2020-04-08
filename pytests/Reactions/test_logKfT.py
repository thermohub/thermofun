import thermofun as thermofun
import pytest as pytest
import unittest


class TestThermoEngine(unittest.TestCase):

    def setUp(self):
        self.engine = thermofun.ThermoEngine('pytests/Reactions/test-logKfT-thermofun.json')
    
    def test_properties_logKfT_reaction(self):
        assert self.engine.thermoPropertiesReaction(298.15, 1e5, "NaHCO3@").log_equilibrium_constant.val == pytest.approx(-0.1805389, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(298.15, 1e5, "NaHCO3@").reaction_heat_capacity_cp.val == pytest.approx(0.0, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(298.15, 1e5, "NaHCO3@").reaction_entropy.val == pytest.approx(35.4716, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(298.15, 1e5, "NaHCO3@").reaction_enthalpy.val == pytest.approx(11606.384, 1e-5, 1e-14)