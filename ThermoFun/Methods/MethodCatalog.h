// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_METHODS_METHODCATALOG_H
#define THERMOFUN_METHODS_METHODCATALOG_H

#include <optional>
#include <string>
#include <vector>

#include <ThermoFun/Methods/MethodSpec.h>
#include <ThermoFun/Model/MethodApplication.h>

namespace ThermoFun::model {

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

/// Whether a method of a reaction reads the reference values of the record (log K, dG, dH, dS) by the rule of logKReading:
/// LogKConstant, LogKZeroEntropy, LogKVantHoff and LogKConstantCp always; LogKPolynomial (LogKNordstromMunoz88) only without
/// a0 to a6, when it makes its terms from dCp and the reference values.
auto readsReferenceValues(const MethodApplication& application) -> bool;

} // namespace ThermoFun::model

#endif
