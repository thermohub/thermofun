# Diagnostics

Generated. Do not edit by hand: run `python -m thermofun.docs generate docs/reference`.

| Code | Severity | Meaning | What to do |
|---|---|---|---|
| V2-VER-001 | Error | The major version of the format is not supported. | Convert the file with the ThermoFun version that wrote it, or update ThermoFun. |
| V2-VER-003 | Error | The file has no key `thermohub` with the version of the format. | Add the key at the top of the file, for example `thermohub: 2.0.0-draft.1`. |
| V2-VER-002 | Warning | The minor version of the format is newer than this ThermoFun knows. | Update ThermoFun to read all fields of the file. |
| V2-KEY-001 | Warning | The key is not known. It is kept and written back. | Correct the spelling of the key, or move it into `extensions`. |
| V2-KEY-002 | Warning | An extension field has the name of a standard field. The standard field is written. | Rename the extension field. |
| V2-YAML-001 | Error | The text cannot be read as YAML. | Correct the text at the line and the column that the message gives. |
| V2-ID-001 | Error | The record has no symbol. | Give the record a unique `symbol`. |
| V2-ID-002 | Error | The symbol inside the record is not the same as its key in the map. | Make the `symbol` the same as the key. |
| V2-ID-003 | Warning | Two records have the same symbol. The later record replaces the earlier one. | Rename one record, or remove the one that is not needed. |
| V2-QTY-001 | Error | A quantity written as text is not "number unit". | Write the quantity as "<number> <unit>", for example "-237.1 kJ/mol". |
| V2-QTY-002 | Error | A quantity object has no `value`. | Add `value` to the quantity. |
| V2-QTY-003 | Warning | A quantity has a key that is not known. | Use only `value`, `unit` and `uncertainty` in a quantity. |
| V2-UNIT-001 | Error | The unit cannot be read. | Write the unit in UCUM form, for example J/(mol.K) or cm3/mol. |
| V2-UNIT-002 | Error | The unit has the wrong dimension for this field. | Use a unit of the dimension that the field needs (see the reference of the field). |
| V2-UNC-001 | Error | An expanded uncertainty has no coverage factor. | Add `coverage_factor`, or give the uncertainty as `standard`. |
| V2-UNC-002 | Error | Both a standard and an expanded uncertainty are given. | Keep only one of the two. |
| V2-UNC-003 | Error | The uncertainty is negative. | Give the uncertainty as a positive number. |
| V2-UNC-004 | Error | The coverage factor is not a positive number. | Give a coverage factor larger than 0, for example 2. |
| V2-DS-001 | Error | A published thermodynamic dataset has no version. | Add `version` to the dataset. |
| V2-VOC-001 | Warning | The value is not in the list of allowed words. The word `other` is used. | Use one of the allowed words; the message names the nearest one. |
| V2-MTH-001 | Error | The method id is not known. | Use a method id from the method reference; the message names the nearest one. |
| V2-MTH-002 | Error | The method has no parameter with this name. | Use a parameter name of the method (see its reference page). |
| V2-MTH-003 | Error | A method has both `parameters` and `segments`. | Use `segments` for a method in temperature intervals, else `parameters`. |
| V2-MTH-004 | Error | A parameter that the method needs is missing. | Add the parameter, or give a `parameter_set` that has it. |
| V2-MTH-005 | Error | The method cannot be used for this class of substance. | Use a method for the class of the record (see `Suits` in the method reference). |
| CNV-007 | Warning | The hydrogen ion has values other than 0 in the geochemical convention. | Set the values to 0, or declare the convention of the record. |
| V2-APP-001 | Error | The minimum of a range is not below its maximum. | Correct the limits of the range. |
| V2-MTH-006 | Warning | The temperature segments overlap or leave gaps. | Make each segment start where the one before ends. |
| V2-MTH-009 | Error | The method has no implementation with this name. | Use one of the implementations of the method, or remove the key `implementation`. |
| V2-MTH-008 | Error | The method is known but not implemented. | Use another method for the record, or give its properties as constant values. |
| V2-MTH-010 | Error | The parameters of a Method record are not the same as those of the built-in method. | Use the parameter names and units of the built-in method, or give the Method record another id. |
| V2-MTH-011 | Error | What a Method record changes is not the same as for the built-in method. | Remove `effects` from the Method record, or give it another id. |
| V2-MTH-012 | Error | The record does not have exactly one base method. | Give one method that sets G, H, S, Cp and V; the other methods can only correct them. |
| V2-MTH-013 | Error | Two methods set the same property. | Remove one of the two methods. |
| V2-MTH-014 | Warning | A method adds to the volume, but no method before it sets the volume. | Give the volume at the reference state, or add a method that sets it. |
| V2-MTH-015 | Warning | A transformation is listed after other methods. It runs before them. | List the transformation first, so that the file shows the order of the calculation. |
| V2-MTH-016 | Info | A parameter of the method replaces the value of the parameter set. | Remove the parameter if the value of the parameter set is wanted. |
| V2-MTH-017 | Error | The record has more than one transformation method. | Keep only one transformation. |
| V2-MTH-018 | Error | The arrays of a table do not have nT x nP values, or a grid is not sorted. | Give nT x nP values in each array and sort the temperature and pressure grids. |
| V2-MTH-020 | Error | A composite method is used in a calculation. | Use the methods of the composite one by one. |
| V2-MTH-021 | Warning | A Method record of the dataset replaces a method of the project or a built-in one. | Give the Method record another id if both are wanted. |
| V2-MTH-022 | Warning | The expression of a method and its code give different results on the test cases. | Correct the expression or the code; the message names the test case. |
| V2-RXN-001 | Error | The equation and the stoichiometry of the reaction are not the same. | Correct the equation or the stoichiometry, or give only one of them. |
| V2-RXN-002 | Warning | The reaction is not balanced in elements or charge. | Correct the coefficients; the message names the elements that are not balanced. |
| V2-REF-001 | Error | The defining reaction is not in the database. | Add the reaction, or correct `defining_reaction`. |
| V2-REF-002 | Warning | The stoichiometry names a substance that is not in the database. | Add the substance, or correct its symbol. |
| V2-REF-003 | Error | The method id or alias is not known. | Use a method id from the method reference. |
| V2-REF-004 | Error | The parameter set is not found, or it belongs to another method. | Correct `parameter_set`, or add the parameter set for this method. |
| V2-REF-005 | Warning | The data source is not found. | Add the data source, or correct `data_source`. |
| V2-REF-010 | Error | The defining reactions make a cycle. | Define one of the substances in the cycle by values or by a method, not by a reaction. |
| V2-INC-001 | Error | The files include each other in a cycle. | Remove one of the `include` entries. |
| CNV-001 | Error | A conversion needs entropies of elements that are not in the database. | Add the elements that the message names, with their entropies. |
| CNV-002 | Error | A conversion from molarity needs a solvent, and there is none. | Add the solvent record, or give the values on the molality scale. |
| CNV-003 | Warning | The method uses another convention than the dataset, and the record declares none. The values are converted. | Declare the convention of the record if the values are already in the convention of the dataset. |
| CNV-004 | Warning | The reaction mixes species with different energy references. The values are converted. | No action is needed if the conversion is wanted. |
| CNV-005 | Error | The value of the convention is not known. | Use one of the allowed words; the message names the nearest one. |
| V2-SYM-001 | Error | The expression has a variable or function that is not known. | Use only the variables T, P, the parameters of the method and the allowed functions. |
| V2-SYM-002 | Error | The expression cannot be read. | Correct the expression; the message names the position. |
| V2-SYM-003 | Warning | The source of the expression changed after the expression was made. | Make the expression again from the source. |
| V2-SYM-004 | Error | A test case of the method fails. | Correct the expression or the test case; the message names the test case. |
| V2-SYM-005 | Error | The expression does not give an energy per mole. | Correct the units of the parameters or the expression. |
| V2-SYM-006 | Error | The expression is too large or uses a construct that is not allowed. | Make the expression smaller, or write the method in C++. |
| V2-COV-001 | Error | The covariance matrix is not symmetric. | Correct the matrix so that it is symmetric. |
| V2-COV-002 | Error | The covariance matrix is not positive semi-definite. | Correct the matrix; a correlation must be between -1 and 1. |
| V2-COV-003 | Error | The size of the covariance matrix does not agree with its list of values. | Give a square matrix with one row for each value of the list. |
| V2-COV-004 | Error | A value of the covariance block is not in the record. | Correct the path of the value, or add the value to the record. |
| V2-SRC-001 | Warning | The symbol of the data source does not follow the NEA pattern. | Write the symbol as year + first letters of the authors, for example 1988TAN/HEL. |
| LEG-QRK-001 | Info | The method number and the method name do not agree. The number is used. | Correct the method name, or convert the file to v2. |
| LEG-QRK-002 | Info | The method name has spaces at the start or end. They are removed. | Remove the spaces from the method name. |
| LEG-QRK-003 | Info | The units of the HKF coefficients are not the expected list. The expected units are used. | No action is needed; convert the file to v2 to remove the message. |
| LEG-QRK-004 | Info | The unit of m_expansivity is "kbar". It is read as 1/K and the value is multiplied by 1e5. | Give m_expansivity in 1/K, or convert the file to v2. |
| LEG-QRK-005 | Info | The unit of m_compressibility is "1e-05/K". It is read as kbar. | Give m_compressibility in kbar, or convert the file to v2. |
| LEG-QRK-006 | Info | The aggregate state is missing or empty. The state `other` is used. | Give the aggregate state of the record. |
| LEG-QRK-007 | Info | The record is inside a `properties` object. It is read from there. | No action is needed; convert the file to v2 to remove the message. |
| LEG-QRK-008 | Info | A property has `values: [null]`. The property is missing. | Remove the property, or give its value. |
| LEG-QRK-009 | Error | A coefficient list has more than 1000 values. | Correct the coefficient list. |
| LEG-QRK-010 | Info | A coefficient list is longer than the method uses. The extra values are not used. | Remove the values that the method does not use. |
| LEG-QRK-011 | Info | The `units` list is shorter than `values`, or has empty units. The units of the method are used. | Give one unit for each value, or convert the file to v2. |
| LEG-QRK-012 | Warning | The unit of m_expansivity is not "kbar". The value is read as the legacy parser reads it, which is not a thermal expansion. | Give m_expansivity in 1/K with the label kbar, as the data do, or convert the file to v2. |
| LEG-MTH-003 | Warning | The legacy method asks for a GEMS implementation that does not exist. The Reaktoro implementation of the same model is used. | No action is needed if the results of the Reaktoro implementation are wanted; convert the file to v2 to remove the message. |
| LEG-MTH-001 | Error | The legacy method number is not known. | Correct the method number; the method reference lists the legacy numbers. |
| LEG-MTH-002 | Warning | The legacy method is not implemented, so it is not used. | Use another method for the record; the message names the replacement if there is one. |
| LEG-KEY-001 | Warning | The legacy key is not read by ThermoFun. | Remove the key, or convert the file to v2 and keep it in `extensions`. |
| LEG-KEY-002 | Warning | A coefficient key does not belong to the method of its entry. It is kept. | Move the coefficients to the entry of the method that uses them. |
| LEG-WRT-001 | Warning | Part of the record cannot be written in the legacy format. It is left out. | Write the database in the v2 format to keep all of it. |
