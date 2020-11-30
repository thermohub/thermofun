import thermofun as thermofun
import pytest as pytest
import unittest


class TestDatabase(unittest.TestCase):

    def setUp(self):
        self.database = thermofun.Database('pytests/test-database-thermofun.json')
    
    def test_parsing_database(self):
        assert self.database.containsElement("C")
        assert self.database.containsSubstance("H2O@")
        assert self.database.containsReaction("Meionite-Ca")
        assert self.database.numberOfElements() == self.database.numberOfReactions() == 4
    
    def test_parsing_substance(self):
        assert self.database.getSubstance("H2O@").name() == "Water HGK"
        assert self.database.getSubstance("H2O@").symbol() == "H2O@"
        assert self.database.getSubstance("H2O@").formula() == "H2O@"
        assert self.database.getSubstance("H2O@").referenceT() == 298.15
        assert self.database.getSubstance("H2O@").referenceP() == 1e5 
        assert self.database.getSubstance("Al(OH)2+").thermoReferenceProperties().gibbs_energy.val == -898292 # J/mol
        assert self.database.getSubstance("Al(OH)2+").thermoReferenceProperties().enthalpy.val == -995581 # J/mol
        assert self.database.getSubstance("Al(OH)2+").thermoReferenceProperties().entropy.val == -27.530000686646 # J/(mol*K)
        assert self.database.getSubstance("Al(OH)2+").thermoReferenceProperties().heat_capacity_cp.val == 40.865230560303 # J/(mol*K)
        assert self.database.getSubstance("Al(OH)2+").thermoReferenceProperties().volume.val == 0.38507527112961 # J/bar
        assert self.database.getSubstance("Al(OH)2+").thermoParameters().HKF_parameters[0] == 0.24940000474453003 # "cal/(mol*bar)"
        assert self.database.getSubstance("Al(OH)2+").thermoParameters().HKF_parameters[1] == -169.08999633789
        assert self.database.getSubstance("Al(OH)2+").thermoParameters().HKF_parameters[2] == 6.4145998954773
        assert self.database.getSubstance("Al(OH)2+").thermoParameters().HKF_parameters[3] == -27091
        assert self.database.getSubstance("Al(OH)2+").thermoParameters().HKF_parameters[4] == 16.743900299072
        assert self.database.getSubstance("Al(OH)2+").thermoParameters().HKF_parameters[5] == -10465
        assert self.database.getSubstance("Al(OH)2+").thermoParameters().HKF_parameters[6] == 53240
        assert self.database.getSubstance("Quartz").thermoParameters().m_landau_phase_trans_props[2] == 0.11879999935626999
    def test_parsing_reaction(self):
        assert self.database.getReaction("Meionite-Ca").thermoReferenceProperties().log_equilibrium_constant.val == 80.875017 # "1"
        assert self.database.getReaction("Meionite-Ca").thermoReferenceProperties().reaction_heat_capacity_cp.val == -2598.4849980000004 # J/(mol*K)

    def test_append_data(self):
        self.database.appendData('pytests/Substances/Solute/test-hkf-thermofun.json')
        assert self.database.getSubstance("SiO2@").name() == "SiO2 (aq)"
        assert self.database.getSubstance("SiO2@").thermoReferenceProperties().gibbs_energy.val == -833411
        assert self.database.numberOfSubstances() == 7
        assert self.database.numberOfElements() == 4

    def test_append_element(self):
        self.database.appendData('pytests/test-element-Co-thermofun.json')
        assert self.database.parseSubstanceFormula('Co+2')
        assert self.database.numberOfElements() == 5
