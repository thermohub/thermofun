// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#ifndef THERMOFUN_COMMON_NUMBERS_H
#define THERMOFUN_COMMON_NUMBERS_H

#include <charconv>
#include <cstddef>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>

namespace ThermoFun::numbers {

/// A number at the start of a text, with a point as the decimal mark in all locales: strtod and stod read the mark of the locale of
/// the program, so "1.5" can give 1. used gets the count of characters that the number has; 0 if there is no number.
inline auto parseDouble(std::string_view text, std::size_t& used) -> double
{
    used = 0;
    std::size_t start = 0;
    while (start < text.size() && (text[start] == ' ' || text[start] == '\t' || text[start] == '\n' || text[start] == '\r'))
        ++start;
    // from_chars does not read a plus sign
    const std::size_t digits = start < text.size() && text[start] == '+' ? start + 1 : start;
    double value = 0.0;
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
    const auto r = std::from_chars(text.data() + digits, text.data() + text.size(), value);
    if (r.ec == std::errc())
        used = static_cast<std::size_t>(r.ptr - text.data());
    else if (r.ec == std::errc::result_out_of_range)
        used = static_cast<std::size_t>(r.ptr - text.data());
#else
    // the end of the number by hand: a stream reads "1.5abc" but it can read past the number on "1e"
    std::size_t end = digits;
    if (end < text.size() && text[end] == '-')
        ++end;
    const std::size_t first = end;
    while (end < text.size() && ((text[end] >= '0' && text[end] <= '9') || text[end] == '.'))
        ++end;
    if (end == first)
        return 0.0;
    if (end < text.size() && (text[end] == 'e' || text[end] == 'E'))
    {
        std::size_t tail = end + 1;
        if (tail < text.size() && (text[tail] == '+' || text[tail] == '-'))
            ++tail;
        if (tail < text.size() && text[tail] >= '0' && text[tail] <= '9')
        {
            while (tail < text.size() && text[tail] >= '0' && text[tail] <= '9')
                ++tail;
            end = tail;
        }
    }
    std::istringstream in{std::string(text.substr(digits, end - digits))};
    in.imbue(std::locale::classic());
    if (in >> value)
        used = end;
#endif
    return value;
}

/// The same for a text that holds only a number.
inline auto parseDouble(std::string_view text) -> double
{
    std::size_t used = 0;
    return parseDouble(text, used);
}

} // namespace ThermoFun::numbers

#endif
