import thermofun as thermofun
import pytest as pytest
import unittest


class TestThermoEngine(unittest.TestCase):

    def setUp(self):
        self.engine = thermofun.ThermoEngine('pytests/test-thermoengine-thermofun.json')
        self.engine2 = thermofun.ThermoEngine('pytests/test-aq17-gem-lma-thermofun.json')
        self.Psat = 0
    
    def test_properties_substance(self):
        assert self.engine.thermoPropertiesSubstance(873.15, 5000e5, "Quartz").gibbs_energy.val == pytest.approx(-889055.513, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(873.15, 5000e5, "Quartz").volume.val == pytest.approx(2.3183336, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(873.15, 5000e5, "Quartz").entropy.val == pytest.approx(105.5428, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(873.15, 5000e5, "Quartz").heat_capacity_cp.val == pytest.approx(75.14837, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(873.15, 5000e5, "CO2@").gibbs_energy.val == pytest.approx(-485154.6363, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(873.15, 5000e5, "CO2@").volume.val == pytest.approx(3.43432, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(873.15, 5000e5, "CO2@").entropy.val == pytest.approx(239.07241, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(873.15, 5000e5, "CO2@").heat_capacity_cp.val == pytest.approx(71.9893685, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesSubstance(423.15, 4.8E5, "Ca+2").gibbs_energy.val == pytest.approx(-544992.793697, 1e-5, 1e-14)
        print(f'Psat {self.Psat}')
    
    def test_properties_reaction_from_equation(self):
        assert self.engine.thermoPropertiesReaction(298.15, 1e5, "Cal = Ca+2 + CO3-2").log_equilibrium_constant.val == pytest.approx(-8.48014, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(298.15, 1e5, "Al+3 + 4 H2O@ + 0Ca+2= 1Al(OH)4- + 4 \n H+").log_equilibrium_constant.val == pytest.approx(-22.3085, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(423.15, 4.8E5, "Cal = Ca+2 + CO3-2").log_equilibrium_constant.val == pytest.approx(-10.1016559, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(873.15, 3000e5, "Cal = Ca+2 + CO3-2").reaction_heat_capacity_cp.val == pytest.approx(-718.75763, 1e-5, 1e-14)
        assert self.engine.thermoPropertiesReaction(348.15, 1e5, "Cal = Ca+2 + CO3-2").reaction_gibbs_energy.ddt == pytest.approx(
              -self.engine.thermoPropertiesReaction(348.15, 1e5, "Cal = Ca+2 + CO3-2").reaction_entropy.val, 1e-5, 1e-14)

    def test_properties_reaction(self):
        assert self.engine2.thermoPropertiesReaction(298.15, 1e5, "Meionite-Ca").log_equilibrium_constant.val == pytest.approx(80.873916, 1e-5, 1e-14)
        assert self.engine2.thermoPropertiesReaction(423.15, 4.8E5, "Meionite-Ca").log_equilibrium_constant.val == pytest.approx(28.244026, 1e-5, 1e-14)
        assert self.engine2.thermoPropertiesReaction(298.15, 0, "Meionite-Ca + 25H+ = HCO3- + 6H4SiO4@ + 4Ca+2 + 6Al+3").log_equilibrium_constant.val == pytest.approx(80.873916, 1e-5, 1e-14)
        assert self.engine2.thermoPropertiesReaction(873.15, 5000e5, "Meionite-Ca + 25H+ = HCO3- + 6H4SiO4@ + 4Ca+2 + 6Al+3").reaction_heat_capacity_cp.val == pytest.approx(-1072.969936, 1e-5, 1e-14)
        assert self.engine2.thermoPropertiesReaction(873.15, 5000e5, "Meionite-Ca + 25H+ = HCO3- + 6H4SiO4@ + 4Ca+2 + 6Al+3").reaction_entropy.val == pytest.approx(-1704.130129, 1e-5, 1e-14)
        assert self.engine2.thermoPropertiesReaction(873.15, 5000e5, "Meionite-Ca + 25H+ = HCO3- + 6H4SiO4@ + 4Ca+2 + 6Al+3").reaction_volume.val == pytest.approx(-60.00281, 1e-5, 1e-14)
        assert self.engine2.thermoPropertiesReaction(873.15, 5000e5, "Meionite-Ca + 25H+ = HCO3- + 6H4SiO4@ + 4Ca+2 + 6Al+3").reaction_enthalpy.val == pytest.approx(-1139970.03233, 1e-5, 1e-14)

    def test_properties_substance_from_reaction(self):
        reacLit2 = '''{
                               "TPMethods":[
                                 {
                                   "dr_ryzhenko_coeffs":{
                                     "values":[
                                       3.61,
                                       1.563,
                                       -193.7
                                     ]
                                   },
                                   "method":{
                                     "10":"solute_eos_ryzhenko_gems"
                                   }
                                 }
                               ],
                               "datasources":"Migdisov et al., 2016",
                               "equation":"LaSO4+_mi = La+3 + SO4-2",
                               "symbol":"LaSO4+_mi"
                             }'''
        substLit2 = '''{
                                  "symbol": "LaSO4+_mi",
                                  "formula": "LaSO4+",
                                  "reaction": "LaSO4+_mi"
                              }'''
        db = thermofun.Database('pytests/mines16-sub-thermofun.json')
        db.addReaction(reacLit2)
        db.addSubstance(substLit2)
        engine3 = thermofun.ThermoEngine(db)
        assert engine3.thermoPropertiesSubstance(473.15, 0, "LaSO4+_mi").gibbs_energy.val == pytest.approx(-1426099.716850813, 1e-5, 1e-14)
        assert engine3.thermoPropertiesSubstance(473.15, 0, "LaSO4+_mi").volume.val == pytest.approx(-1.883700250726727, 1e-5, 1e-14)
        assert engine3.thermoPropertiesSubstance(473.15, 0, "LaSO4+_mi").entropy.val == pytest.approx(-168.47492835119232, 1e-5, 1e-14)
