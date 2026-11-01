<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# How methods combine

A record has one base method and any number of corrections. A base method gives all the properties. A correction changes what comes before it.

**The order comes from the effects.** Each method says what it does to G, H, S, Cp and V: nothing, set, add, or read. The engine sorts the methods from this. So the order in a file does not count, and two files with the same methods give the same result.

**A conflict is reported.** Two methods that set the same property of one record are a conflict. `validate` reports it with what to do.

**A reaction also has methods.** Its base method gives log K. A second method can add the volume.

**Where the properties come from.** A substance can come from its methods or from its defining reaction. A reaction can come from its methods or from its reactants. By default a record uses its own methods. Without them, a substance uses its reaction, and a reaction uses its reactants. `property_source` changes this for all records, and `setPropertySource` changes it for one.

**A pressure method on a substance from a reaction.** If the reaction has no pressure dependence, the reaction and the species are at the pressure of the data, and the method of the substance adds the rest. If the reaction has it, the substance does not get it twice.

`engine.calculationPath(symbol)` shows the path, and `engine.explain(T, P, symbol)` shows each step.
