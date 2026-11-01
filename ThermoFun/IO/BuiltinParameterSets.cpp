// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <ThermoFun/Model/BuiltinParameterSets.h>

#include <ThermoFun/Common/Exception.h>
#include <ThermoFun/IO/V2/V2Reader.h>

namespace ThermoFun::v2 {

namespace detail {

/// One file of Methods/ParameterSets. The build generates the table.
struct BuiltinFile
{
    const char* name;
    const unsigned char* text;
    unsigned long size;
};
extern const BuiltinFile builtin_parameter_files[];
extern const unsigned long builtin_parameter_file_count;

} // namespace detail

auto builtinParameterSets() -> const std::map<std::string, MethodParameterSet>&
{
    static const auto sets = [] {
        std::map<std::string, MethodParameterSet> out;
        for (unsigned long i = 0; i < detail::builtin_parameter_file_count; ++i)
        {
            const auto& file = detail::builtin_parameter_files[i];
            Diagnostics diagnostics;
            auto document = readV2Document(std::string(reinterpret_cast<const char*>(file.text), file.size), diagnostics, file.name);
            if (!diagnostics.ok())
                funError(std::string("The constants of the methods in ") + file.name + " cannot be read", diagnostics.toString(), __LINE__, __FILE__);
            for (auto& set : document.parameter_sets)
                out[set.method] = std::move(set);
        }
        return out;
    }();
    return sets;
}

} // namespace ThermoFun::v2
