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

    def test_reference_element(self):
        elment_O = self.database.getElement("O")
        elment_O.setValence(-3)
        assert self.database.getElement("O").valence() != -3
        assert elment_O.valence() == -3
        elment_H = self.database.element("H")
        elment_H.setValence(1)
        assert self.database.getElement("H").valence() == 1
        # new
        elment_new = self.database.element("Co");  # get from defaults
        assert self.database.numberOfElements() == 5
        assert self.database.element("Co").name() == 'Co'
        assert self.database.element("Co").symbol() == 'Co'
        assert self.database.element("Co").molarMass() == 58.9332008361816
        assert self.database.element("Co").entropy() == 30.0400009155273
        assert self.database.element("Co").heatCapacity() == 24.8099994659424
        assert self.database.element("Co").volume() == 6.61999988555908
        assert self.database.element("Co").valence() == 2
        assert self.database.element("Co").class_() == 0
        assert self.database.element("Co").isotopeMass() == 0
        assert self.database.element("Co").number() == 27

    def test_reference_substance(self):
        subst = self.database.substance("Quartz")
        subst.setMolarMass(60.084300994873)
        assert self.database.getSubstance("Quartz").molarMass() == 60.084300994873
        subst2 = self.database.substance("H2O@")
        subst2.setMolarMass(18.015300750732)
        assert self.database.getSubstance("H2O@").molarMass() == 18.015300750732
        # new empty
        subst_new = self.database.substance("Co+2");  # zero values
        assert self.database.numberOfSubstances() == 6
        assert self.database.getSubstancesList() ==  ['Al(OH)2+', 'Al(OH)3@', 'Al(OH)4-', 'Co+2', 'H2O@', 'Quartz']
        assert self.database.getSubstance("Co+2").name() == 'Co+2'
        assert self.database.getSubstance("Co+2").symbol() == 'Co+2'
        assert self.database.getSubstance("Co+2").molarMass() == 0

    def test_reference_reaction(self):
        react = self.database.reaction("Gedrite-Mg")
        react.setUpperP(1e5)
        assert self.database.getReaction("Gedrite-Mg").upperP() == 1e5
        # new empty
        react_new = self.database.reaction("Co+2");  # zero values
        assert self.database.numberOfReactions() == 5
        assert self.database.getReactionsList() ==  ['Co+2', 'Gedrite-Mg', 'Meionite-Ca', 'Pargasite-Mg', 'Tschermakite-Mg']
        assert self.database.getReaction("Co+2").name() == 'Co+2'
        assert self.database.getReaction("Co+2").symbol() == 'Co+2'
        assert self.database.getReaction("Co+2").upperP() == 0

    def test_formula_parser(self):
        self.database.appendData('pytests/Fe-O_system.json')
        assert self.database.parseSubstanceFormula('FeFe|3|2O4')
        elements = self.database.parseSubstanceFormula('FeFe|3|2O4')
        assert len(elements) == 2
        assert elements[list(elements.keys())[0]] == 3
        assert elements[list(elements.keys())[1]] == 4
        elements = self.database.parseSubstanceFormula('C|-4|H4@')
        assert elements[list(elements.keys())[0]] == 1
        assert elements[list(elements.keys())[1]] == 4
        assert elements[list(elements.keys())[2]] == 0

    def test_add_json_reaction_substance(self):
        reacLit = R'''
{
  "TPMethods":[
    {
      "dr_ryzhenko_coeffs":{
        "values":[
          2.426,
          2.899,
          -662.33
        ]
      },
      "method":{
        "10":"solute_eos_ryzhenko_gems"
      }
    }
  ],
  "datasources":"Migdisov et al., 2011",
  "equation":"Co+2 + 4Cl- = CoCl4-2R",
  "symbol":"CoCl4-2R"
}
'''
        substLit = R'''
{
  "Pst": 100000,
  "TPMethods": [
    {
      "eos_hkf_coeffs": {
        "names": [
          "a1",
          "a2",
          "a3",
          "a4",
          "c1",
          "c2",
          "wref"
        ],
        "units": [
          "cal/mol/bar",
          "cal/mol",
          "cal*K/mol/bar",
          "cal*K/mol",
          "cal/K/mol",
          "cal*K/mol",
          "cal/mol"
        ],
        "values": [
          -0.12252,
          -893.56,
          5.3191,
          -24095,
          15.2013,
          -46234,
          147690
        ]
      },
      "method": {
        "3": "solute_hkf88_reaktoro"
      }
    }
  ],
  "Tst": 298.15,
  "aggregate_state": {
    "4": "AS_AQUEOUS"
  },
  "class_": {
    "2": "SC_AQSOLUTE"
  },
  "datasources": [
    "Shock, 1997a"
  ],
  "formula": "Co+2",
  "formula_charge": 2,
  "name": "Co+2",
  "reaction": "Co+2",
  "sm_enthalpy": {
    "values": [
      -13900
    ],
    "units": [
      "J/mol"
    ]
  },
  "sm_entropy_abs": {
    "values": [
      -27
    ]
  },
  "sm_gibbs_energy": {
    "values": [
      -13000
    ],
    "units": [
      "J/mol"
    ]
  },
  "symbol": "Co+2_string_lit"
}
'''
        self.database.addReaction(reacLit)
        self.database.addSubstance(substLit)
        subst = thermofun.Substance(substLit)
        assert self.database.getReaction("CoCl4-2R").equation() == "Co+2 + 4Cl- = CoCl4-2R"
        assert self.database.getSubstance("Co+2_string_lit").thermoParameters().HKF_parameters[0] == -0.12252 # "cal/(mol*bar)"
        assert subst.thermoReferenceProperties().gibbs_energy.val == -13000.0 # J/mol

