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
