// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_METHODCATALOG_H
#define THERMOFUN_METHODS_METHODCATALOG_H

#include <optional>
#include <string>
#include <vector>

#include <ThermoFun/Methods/MethodSpec.h>

namespace ThermoFun::v2 {

/// The specifications of all built-in methods. Data only: it has no code of the methods.
class MethodCatalog
{
public:
    /// The one catalog of the library.
    static auto instance() -> const MethodCatalog&;

    /// All specifications, the substance methods first.
    auto specs() const -> const std::vector<MethodSpec>&;

    /// The method with this id or alias, or nullptr. Give operatesOn if the name may mean a substance or a reaction method.
    auto find(const std::string& idOrAlias, std::optional<OperatesOn> operatesOn = std::nullopt) const -> const MethodSpec*;

    /// The id nearest to a name that is not known, for "did you mean", or "" if no id is near.
    auto suggest(const std::string& unknown) const -> std::string;

private:
    MethodCatalog();
    std::vector<MethodSpec> specs_;
};

} // namespace ThermoFun::v2

#endif
