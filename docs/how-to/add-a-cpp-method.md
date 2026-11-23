<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# Add a method in C++

**You need:** a build from the source. For a closed-form model, [a method as data](add-a-method-as-data.md) needs no C++ and no build: the example below is written there as data too ([the same method as data](add-a-method-as-data.md#the-same-method-in-c-and-as-data)). Python code cannot be a method; Python gives a method as data.

A method is a function and a table entry.

1. Write the formula as a function template in `ThermoFun/Methods/<family>/`. It works on `SubstanceState<Scalar>`, so the same code gives the derivatives.

   ```cpp
   template <typename Scalar>
   auto myMethod(SubstanceState<Scalar>& state, const SubstanceInputs<Scalar>& in, const MethodParameters<>& p) -> void
   {
       const Scalar a = p.value(0, in);
       state.G += a * (in.T - in.reference.T);   // dG = a (T - Tr)
       state.S -= a;                             // dS = -d(dG)/dT
       state.H -= a * in.reference.T;            // dH = dG + T dS
   }
   ```

2. Make the method object with `makeMethod`, the formula and a rule for the status. Use a method of the same family as your example.
3. Add a `Spec` to `ThermoFun/Methods/MethodCatalog.cpp`: its id, title, one sentence of what it does, its source, what it contributes to (`contributes(...)`: sets, adds or uses for each property), its parameters and its model parameters. Give each parameter its SI unit and the unit of the source.
4. Add the id to the table of what it depends on (`depends_on`) in the same file.
5. Add one line to `ThermoFun/Methods/MethodRegistry.cpp`.
6. Put the model parameters (the values of the model, the same for every substance) in `ThermoFun/Methods/ParameterSets/<Id>.yaml`, not in the code. Read `ThermoFun/Methods/ParameterSets/README.md`.
7. Add the id to the lists in `tests/model/test_catalog.cpp`.
8. Run `python -m thermofun.docs generate docs/reference` and `python pytests/test_add_data_guide.py --write`.

Add a test with values from the paper, and a test of the derivatives against finite differences. Run `ctest` and `python -m pytest`.
