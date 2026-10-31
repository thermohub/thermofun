// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Uncertainty.h"

#include <algorithm>
#include <map>

namespace ThermoFun::v2 {

namespace {

auto standardOf(const Quantity& q) -> double
{
    return q.uncertainty ? q.uncertainty->standard : 0.0;
}

auto nameOf(PropertyName name) -> std::string
{
    return toString(name);
}

auto nameOf(ReactionPropertyName name) -> std::string
{
    return toString(name);
}

template <typename Record>
auto methodValues(const Record& record, std::vector<RecordValue>& out) -> void
{
    for (std::size_t i = 0; i < record.methods.size(); ++i)
    {
        const auto& a = record.methods[i];
        const std::string path = "/methods/" + std::to_string(i);
        for (const auto& [name, q] : a.parameters)
            out.push_back({path + "/parameters/" + name, {record.symbol, a.method, name}, standardOf(q)});
        for (std::size_t k = 0; k < a.segments.size(); ++k)
            for (const auto& [name, q] : a.segments[k].parameters)
                out.push_back({path + "/segments/" + std::to_string(k) + "/parameters/" + name, {record.symbol, a.method, k == 0 ? name : name + "[" + std::to_string(k) + "]"}, standardOf(q)});
        for (std::size_t k = 0; k < a.transitions.size(); ++k)
            for (const auto& [name, q] : a.transitions[k])
                if (name != "T" && name != "dTdP")
                    out.push_back({path + "/transitions/" + std::to_string(k) + "/" + name, {record.symbol, a.method, name + "[" + std::to_string(k) + "]"}, standardOf(q)});
    }
}

/// a0: the parameter, or that of the first segment. a0[1]: of the second segment. dS[0]: the change at the first transition.
template <typename Record>
auto findInMethods(Record& record, const ParameterRef& ref) -> Quantity*
{
    const auto open = ref.parameter.find('[');
    const std::string base = ref.parameter.substr(0, open);
    const std::size_t row = open == std::string::npos ? 0 : std::stoul(ref.parameter.substr(open + 1));
    for (auto& a : record.methods)
    {
        if (a.method != ref.method)
            continue;
        if (open == std::string::npos)
            if (const auto it = a.parameters.find(base); it != a.parameters.end())
                return &it->second;
        if (row < a.segments.size())
            if (const auto it = a.segments[row].parameters.find(base); it != a.segments[row].parameters.end())
                return &it->second;
        if (open != std::string::npos && row < a.transitions.size())
            if (const auto it = a.transitions[row].find(base); it != a.transitions[row].end())
                return &it->second;
    }
    return nullptr;
}

auto addAll(UncertainInputs& to, const std::vector<RecordValue>& values, const std::vector<CovarianceBlock>& covariance) -> void
{
    std::map<std::string, std::size_t> places;
    auto place = [&](const RecordValue& value, bool inBlock) {
        const auto it = places.find(value.path);
        if (it != places.end())
        {
            to.inputs[it->second].in_block = to.inputs[it->second].in_block || inBlock;
            return it->second;
        }
        to.inputs.push_back({value.ref, value.standard, inBlock});
        places[value.path] = to.inputs.size() - 1;
        return to.inputs.size() - 1;
    };
    for (const auto& value : values)
        if (value.standard > 0.0)
            place(value, false);
    for (const auto& block : covariance)
    {
        UncertainInputs::Block resolved;
        resolved.matrix = block.matrix;
        for (const auto& path : block.paths)
        {
            const auto it = std::find_if(values.begin(), values.end(), [&](const RecordValue& v) { return v.path == path; });
            // the reader checks the paths of a v2 file
            resolved.inputs.push_back(it == values.end() ? to.inputs.size() : place(*it, true));
            if (it == values.end())
                to.inputs.push_back({{"", "", path}, 0.0, true});
        }
        to.blocks.push_back(std::move(resolved));
    }
}

} // namespace

auto recordValues(const SubstanceRecord& record) -> std::vector<RecordValue>
{
    std::vector<RecordValue> values;
    for (const auto& [name, q] : record.standard_state_properties)
        values.push_back({"/standard_state_properties/" + nameOf(name), {record.symbol, "", nameOf(name)}, standardOf(q)});
    methodValues(record, values);
    return values;
}

auto recordValues(const ReactionRecord& record) -> std::vector<RecordValue>
{
    std::vector<RecordValue> values;
    for (const auto& [name, q] : record.reaction_properties)
        values.push_back({"/reaction_properties/" + nameOf(name), {record.symbol, "", nameOf(name)}, standardOf(q)});
    methodValues(record, values);
    return values;
}

auto findValue(SubstanceRecord& record, const ParameterRef& ref) -> Quantity*
{
    if (ref.method.empty())
    {
        for (auto& [name, q] : record.standard_state_properties)
            if (nameOf(name) == ref.parameter)
                return &q;
        return nullptr;
    }
    return findInMethods(record, ref);
}

auto findValue(ReactionRecord& record, const ParameterRef& ref) -> Quantity*
{
    if (ref.method.empty())
    {
        for (auto& [name, q] : record.reaction_properties)
            if (nameOf(name) == ref.parameter)
                return &q;
        return nullptr;
    }
    return findInMethods(record, ref);
}

auto UncertainInputs::add(const SubstanceRecord& record) -> void
{
    addAll(*this, recordValues(record), record.covariance);
}

auto UncertainInputs::add(const ReactionRecord& record) -> void
{
    addAll(*this, recordValues(record), record.covariance);
}

auto UncertainInputs::variance(const std::vector<double>& J) const -> double
{
    double sum = 0.0;
    for (std::size_t i = 0; i < inputs.size(); ++i)
        if (!inputs[i].in_block)
            sum += J[i] * J[i] * inputs[i].standard * inputs[i].standard;
    for (const auto& block : blocks)
    {
        const std::size_t n = block.inputs.size();
        for (std::size_t a = 0; a < n; ++a)
            for (std::size_t b = 0; b < n; ++b)
                sum += J[block.inputs[a]] * block.matrix[a * n + b] * J[block.inputs[b]];
    }
    return sum;
}

} // namespace ThermoFun::v2
