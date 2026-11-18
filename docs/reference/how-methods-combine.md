<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# How methods combine

A record has one base method and any number of increments. A base method gives all the properties. An increment changes what comes before it.

**The order comes from the effects.** Each method says what it does to G, H, S, Cp and V: nothing, set, add, or read. The engine sorts the methods from this. So the order in a file does not count, and two files with the same methods give the same result.

**A conflict is reported.** Two methods that set the same property of one record are a conflict. `validate` reports it with what to do.

**A reaction also has methods.** Its base method gives log K. A second method can add the volume.

**Where the properties come from.** A substance can come from its methods or from its defining reaction. A reaction can come from its methods or from its reactants. By default a record uses its own methods. Without them, a substance uses its reaction, and a reaction uses its reactants. `property_source` changes this for all records, and `setPropertySource` changes it for one.

**A pressure method on a substance from a reaction.** If the reaction has no pressure dependence, the reaction and the species are at the pressure of the data, and the method of the substance adds the rest. If the reaction has it, the substance does not get it twice.

`engine.calculationPath(symbol)` shows the path, and `engine.explain(T, P, symbol)` shows each step.

## Methods by region

A water equation of state and a dielectric model give their values directly at each T and P. So a record can use two methods of one of these kinds, each in a region. Give the first one a `region`, and list a method of the same kind without a region after it. At each T and P the engine uses the first method whose region holds T and P, in the order of the record. Here the order counts.

```yaml
methods:
  - {method: WaterHGK84, implementation: gems}
  - method: BradleyPitzer79
    region: {t_max: 343.15, p_min: 1.0e+8}
  - method: HelgesonKirkham74
```

This is the dielectric constant of Tanger and Helgeson (1988): Bradley and Pitzer (1979) below 70 °C above 1 kbar, and Helgeson and Kirkham (1974) elsewhere. A missing bound is no limit. `WaterLVS83` has its own region and needs none.

The values jump on the boundary of a region, as in the published models. The methods of a substance or a reaction cannot have a region: their values are integrated from the reference state. `validate` reports a region on such a method, and a method in a region that has no method of its kind after it (`V2-MTH-027`).
