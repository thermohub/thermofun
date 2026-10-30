// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_CONVENTIONS_HYDROGENION_H
#define THERMOFUN_CONVENTIONS_HYDROGENION_H

#include <ThermoFun/Model/SubstanceRecord.h>

namespace ThermoFun::v2 {

/// True for the record of the hydrogen ion, whose properties are all 0 in the aqueous convention. The rule looks at
/// the class and the formula, not at the symbol. A record may have no charge: it is then the charge of the formula.
inline auto isHydrogenIon(const SubstanceRecord& record) -> bool
{
    return record.class_ == SubstanceClass::AqueousSolute && record.formula == "H+" && record.charge.value_or(1) == 1;
}

} // namespace ThermoFun::v2

#endif
