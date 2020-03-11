import thermofun as thermofun
import pytest as pytest
import unittest


class TestThermoEngine(unittest.TestCase):

    def setUp(self):
        self.engine = thermofun.ThermoEngine('pytests/test-thermoengine-thermofun.json')
    
    def test_properties_reaction_from_equation(self):
        assert self.engine.thermoPropertiesReaction(298.15, 1e5, "Cal = Ca+2 + CO3-2").log_equilibrium_constant.val == pytest.approx(-8.48014, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(298.15, 1e5, "Al+3 + 4 H2O@ + 0Ca+2= 1Al(OH)4- + 4 \n H+").log_equilibrium_constant.val == pytest.approx(-22.3085, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(423.15, 0, "Cal = Ca+2 + CO3-2").log_equilibrium_constant.val == pytest.approx(-10.10169, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(873.15, 3000e5, "Cal = Ca+2 + CO3-2").reaction_heat_capacity_cp.val == pytest.approx(-718.75763, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(348.15, 1e5, "Cal = Ca+2 + CO3-2").reaction_gibbs_energy.ddt == pytest.approx(
              -self.engine.thermoPropertiesReaction(348.15, 1e5, "Cal = Ca+2 + CO3-2").reaction_entropy.val, 1e-5, 1e-14)
