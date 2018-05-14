    #include "ThermoFun.h"
#include "thermofun/Common/ThermoScalar.hpp"
    using namespace std;
    using namespace ThermoFun;

    int main(int argc, char *argv[])
    {
      // Create the interface object using a database file in JSON
      Interface interface("Resources/aq17.json");

      // Optional: set the solvent symbol used for claulating properties of aqueous species
      interface.setSolventSymbol("H2O@");

      // Optional: change default units
      interface.setPropertiesUnits({"temperature", "pressure"},{"degC","bar"});

      // Optional: change default digits
      interface.setPropertiesDigits({"gibbs_energy","entropy", "volume", "enthalpy", "temperature", "pressure"}, {0, 1, 2, 0, 0, 0});

      // Retrieve the entropy of H2O
      double H2Oentropy = interface.thermoPropertiesSubstance( 400, 1000, "H2O@", "entropy").toDouble();
      Reaktoro_::ThermoScalar entro = interface.thermoPropertiesSubstance( 250, 1000, "H2O@", "entropy").toThermoScalar();
      Reaktoro_::ThermoScalar G1 = interface.thermoPropertiesSubstance( 25, 1000, "H2O@", "gibbs_energy").toThermoScalar();
      Reaktoro_::ThermoScalar G2 = interface.thermoPropertiesSubstance( 25, 1, "H2O@", "gibbs_energy").toThermoScalar();
      auto G= G1-G2;
      Reaktoro_::ThermoScalar V = interface.thermoPropertiesSubstance( 250, 1000, "H2O@", "volume").toThermoScalar();

      // Write results to a comma separate files for a list of T-P pairs, substances, and properties
      interface.thermoPropertiesSubstance({{25, 1},{40, 1},{70, 100},{90, 100},{100, 100}}, // list of T-P pairs
                                           {"Al+3", "OH-", "SiO2@"},                        // list of substance symbols
                                           {"gibbs_energy","entropy", "volume", "enthalpy"} // list of properties
                                          ).toCSV("results.csv");                           // output
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

