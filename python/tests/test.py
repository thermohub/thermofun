import PyThermoFun

properties = PyThermoFun.ThermoPropertiesSubstance

engine = PyThermoFun.ThermoEngine("Resources/aq17new-format.json")

prop = engine.thermoPropertiesSubstance(373.15, 100000000, "H2O@")

print(prop.gibbs_energy.val)
print(prop.gibbs_energy.ddt)
print(prop.entropy.val)
print(prop.gibbs_energy.ddp)
print(prop.gibbs_energy.err)
print(prop.gibbs_energy.sta)

# Create the engine object using a database file in JSON
batch = PyThermoFun.ThermoBatch("Resources/aq17new-format.json")

# Optional: set the solvent symbol used for claulating properties of aqueous species
batch.setSolventSymbol("H2O@")

# Optional: change default units
batch.setPropertiesUnits(["temperature", "pressure"],["degC","bar"])

# Optional: change default digits
batch.setPropertiesDigits(["gibbs_energy","entropy", "volume", "enthalpy", "temperature", "pressure"], [0, 1, 2, 0, 0, 0])

H2Oentropy = batch.thermoPropertiesSubstance( 300, 2000, "H2O@", "entropy").toDouble()
print(H2Oentropy)

entro = batch.thermoPropertiesSubstance( 250, 1000, "H2O@", "entropy").toThermoScalar()
G1 = batch.thermoPropertiesSubstance( 25, 1000, "H2O@", "gibbs_energy").toThermoScalar()
G2 = batch.thermoPropertiesSubstance( 25, 1, "H2O@", "gibbs_energy").toThermoScalar()
# G= G1-G2; - no overloaded opreations for python - for now
V = batch.thermoPropertiesSubstance( 250, 1000, "H2O@", "volume").toThermoScalar()

# Write results to a comma separate files for a list of T-P pairs, substances, and properties
batch.thermoPropertiesSubstance( [[25, 1],[40, 1],[70, 100],[90, 100],[100, 100]],  # // list of T-P pairs
                                 ["Al+3", "OH-", "SiO2@"],                          # // list of substance symbols
                                 ["gibbs_energy","entropy", "volume", "enthalpy"]   # // list of properties
                               ).toCSV("results.csv")                               # // output

PyThermoFun.setDatabaseConnectionFilePath("Resources/ThermoFunGUI-config.json")

# Initialize a database client object
dbc = PyThermoFun.DatabaseClient()

# Retrieve list of records given a ThermoDataSet symbol
records = dbc.recordsFromThermoDataSet("PSINagra07") 

# Create a ThermoFun database using the records list
db = databaseFromRecordList(dbc, records)

# Initialize an interface object using the database
batch2 = PyThermoFun.ThermoBatch(db)

# Optional: set the solvent symbol used for calculating properties of aqueous species
batch2.setSolventSymbol("H2O@")

# Optional set calculation and output preferences
#   ThermoFun::OutputSettings op;
#   op.isFixed = true;
#   op.outSolventProp       = true;
#   op.calcReactFromSubst   = false;
#   op.calcSubstFromReact   = false;
#   interface.setOutputSettings(op);

# Optional set units and significant digits
batch.setPropertiesUnits({"temperature", "pressure"},{"degC","bar"})

batch.setPropertiesDigits({"reaction_gibbs_energy","reaction_entropy", "reaction_volume",
                            "reaction_enthalpy","logKr", "temperature", "pressure"}, {0, 4, 4, 4, 4, 0, 0})

# Do calculations and write output
batch.thermoPropertiesReaction({{25,1}}, {"AmSO4+", "MgSiO3@"}, {"reaction_gibbs_energy", "reaction_entropy",
                                "reaction_volume", "reaction_enthalpy", "logKr"}).toCSV("results.csv")

batch.thermoPropertiesReaction({0,20,50,75},{0,0,0,0},{"AmSO4+", "MgSiO3@"}, {"reaction_gibbs_energy", "reaction_entropy",
                                "reaction_volume", "reaction_enthalpy", "logKr"}).toCSV("results.csv")

