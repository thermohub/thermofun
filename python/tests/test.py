import PyThermoFun

properties = PyThermoFun.ThermoPropertiesSubstance

engine = PyThermoFun.ThermoEngine("aq17.json")

prop = engine.thermoPropertiesSubstance(373.15, 100000000, "H2O@")

print(prop.gibbs_energy.val)
print(prop.gibbs_energy.ddt)
print(prop.entropy.val)
print(prop.gibbs_energy.ddp)
print(prop.gibbs_energy.err)
# print(prop.gibbs_energy.sta)