    #include "ThermoFun.h"
#include "thermofun/Common/ThermoScalar.hpp"
    using namespace std;
    using namespace ThermoFun;

    int main()
    {
      // Create the interface object using a database file in JSON
      ThermoBatch batch("aq17_.json");

      // Optional: set the solvent symbol used for claulating properties of aqueous species
      batch.setSolventSymbol("H2O@");

      // Optional: change default units
      batch.setPropertiesUnits({"temperature", "pressure"},{"degC","bar"});

      // Optional: change default digits
      batch.setPropertiesDigits({"gibbs_energy","entropy", "volume", "enthalpy", "temperature", "pressure"}, {0, 1, 2, 0, 0, 0});

      // Retrieve the entropy of H2O
      double H2Oentropy = batch.thermoPropertiesSubstance( 300, 2000, "H2O@", "entropy").toDouble();
      double H2Ogentropy = batch.thermoPropertiesSubstance( 300, 2000, "H2O@", "entropy").toDouble();
      Reaktoro_::ThermoScalar entro = batch.thermoPropertiesSubstance( 250, 1000, "H2O@", "entropy").toThermoScalar();
      Reaktoro_::ThermoScalar G1 = batch.thermoPropertiesSubstance( 25, 1000, "H2O@", "gibbs_energy").toThermoScalar();
      Reaktoro_::ThermoScalar G2 = batch.thermoPropertiesSubstance( 25, 1, "H2O@", "gibbs_energy").toThermoScalar();
      auto G= G1-G2;
      Reaktoro_::ThermoScalar V = batch.thermoPropertiesSubstance( 250, 1000, "H2O@", "volume").toThermoScalar();

      // Write results to a comma separate files for a list of T-P pairs, substances, and properties
      batch.thermoPropertiesSubstance({{25, 1},{40, 1},{70, 100},{90, 100},{100, 100}, {600, 4100}}, // list of T-P pairs
                                           {"Al+3", "OH-", "SiO2@", "H2O@", "CO2@", "CO2"},                        // list of substance symbols
                                           {"gibbs_energy","entropy", "volume", "enthalpy"} // list of properties
                                          ).toCSV("results_after.csv");                           // output


      // Test
      ThermoEngine engine("aq17.json");

      auto elements = engine.parseSubstanceFormula("H2O@");

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

