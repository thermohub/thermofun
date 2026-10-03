# Third-party notices

ThermoFun is licensed under the LGPL-3.0-or-later (see [LICENSE](LICENSE)). The files below are derived from other
projects and keep the copyright and license notice of their origin in the file header; the notice must be kept.

## Derived from Reaktoro

Copyright (C) 2014-2015 Allan Leal. These files were first published in Reaktoro under the GPL v3. Reaktoro was relicensed
to the **GNU Lesser General Public License version 2.1 or (at your option) any later version** on 2018-08-25 (Reaktoro
commit `a8abc79e`, "Updated the license from GPLv3.0 to LGPLv2.1"), and the headers of these files carry the notice of
that license (as in Reaktoro today). Reaktoro depends on ThermoFun, so ThermoFun must stay compatible with the LGPL-2.1+ of Reaktoro:

- `ThermoFun/Common/ScalarTypes.hpp`
- `ThermoFun/Common/ThermoProperty.hpp`
- `ThermoFun/Common/ThermoScalar.hpp`
- `ThermoFun/Common/Units.cpp`
- `ThermoFun/Common/Units.hpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterConstants.hpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterElectroState.hpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterElectroStateJohnsonNorton.cpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterElectroStateJohnsonNorton.hpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterHelmholtzState.hpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterHelmholtzStateHGK.cpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterHelmholtzStateHGK.hpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterHelmholtzStateWagnerPruss.cpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterHelmholtzStateWagnerPruss.hpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterThermoState.hpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterThermoStateUtils.cpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterThermoStateUtils.hpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterUtils.cpp`
- `ThermoFun/Substances/Solvent/Reaktoro/WaterUtils.hpp`

## Derived from GEMS3K

Copyright by T. Wagner, D. Kulik, S. Dmitrieva, F. Hingerl, S. Churakov, A. Rysin and others (see each header), GNU Lesser
General Public License version 3 or (at your option) any later version:

- `ThermoFun/Substances/Gases/s_solmod2_.cpp`
- `ThermoFun/Substances/Gases/s_solmod_.cpp`
- `ThermoFun/Substances/Gases/s_solmod_.h`
- `ThermoFun/Substances/Gases/verror.h`

## Files whose origin is to be confirmed

These files have no copyright or license notice. Their names show they implement models taken from Reaktoro (`...reaktoro`)
or GEMS3K (`...gems`), so they are **not** marked with the ThermoFun SPDX header; the authors should confirm their origin and
add the notice of the original (or the ThermoFun header if they are not derived):

- `ThermoFun/Substances/Solute/SoluteADgems.cpp`
- `ThermoFun/Substances/Solute/SoluteADgems.h`
- `ThermoFun/Substances/Solute/SoluteHKFgems.cpp`
- `ThermoFun/Substances/Solute/SoluteHKFgems.h`
- `ThermoFun/Substances/Solute/SoluteHKFreaktoro.cpp`
- `ThermoFun/Substances/Solute/SoluteHKFreaktoro.h`
- `ThermoFun/Substances/Solvent/WaterHGK-JNgems.cpp`
- `ThermoFun/Substances/Solvent/WaterHGK-JNgems.h`
- `ThermoFun/Substances/Solvent/WaterHGKreaktoro.cpp`
- `ThermoFun/Substances/Solvent/WaterHGKreaktoro.h`
- `ThermoFun/Substances/Solvent/WaterJN91reaktoro.cpp`
- `ThermoFun/Substances/Solvent/WaterJN91reaktoro.h`
- `ThermoFun/Substances/Solvent/WaterWP95reaktoro.cpp`
- `ThermoFun/Substances/Solvent/WaterWP95reaktoro.h`
- `pytests/Substances/Solute/test_hkf_reaktoro.py`

## Dependencies (not part of this repository)

autodiff, ChemicalFun, nlohmann_json, spdlog and pybind11 are used as libraries and are not copied here; see their own licenses.
