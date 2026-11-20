// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Checks for the unit tests. They also work with NDEBUG, which turns assert() into nothing.
#pragma once

#include <cstdio>

namespace tfun_test {

inline int& failures()
{
    static int count = 0;
    return count;
}

inline void checkImpl(bool ok, const char* expr, const char* file, int line)
{
    if (!ok)
    {
        ++failures();
        std::printf("  FAIL %s:%d: %s\n", file, line, expr);
    }
}

}

#define CHECK(cond) ::tfun_test::checkImpl((cond), #cond, __FILE__, __LINE__)
