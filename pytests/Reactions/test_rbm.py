import thermofun as thermofun
import pytest as pytest
import unittest


class TestThermoEngine(unittest.TestCase):

    def setUp(self):
        self.engine = thermofun.ThermoEngine('pytests/Reactions/Co_Cu_aq-thermofun.json')
    
    def test_properties_Ryzhenko_Bryzgalin_reaction(self):
        assert self.engine.thermoPropertiesReaction(298.15, 0, "CoCl4-2").log_equilibrium_constant.val == pytest.approx(-2.42632584, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(673.15, 800e5, "CoCl4-2").log_equilibrium_constant.val == pytest.approx(-10.327743, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(573.15, 0, "CoCl4-2").reaction_entropy.val == pytest.approx(-675.96968, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(473.15, 0, "CoCl4-2").reaction_heat_capacity_cp.val == pytest.approx(-912.68, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(373.15, 0, "CoCl4-2").reaction_volume.val == pytest.approx(-2.828973221, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(673.15, 800e5, "CoCl4-2").gibbs_energy.val == pytest.approx(-654891.056, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(873.15, 3000e5, "CoCl4-2").entropy.val == pytest.approx(202.67078, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(373.15, 5000e5, "CoCl4-2").heat_capacity_cp.val == pytest.approx(163.0106, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(673.15, 800e5, "CoCl4-2").volume.val == pytest.approx(-43.8768625, 1e-5, 1e-14)
        #assert self.engine.thermoPropertiesReaction(298.15, 1e5, "NaHCO3@").reaction_heat_capacity_cp.val == pytest.approx(0.0, 1e-5, 1e-14)
        #assert self.engine.thermoPropertiesReaction(298.15, 1e5, "NaHCO3@").reaction_entropy.val == pytest.approx(35.4716, 1e-5, 1e-14)
        #assert self.engine.thermoPropertiesReaction(298.15, 1e5, "NaHCO3@").reaction_enthalpy.val == pytest.approx(11606.384, 1e-5, 1e-14)