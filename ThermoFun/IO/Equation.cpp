// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include "Equation.h"

#include <ThermoFun/Common/Numbers.h>

#include <algorithm>
#include <cctype>
#include <regex>

namespace ThermoFun::v2 {

auto stoichiometryOfEquation(const std::string& equation) -> std::vector<std::pair<std::string, double>>
{
    // the coefficient of "+10.7H2O", "2 H2O" or "H2O"
    auto extract = [](const std::string& text, double& coefficient) -> std::string {
        coefficient = 1.0;
        if (text.empty() || std::isalpha(static_cast<unsigned char>(text[0])) || text[0] == '(')
            return text;
        if (text[0] == '+' && text.size() > 1 && (std::isalpha(static_cast<unsigned char>(text[1])) || text[1] == '('))
            return text.substr(1);
        if (text[0] == '-' && text.size() > 1 && (std::isalpha(static_cast<unsigned char>(text[1])) || text[1] == '('))
        {
            coefficient = -1.0;
            return text.substr(1);
        }
        std::size_t used = 0;
        coefficient = numbers::parseDouble(text, used);
        return text.substr(used);
    };

    std::vector<std::pair<std::string, double>> out;
    auto add = [&](const std::string& side, double sign) {
        static const std::regex separator("\\s+\\+\\s+");
        for (std::sregex_token_iterator it(side.begin(), side.end(), separator, -1), end; it != end; ++it)
        {
            std::string part = *it;
            part.erase(std::remove_if(part.begin(), part.end(), [](unsigned char c) { return c == ' ' || c == '\n' || c == '\t'; }), part.end());
            double coefficient = 1.0;
            const std::string symbol = extract(part, coefficient);
            if (symbol.empty())
                continue;
            const auto known = std::find_if(out.begin(), out.end(), [&](const auto& item) { return item.first == symbol; });
            if (known != out.end())
                known->second = coefficient * sign;
            else
                out.emplace_back(symbol, coefficient * sign);
        }
    };
    const auto equal = equation.find('=');
    add(equation.substr(0, equal), -1.0);
    if (equal != std::string::npos)
        add(equation.substr(equal + 1), 1.0);
    return out;
}

} // namespace ThermoFun::v2
