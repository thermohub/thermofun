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

/// The number that starts at digits, read by hand: for a standard library without from_chars of double (libc++ of macOS
/// before 16). A stream reads "1.5abc", but it can read past the number on "1e", so the end is found first. It reads as
/// from_chars does, except inf and nan, which it does not read. used gets the end of the number; 0 if there is none.
inline auto parseDoubleByHand(std::string_view text, std::size_t digits, std::size_t& used) -> double
{
    used = 0;
    std::size_t end = digits;
    if (end < text.size() && text[end] == '-')
        ++end;
    const std::size_t first = end;
    bool point = false;   // one decimal point: "1.5.2" is 1.5 followed by ".2", as from_chars reads it
    while (end < text.size() && ((text[end] >= '0' && text[end] <= '9') || (text[end] == '.' && !point)))
    {
        if (text[end] == '.')
            point = true;
        ++end;
    }
    if (end == first || (end == first + 1 && point))
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
    double value = 0.0;
    std::istringstream in{std::string(text.substr(digits, end - digits))};
    in.imbue(std::locale::classic());
    if (in >> value)
        used = end;
    return value;
}

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
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
    double value = 0.0;
    const auto r = std::from_chars(text.data() + digits, text.data() + text.size(), value);
    if (r.ec == std::errc() || r.ec == std::errc::result_out_of_range)
        used = static_cast<std::size_t>(r.ptr - text.data());
    return value;
#else
    return parseDoubleByHand(text, digits, used);
#endif
}

/// The same for a text that holds only a number.
inline auto parseDouble(std::string_view text) -> double
{
    std::size_t used = 0;
    return parseDouble(text, used);
}

} // namespace ThermoFun::numbers

#endif
