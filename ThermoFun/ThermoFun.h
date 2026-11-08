// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

#pragma once

#include <ThermoFun/Batch/ThermoBatch.h>
#include <ThermoFun/Batch/OutputBatch.h>
#include <ThermoFun/ThermoEngine.h>
#include <ThermoFun/Database.h>
#include <ThermoFun/Element.h>
#include <ThermoFun/Substance.h>
#include <ThermoFun/Reaction.h>
#include <ThermoFun/ThermoProperties.h>
#include <ThermoFun/ThermoParameters.h>
#include <ThermoFun/GlobalVariables.h>
#include <ThermoFun/Model/DataFormat.h>
#include <ThermoFun/Model/Records.h>

namespace ThermoFun {
/// The records and the rest of the model were in the namespace ThermoFun::v2 before 1.0; this name stays until 2.0
namespace v2 = model;
}
