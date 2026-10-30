// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_MODEL_RECORDS_H
#define THERMOFUN_MODEL_RECORDS_H

// All record types of the model

#include <ThermoFun/Model/Convention.h>
#include <ThermoFun/Model/Covariance.h>
#include <ThermoFun/Model/Applicability.h>
#include <ThermoFun/Model/DataFormat.h>
#include <ThermoFun/Model/DataSource.h>
#include <ThermoFun/Model/ElementRecord.h>
#include <ThermoFun/Model/MethodApplication.h>
#include <ThermoFun/Model/MethodDefinition.h>
#include <ThermoFun/Model/MethodParameterSet.h>
#include <ThermoFun/Model/Quantity.h>
#include <ThermoFun/Model/ReactionRecord.h>
#include <ThermoFun/Model/ReferenceConditions.h>
#include <ThermoFun/Model/SubstanceRecord.h>
#include <ThermoFun/Model/ThermoDataSetInfo.h>
#include <ThermoFun/Model/Vocabulary.h>

#include <memory>

namespace ThermoFun::v2 {

// Records are immutable once they are in a database: everything holds them through these pointers, and a change
// makes a new record.
using ElementRecordPtr = std::shared_ptr<const ElementRecord>;
using SubstanceRecordPtr = std::shared_ptr<const SubstanceRecord>;
using ReactionRecordPtr = std::shared_ptr<const ReactionRecord>;

} // namespace ThermoFun::v2

#endif
