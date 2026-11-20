<!--
 SPDX-License-Identifier: LGPL-3.0-or-later
 Copyright (C) 2026 ThermoFun contributors
-->

# The legacy format

ThermoFun reads and writes this format. [Convert a legacy database](../how-to/convert-legacy-database.md) gives the steps.

| Legacy key | v2 |
|---|---|
| `sm_gibbs_energy`, `sm_enthalpy`, `sm_entropy_abs`, `sm_heat_capacity_p`, `sm_volume` | `standard_state_properties`: `G`, `H`, `S`, `Cp`, `V` |
| `TPMethods` | `methods` |
| `reaction` of a substance | `defining_reaction` |
| `reactants` | `stoichiometry` |
| `logKr`, `drsm_gibbs_energy`, `drsm_enthalpy`, ... | `reaction_properties`: `logK`, `dG`, `dH`, ... |
| `limitsTP` | `applicability`, or `segments` of a method |
| `Tst`, `Pst` | `reference_conditions` |
| `m_expansivity`, `m_compressibility` | `alpha0`, `kappa0` of the mineral methods |
| `m_landau_phase_trans_props` | `Tc0`, `Smax`, `Vmax` of `LandauHP98` |
| `entropy`, `atomic_mass` of an element | `standard_entropy`, `molar_mass` |
| `datasources` | `data_sources` |

The page of each method in [Methods](methods/index.md) lists its legacy names.

A substance with no `TPMethods` is calculated from its reaction. A coefficient is `{"values": [...], "units": [...]}`, with `"errors": [...]` for an uncertainty.

A `cp_ft_equation` substance with no coefficients has a constant Cp equal to `sm_heat_capacity_p`, with a warning. A gas with a reaction and no methods gets `IdealGas` when the file is read. The method is not written back to a legacy file.
