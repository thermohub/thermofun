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

    def test_append_data(self):
        self.database.appendData('pytests/Substances/Solute/test-hkf-thermofun.json')
        assert self.database.getSubstance("SiO2@").name() == "SiO2 (aq)"
        assert self.database.getSubstance("SiO2@").thermoReferenceProperties().gibbs_energy.val == -833411
        assert self.database.numberOfSubstances() == 6