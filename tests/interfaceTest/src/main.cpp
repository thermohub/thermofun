    #include "ThermoFun.h"
#include "ThermoFun/Common/formuladata.h"
//#include "ThermoFun/Common/ThermoScalar.hpp"
    using namespace std;
    using namespace ThermoFun;

    int main()
    {
      // Create the interface object using a database file in JSON
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
                                          ).toCSVPropertyGrid("grid.csv");                           // output


      // Test
      ThermoEngine engine("aq17_.json");

      auto elements = engine.parseSubstanceFormula("H2O@");

      Reaction reac;

      reac.fromEquation("H2O@ = H+ + OH-");

      P = 1e5;
      auto waterprop = engine.thermoPropertiesReaction(298.15, P, "H2O@ = H+ + OH-");

      batch.thermoPropertiesReaction(25, 1, "H2O@ = H+ + OH-", "logKr").toCSV("test_reac.cvs");
      batch.thermoPropertiesReaction(25, 1, "Al+3 + 4H2O@ + 0Ca+2 = 1Al(OH)4- + 4 \nH+", "logKr").toCSV("test_reac2.cvs");

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

