// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_IO_LEGACY_LEGACYVOCABULARY_H
#define THERMOFUN_IO_LEGACY_LEGACYVOCABULARY_H

#include <string>

#include <ThermoFun/GlobalVariables.h>
#include <ThermoFun/Model/Vocabulary.h>

// The legacy codes and keys of the words of the data format. Only the legacy reader, the legacy writer and the
// old API use them. A code that is not known gives Other; Other gives the legacy code for "other" (101).

namespace ThermoFun {

auto substanceClassFromLegacy(int code) -> v2::SubstanceClass;
auto toLegacy(v2::SubstanceClass value) -> SubstanceClass::type;

auto aggregateStateFromLegacy(int code) -> v2::AggregateState;
auto toLegacy(v2::AggregateState value) -> AggregateState::type;

/// Only the codes 0 (element) and 4 (charge) have a word: the meaning of 1, 2 and 3 is not verified, so they give
/// Other. A reader keeps the raw code of such an element.
auto elementClassFromLegacy(int code) -> v2::ElementClass;
auto toLegacy(v2::ElementClass value) -> int;

/// The key of a property in a legacy record, or "" if legacy records do not have it.
auto legacyKey(v2::PropertyName p) -> std::string;
auto legacyKey(v2::ReactionPropertyName p) -> std::string;

} // namespace ThermoFun

#endif
