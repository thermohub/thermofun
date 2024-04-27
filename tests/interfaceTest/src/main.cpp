#include <iostream>
#include "ThermoFun.h"
#include "ChemicalFun/FormulaParser.h"
#include "GlobalVariables.h"
//#include "ThermoFun/Common/ThermoScalar.hpp"
    using namespace std;
    using namespace ThermoFun;

    int main()
    {
/*      // Create the interface object using a database file in JSON
      ThermoBatch batch("aq17-thermofun-new.json");

      auto elements1 = ThermoFun::ChemicalFormula::extractElements({"H4SiO2@"} );

      // Optional: set the solvent symbol used for claulating properties of aqueous species
      batch.setSolventSymbol("H2O@");

      // Optional: change default units
      batch.setPropertiesUnits({"temperature", "pressure"},{"degC","bar"});

      // Optional: change default digits
      batch.setPropertiesDigits({"gibbs_energy","entropy", "volume", "enthalpy", "temperature", "pressure"}, {0, 1, 2, 0, 0, 4});

      ThermoEngine en("cemdata18-thermofun.json");

      double P =1;
      auto pros5CA = en.thermoPropertiesSubstance(25, P, "5CA");

      // Retrieve the entropy of H2O
      double H2Oentropy = batch.thermoPropertiesSubstance( 300, 2000, "H2O@", "entropy").toDouble();
      double H2Ogentropy = batch.thermoPropertiesSubstance( 300, 2000, "H2O@", "entropy").toDouble();
      Reaktoro_::ThermoScalar entro = batch.thermoPropertiesSubstance( 250, 1000, "H2O@", "entropy").toThermoScalar();
      Reaktoro_::ThermoScalar G1 = batch.thermoPropertiesSubstance( 25, 1000, "H2O@", "gibbs_energy").toThermoScalar();
      Reaktoro_::ThermoScalar G2 = batch.thermoPropertiesSubstance( 25, 1, "H2O@", "gibbs_energy").toThermoScalar();
      auto G= G1-G2;
      Reaktoro_::ThermoScalar V = batch.thermoPropertiesSubstance( 250, 1000, "H2O@", "volume").toThermoScalar();

      // Write results to a comma separate files for a list of T-P pairs, substances, and properties
      batch.thermoPropertiesSubstance({{25, 0},{40, 0}}, // list of T-P pairs
                                           {"Al+3", "OH-", "SiO2@", "H2O@", "CO2@", "CO2"},                        // list of substance symbols
                                           {"gibbs_energy","entropy", "volume", "enthalpy"} // list of properties
                                          ).toCSV("results_after_2.csv");                           // output

      BatchPreferences pref;
      pref.loopTemperatureThenPressure = true;
      batch.setBatchPreferences(pref);
      batch.setTemperatureIncrement(0, 150, 5);
      batch.setPressureIncrement(0, 1000, 100);
      batch.thermoPropertiesSubstance({"Al+3", "OH-", "SiO2@", "H2O@", "CO2@", "CO2"},                        // list of substance symbols
                                      {"gibbs_energy","entropy", "volume", "enthalpy"} // list of properties
                                     ).toCSV("test_2.csv");                           // output

      batch.thermoPropertiesSubstance(  { 0.01,  25,  50,  75, 100, 125, 150, 175, 200, 225, 250, 275, 300, 325, 350,
                                        400, 410, 420, 430, 440, 450, 460, 470, 480, 490, 500, 510, 520, 530, 540, 550, 560,
                                        570, 580, 590, 600 },
                                        { 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000}, // list of T-P pairs
                                        {"Al+3", "OH-", "SiO2@", "H2O@", "CO2@", "CO2"},                        // list of substance symbols
                                        {"gibbs_energy","entropy", "volume", "enthalpy"} // list of properties
                                          ).toCSVPropertyGrid("grid.csv");    */                       // output


      // Test
      Database db("mines16-thermofun.json");
      std::cout << "Elements \n" << db.getElementsList() << endl;
      //std::cout << "Substances \n" << db.getSubstancesList() << endl;
      //std::cout << "Reactions \n" << db.getReactionsList() << endl;

      auto elment = db.getElement("O");
      std::cout << elment << endl;
      elment.setValence(ChemicalFun::DBElements::defaultValence("O"));
      std::cout << elment << endl;
      std::cout << db.getElement("O") << endl;

      auto& elment_ref = db.element("H");
      std::cout << elment_ref << endl;
      elment_ref.setValence(ChemicalFun::DBElements::defaultValence("H"));
      std::cout << elment_ref << endl;
      std::cout << db.getElement("H") << endl;

      auto& elment_new = db.element("Ir");  // get from defaults
      std::cout << elment_new << endl;

      auto& elment_empty = db.element("Ac");  // get from defaults
      std::cout << elment_empty << endl;

      auto sbst = db.getSubstance("O2");
      //std::cout << sbst << endl;

      auto react = db.getReaction("ZrCl+3");
      //std::cout << react << endl;

      //std::cout << db.getElements() << endl;
      //std::cout << db.getSubstances() << endl;
      //std::cout << db.getReactions() << endl;
      return 0;
      ThermoEngine engine("aq17-thermofun.json");

      //engine.appendData("append-thermofun.json");

      auto P = 1e5;

//      engine.thermoPropertiesReaction(298.15, P, "NaHCO3@");
//      auto tps = engine.thermoPropertiesSubstance(298.15, P, "NaHCO3@");

      //ThermoEngine engine2("slop98-thermofun.json");

      //auto P = 1e5;

     // auto tpr = engine2.thermoPropertiesReaction(298.15, P, "Cal = Ca+2 + CO3-2");

      //auto elements = engine2.parseSubstanceFormula("H2O@");

      Reaction reac;

      reac.fromEquation("H2O@ = H+ + OH-");


      //auto waterprop = engine2.thermoPropertiesReaction(298.15, P, "H2O@ = H+ + OH-");

      availableSubstanceTPMethods();

      std::string substLit = R"(
                           {
                             "Pst":100000,
                             "TPMethods":[
                               {
                                 "eos_hkf_coeffs":{
                                   "names":[
                                     "a1",
                                     "a2",
                                     "a3",
                                     "a4",
                                     "c1",
                                     "c2",
                                     "wref"
                                   ],
                                   "units":[
                                     "cal/mol/bar",
                                     "cal/mol",
                                     "cal*K/mol/bar",
                                     "cal*K/mol",
                                     "cal/K/mol",
                                     "cal*K/mol",
                                     "cal/mol"
                                   ],
                                   "values":[
                                     -1.2252e-1,
                                     -8.9356e2,
                                     5.3191,
                                     -2.4095e4,
                                     15.2013,
                                     -4.6234e4,
                                     1.4769e5
                                   ]
                                 },
                                 "method":{
                                   "3":"solute_hkf88_reaktoro"
                                 }
                               }
                             ],
                             "Tst":298.15,
                             "aggregate_state":{
                               "4":"AS_AQUEOUS"
                             },
                             "class_":{
                               "2":"SC_AQSOLUTE"
                             },
                             "datasources":[
                               "Shock, 1997a"
                             ],
                             "formula":"Co+2",
                             "formula_charge":2,
                             "name":"Co+2",
                             "reaction":"Co+2",
                             "sm_enthalpy":{
                               "values":[
                                 -13900
                               ],
                               "units":[
                                 "J/mol"
                               ]
                             },
                             "sm_entropy_abs":{
                               "values":[
                                 -27.000
                               ]
                             },
                             "sm_gibbs_energy":{
                               "values":[
                                 -13000
                               ],
                               "units":[
                                 "J/mol"
                               ]
                             },
                             "symbol":"Co+2_string_lit"
                           }
           )";

      std::string reacLit = R"(
                            {
                              "TPMethods": [
                                {
                                  "dr_ryzhenko_coeffs": {
                                    "values": [
                            2.426,
                            2.899,
                            -662.33
                                    ]
                                  },
                                  "method": {
                                    "10": "solute_eos_ryzhenko_gems"
                                  }
                                }
                              ],
                              "datasources": "Migdisov et al., 2011",
                              "equation": "Co+2 + 4Cl- = CoCl4-2R",
                              "symbol": "CoCl4-2R"
                            }
                            )";

      Substance subst(substLit);
      availableReactionTPMethods();
      std::cout << availablePropertiesReaction()<< endl;
      std::cout << availablePropertiesSubstance() << endl;

      db.addSubstance(substLit);
      db.addReaction(reacLit);

      std::string reacLit2 = R"(
                             {
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
                             }
                             )";
      std::string substLit2 = R"(
                              {
                                  "symbol": "LaSO4+_mi",
                                  "formula": "LaSO4+",
                                  "reaction": "LaSO4+_mi",
                                  "aggregate_state": {
                                      "4": "AS_AQUEOUS"
                                  },
                                  "class_": {
                                      "3": "SC_AQSOLVENT"
                                  },
                                  "Tst": 298.15,
                                  "Pst": 100000
                              }
                             )";

      P = 800e5;

      db.addReaction(reacLit2);
      db.addSubstance(substLit2);

      ThermoEngine engine3(db);
      auto res5 = engine3.thermoPropertiesSubstance(473, P, "Co+2_string_lit");
      auto res6 = engine3.thermoPropertiesReaction(673, P, "CoCl4-2R");

      auto res7 = engine3.thermoPropertiesSubstance(473, P, "LaSO4+_mi");
      auto res8 = engine3.thermoPropertiesSubstance(473, P, "La(SO4)+");

//      batch.thermoPropertiesReaction(25, 1, "H2O@ = H+ + OH-", "logKr").toCSV("test_reac.cvs");
//      batch.thermoPropertiesReaction(25, 1, "Al+3 + 4H2O@ + 0Ca+2 = 1Al(OH)4- + 4 \nH+", "logKr").toCSV("test_reac2.cvs");

      return 0;
    }



//    int main(int argc, char *argv[])
//    {
//      DatabaseClient dbc; // connecting to the database local/remote

//      // retrieving records from supcrt92 dataset
//      auto records = dbc.recordsFromThermoDataSet("supcrt92");

//      // loading ThermoFun database object
//      Database db = databaseFromRecordList(dbc, records);

//      // creating the Thermo object
//      Thermo th(db);

//      double T = 398.15; double P = 1e7; //100 degC and 100 bar
//      auto propAl = th.thermoPropertiesSubstance(T, P, "Al+3");

//      // extracting values from results for the Gibbs energy
//      double G0       = propAl.gibbs_energy.val; // value
//      double G0_dT    = propAl.gibbs_energy.ddt; // derivative with T = -S0
//      double G0_error = propAl.gibbs_energy.err; // propagated error

//      return 0;
//    }

