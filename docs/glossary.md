# Glossary

Generated. Do not edit by hand: run `python -m thermofun.docs generate docs/reference`. The words are in `docs/reference/vocabulary.yaml`.

Each word has one meaning in these pages. The source is the standard that defines the word (NEA TDB guidelines, IUPAC Green Book GB4, ISO 80000-5, GUM, CODATA), or ThermoFun for a word of the software.

| Word | Meaning | Term of the standard | Source |
|---|---|---|---|
| Base method | A method that gives all the properties of a record. A record has one. |  | ThermoFun |
| Batch calculation | Many records at many conditions at once, from a batch setup. `tf.batch(setup)` gives the results, one result table for each entry of `results`. |  | ThermoFun |
| Batch setup | What a batch calculation does, in four blocks: `database` and `engine`, `conditions`, `results`, `output`. A dict, a YAML file or a `BatchSetup`. |  | ThermoFun |
| Batch template | A file with some blocks of a batch setup. A **fragment** has one block only, such as the conditions. `blank` has a placeholder `<...>` for each value to give. |  | ThermoFun |
| Canonical convention | The convention that the engine calculates in. `benson_helgeson`, molality, 1 bar, `hydrogen_ion`, and the anchor of water of the dataset. |  | ThermoFun |
| Catalog | The list of all methods of the library, with their parameters, model parameters, terms and variables. `tf.methods.catalog()` gives it. |  | ThermoFun |
| Coefficient | The multiplier of a term in a formula (the Cp coefficients a, b, c), and a part of the name of some physical quantities (activity coefficient). Not a value of a model (that is a model parameter). |  | NEA TDB-4 |
| Constant | A physical constant, such as R, F or NA. The engine option `physical_constants` chooses the set. | fundamental physical constant | CODATA |
| Contributes to | What a method does to each property of a record: `sets` the whole value, `adds` an increment, or `uses` it as an input. The page of each method gives it. |  | ThermoFun |
| Contribution | What one method adds to (or replaces in) G, H, S, Cp, V of a result. The result is the sum of the contributions and the changes of the conversions. |  | ThermoFun |
| Convention | How values are defined. The standard state (standard pressure, concentration scale) and the energy reference (of the elements, of water, of the ions). |  | ThermoFun |
| Conversion | A change after the source of the values, on the route of a result. A convention (a method in another convention, the conventions of the results), an option of gases, the fallback to the values of the record, the rounding to the uncertainty. |  | ThermoFun |
| Data source | Where a record comes from. `data_sources` is a list of reference codes of the NEA form (`92GRE/FUG`) or texts. | reference code | NEA TDB-5 1.11 |
| Dataset | The information about a database. Symbol, name, version, conventions. The key is `thermodataset`. |  | ThermoFun |
| Defining reaction | The reaction that gives the properties of a substance. |  | ThermoFun |
| Derivation | How a value of a record was obtained (`quantity.derivation`), whatever its origin. |  | ThermoFun |
| Dielectric model | A method that gives the dielectric constant of water and the Born functions. |  | ThermoFun |
| Electron | With `ions: hydrogen_ion`, the electron of the standard hydrogen electrode. H+ + e- = 1/2 H2(g) with ΔrG° = 0 at every T, so G(e-) = 1/2 G(H2, g) - G(H+). | electron, standard hydrogen electrode | NEA TDB-5 1.7.e |
| Energy reference | From what zero G, H and S of a record are counted. `elements` (such as `benson_helgeson`), `water` (the anchor) and `ions` (`hydrogen_ion`). |  | ThermoFun |
| Engine | The object that calculates. `ThermoEngine`. Its summary (`engine.summary()`) names what the results depend on. |  | ThermoFun |
| Equation of state of water | A method that gives the density of water and its properties as a substance. |  | ThermoFun |
| Expanded uncertainty | U = k u. The 95 % intervals of the NEA reviews are read as expanded uncertainties with k = 1.96. | expanded uncertainty | GUM; GB4 5.1; NEA TDB-3 |
| Fluid | A gas or a supercritical fluid with an equation of state, such as CORK. |  | ThermoFun |
| HKF | The Helgeson-Kirkham-Flowers equation of state of aqueous species. |  | Helgeson et al. (1981), Tanger and Helgeson (1988) |
| Hydrogen ion | With `ions: hydrogen_ion` (the default), H+(aq) has G, H, S, Cp, V = 0 at every T and P. | convention for the hydrogen ion | NEA TDB-5 1.6; CODATA 1989 |
| Implementation | One code of a method. `reaktoro` and `gems` are the implementations of `WaterHGK84`. |  | ThermoFun |
| Increment | A method that adds to what the base method gives. A phase transition, a volume, a term of the pressure. |  | ThermoFun |
| Legacy format | The JSON format before version 1.0, with keys such as `sm_gibbs_energy` and `TPMethods`. |  | ThermoFun |
| Method | A calculation of properties at T and P, with its parameters. A record lists its methods. |  | ThermoFun |
| Model parameter | A value of a model, the same for every record, for example HKF eta or the coefficients of IAPWS-95. It is in a file of `ThermoFun/Methods/ParameterSets`. |  | NEA TDB-4 |
| Parameter | A value that a method reads from a record, such as `a1` of `HKF88`. It can be fitted. |  | NEA TDB-4 |
| Parameter set | A named group of values for a method. A record names it with `parameter_set`. |  | ThermoFun |
| Property source | Where the properties of a record come from. Its methods, its defining reaction, or its reactants. |  | ThermoFun |
| Provenance | How a result of a batch calculation was made. The database and the SHA-256 of its records, the templates, and the summary of each engine. |  | ThermoFun |
| Reactant | A substance in a reaction, with its coefficient. The coefficient is negative on the left side of the equation. | stoichiometric number | IUPAC GB4 2.11 |
| Record | One element, substance, reaction, method definition, parameter set or data source. |  | ThermoFun |
| Reference state | The state of an element in which its energies of formation are 0, such as `O2(g)` or `C(cr, graphite)` (`element.reference_state`). | reference state of an element | NEA TDB-5 3.1; GB4 4.11.1 |
| Reference temperature | The temperature of the values of a record (Tr, `reference_conditions.T`), 298.15 K by default. | reference temperature | NEA TDB-5 3.3 |
| Reference values | The values of a record at its reference temperature and its standard pressure. The key is `standard_state_properties`. |  | ThermoFun |
| Result table | One result of a batch calculation (`ResultTable`). A row for each record and point, a column for each property and each variant of `compare`. |  | ThermoFun |
| Route | How a result was calculated (`result.route`). Where its values came from (`by`), the methods with their contributions, the records it needed (`needs`), the conversions. `engine.route(symbol)` gives it without numbers. |  | ThermoFun |
| Segment | A T interval of a method, with its own parameters. |  | ThermoFun |
| Solute | A substance dissolved in water. Its class is `aqueous_solute`. | solute | IUPAC GB4 2.10 |
| Solvent | The water record that solutes read. `engine.solventSymbol()` gives its symbol. | solvent | IUPAC GB4 2.10 |
| Standard pressure | p°, the pressure of the standard state, `standard_state.pressure`. 1 bar by default, also 1 atm or any pressure. | standard pressure | GB4 4.11.1 (v); NEA TDB-5 3.2 |
| Standard state | The state that a value refers to. The class of the record, its aggregate state, and `standard_state` (pressure, concentration scale). | standard state | NEA TDB-5 Table 1; GB4 4.11.1 |
| Standard uncertainty | u, the uncertainty of a value as one standard deviation. The form in which ThermoFun keeps uncertainties. | standard uncertainty | GUM; GB4 5.1 |
| Status | How a value was made. `calculated`, `read`, `assigned` or `notdefined`. |  | ThermoFun |
| Symbol | The name of a record in a database, such as `Ca+2`. |  | ThermoFun |
| Symbolic method | A method that is an expression in a Method record, with no C++ code. |  | ThermoFun |
| Terms | The named parts of the contribution of one method to one property, such as the Born term of G of HKF88. The catalog names them. |  | ThermoFun |
| v2 format | The format since version 1.0, in YAML or JSON, with the key `thermohub`. |  | ThermoFun |
| Variables | The quantities of a model that its terms come from, not summed, such as the g function of HKF88. The catalog names them. |  | ThermoFun |
| Variant | A parameter set with `default: false`. A record uses it only if it names it. |  | ThermoFun |
| Water anchor | The data of liquid water that tie an equation of state of water to the values of formation. `helgeson_kirkham_1974` or `codata_1989`. Data of the dataset, not a convention, so never converted. |  | ThermoFun; CODATA 1989; Helgeson and Kirkham (1974) |
