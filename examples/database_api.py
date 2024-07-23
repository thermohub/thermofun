import thermofun as thermofun

database = thermofun.Database('test-database-thermofun.json')

print("Elements \n", database.getElementsList())
print("Substances \n", database.getSubstancesList())
print("Reactions \n", database.getReactionsList())

# Element
elment_O = database.getElement("O")
print(elment_O)
elment_O.setValence(-2)
print(elment_O)
print(database.getElement("O"))

elment_ref = database.element("H")
print(elment_ref)
elment_ref.setValence(1)
print(elment_ref)
print(database.getElement("H"))

elment_new = database.element("Co");  # get from defaults
print(elment_new)
print(database.getElement("Co"))

elment_empty = database.element("Ac"); #empty data
print(elment_empty)
print(database.getElement("Ac"))

# Substance

subst = database.substance("Quartz")
subst.setMolarMass(60.084300994873)
print(database.getSubstance("Quartz"))
subst_H2O = database.substance("H2O@")
subst_H2O.setMolarMass(18.015300750732)
print(database.getSubstance("H2O@"))

subst_Co = database.substance("Co+2")
print(subst_Co)

# Reaction
react = database.reaction("Gedrite-Mg")
react.setUpperP(1e5)
print(react)
#print(database)
