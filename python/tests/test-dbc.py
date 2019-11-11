import PyThermoFun
import PyThermoHubClient

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

PyThermoHubClient.setDatabaseConnectionFilePath("Resources/fun-hubclient-config.json")

print("\n# Initialize a database client object\n")
dbc = PyThermoHubClient.DatabaseClient()

print("\n# Retrieve list of records given a ThermoDataSet symbol\n")
records = dbc.recordsFromThermoDataSet("Cemdata18") 

print("\n# Create a ThermoFun database using the records list\n")
db = PyThermoHubClient.databaseFromRecordList(dbc, records)

print("\n# Initialize an interface object using the database\n")
batch2 = PyThermoFun.ThermoBatch(db)

print("\n# Optional: set the solvent symbol used for calculating properties of aqueous species\n")
batch2.setSolventSymbol("H2O@")

print("\n# Optional set calculation and output preferences\n")
op = PyThermoFun.BatchPreferences()
op.isFixed = True
op.outputSolventProperties       = True
op.reactionPropertiesFromReactants   = False
op.substancePropertiesFromReaction   = False
batch2.setBatchPreferences(op)

print("\n# Optional set units and significant digits\n")
batch2.setPropertiesUnits(["temperature", "pressure"],["degC","bar"])

batch2.setPropertiesDigits(["gibbs_energy","entropy", "volume",
                            "enthalpy","logKr", "temperature", "pressure"], [0, 4, 4, 4, 4, 0, 0])

print("\n# Do calculations and write output\n")
batch2.thermoPropertiesSubstance([[25,1]], ["Na(CO3)-", "Mg+2"], ["gibbs_energy", "entropy",
                                "volume", "enthalpy"]).toCSV("results_dbc.csv")