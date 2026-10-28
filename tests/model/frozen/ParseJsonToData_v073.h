// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

// FROZEN COPY of ThermoFun/Common/ParseJsonToData.h. Do not edit.
#pragma once

#include <string>

namespace ThermoFun {
class Substance;
class Reaction;
class Element;
struct DatabaseConventions;
}

namespace frozen_v073 {

auto parseElement(const std::string& data) -> ThermoFun::Element;
auto parseSubstance(const std::string& data) -> ThermoFun::Substance;
auto parseReaction(const std::string& data) -> ThermoFun::Reaction;
auto readConventions(const std::string& data) -> ThermoFun::DatabaseConventions;

}
