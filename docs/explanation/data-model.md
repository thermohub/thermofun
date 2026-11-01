<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# The data model

A database holds records: elements, substances, reactions, method definitions, parameter sets and data sources. The records come first. The formats and the old API are only ways in and out.

**Values are in SI inside.** A reader converts each value from the unit of its file. The formulas receive the units they need. So a record has one meaning, whatever file it came from.

**Missing is not zero.** A value that is not known is not there. A method that needs it gives the status `notdefined`.

**A record does not change.** An engine binds a record to method objects once, and keeps them. A change makes a new record, and the engine calculates again only what depends on it.

**A method belongs to a record.** A record lists its methods. The class or the aggregate state of a substance does not decide a calculation. For example, a gas has a pressure term because its record has a fluid method or `IdealGas`.

**Water passes its properties on.** The record of the solvent has its methods. A solute and a dielectric model read what those methods give.

**Constants are data.** The constants of a model are in `ThermoFun/Methods/ParameterSets`, in the form of a database section. A model can have variants, and a record can replace a value for itself.
