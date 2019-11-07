    #include "ThermoFun.h"
    using namespace std;
    using namespace ThermoFun;

    int main()
    {
      // Create the interface object using a database file in JSON
      ThermoEngine engine("aq17-gem-lma-thermofun.json");

      std::vector<double> Ts =  {0.01,  25,  50,  75, 100, 125, 150, 175, 200, 225, 250, 275, 300, 325, 350, /*(L-V)*/
              400, 410, 420, 430, 440, 450, 460, 470, 480, 490, 500, 510, 520, 530, 540, 550, 560,
              570, 580, 590, 600};

      std::vector<double> Ps =  {1.000, 1.0133, 2.321, 4.758, 8.919, 15.537, 25.479, 39.737, 59.432, 85.839,
              120.458, 165.212, /*(L-V saturation)*/ 200, 250, 300, 350, 400, 450, 500, 550, 600, 650,
              700, 750, 800, 850, 900, 950, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000};
      vtpr results;

      engine.setSolventSymbol("H2O@");
  //    auto reactions = engine.database().mapReactions();

      std::map<double, double> psats;

 //     for (auto reaction : reactions)
      for (auto pbar : Ps)
          for (auto tC : Ts)
          {
              auto pPa = pbar*1e5; // to Pa
              auto tK = tC+273.15; // to K
              ThermoPropertiesReaction tpr;
              tpr.log_equilibrium_constant.val = 99999.9989;
              PropertiesSolvent prop_solvent;
              try {
                  if (!(tC>350. && pbar <200.))
                      prop_solvent = engine.propertiesSolvent(tK,pPa,"H2O@");
                  if (!(tC>350.))
                  {
                      double psat = 0;
                      if (psats.find(tC) == psats.end())
                      {
                          engine.propertiesSolvent(tK,psat,"H2O@");
                          psats[tC]=psat;
                      }

                      if (pPa < psats[tC])
                          prop_solvent.density = 0.0;
                  }
                  if (prop_solvent.density.val > 350.0)
                      tpr = engine.thermoPropertiesReactionFromReactants(tK,pPa, "Calcite");
              } catch (std::runtime_error e) {
                  std::cout << e.what() << std::endl;
                  tpr.log_equilibrium_constant.val = 99999.9989;
              }
              results.push_back(tpr);
          }

      ThermoBatch batch(engine);

      BatchPreferences prefs;
      prefs.TthenPincrements = false;
      prefs.calcReactFromSubst = true;

      batch.setBatchPreferences(prefs);

      batch.setSolventSymbol("H2O@");

      batch.setPropertiesUnits({"temperature", "pressure"},{"degC","bar"});

      batch.setPropertiesDigits({"logKr", "temperature", "pressure"}, {3, 0, 3});

      batch.thermoPropertiesReaction(Ts, Ps, {"Calcite"}, {"logKr"}, results).toCSVPropertyGrid("test_grid.csv");

      return 0;
    }
