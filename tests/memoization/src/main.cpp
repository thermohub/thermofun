// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Regression tests for the cache of results of the engine, ThermoFun/Engine/ResultCache.h.
//
// Callers pass P == 0 to mean "at the saturation pressure", and the calculation writes the pressure back. So the
// cache must key on the value at the call, and a hit must deliver the pressure after the calculation, though no
// calculation runs. The engine depends on both.

#include <cstdio>
#include <string>

#include <ThermoFun/Engine/ResultCache.h>

using ThermoFun::v2::ResultCache;
using ThermoFun::v2::ResultKey;

// assert() is compiled out in the default Release build
static int failures = 0;
#define CHECK(condition)                                                     \
    do                                                                       \
    {                                                                        \
        if (!(condition))                                                    \
        {                                                                    \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            ++failures;                                                      \
        }                                                                    \
    } while (false)

static auto key(const std::string& symbol, double T = 300.0, double P = 1e5, int state = -1, bool gas = false) -> ResultKey
{
    return {symbol, T, P, state, gas};
}

int main()
{
    // an entry for each symbol, T, P, state and flag of the gas correction
    {
        ResultCache<int> cache;
        double P = 1e5;
        CHECK(!cache.find(key("A"), P));
        cache.store(key("A"), 1, P, 0);
        cache.store(key("B"), 2, P, 0);
        cache.store(key("A", 301.0), 3, P, 0);
        cache.store(key("A", 300.0, 2e5), 4, P, 0);
        cache.store(key("A", 300.0, 1e5, 1), 5, P, 0);
        cache.store(key("A", 300.0, 1e5, -1, true), 6, P, 0);
        CHECK(cache.size() == 6);
        CHECK(*cache.find(key("A"), P) == 1);
        CHECK(*cache.find(key("B"), P) == 2);
        CHECK(*cache.find(key("A", 301.0), P) == 3);
        CHECK(*cache.find(key("A", 300.0, 2e5), P) == 4);
        CHECK(*cache.find(key("A", 300.0, 1e5, 1), P) == 5);
        CHECK(*cache.find(key("A", 300.0, 1e5, -1, true), P) == 6);
    }

    // P == 0: a hit gives the saturation pressure that the calculation wrote, under the key of the pressure asked for
    {
        ResultCache<int> cache;
        const double Psat = 1554939.22;
        cache.store(key("H2O", 473.15, 0.0), 7, Psat, 0);
        double P = 0.0;
        CHECK(*cache.find(key("H2O", 473.15, 0.0), P) == 7);
        CHECK(P == Psat);

        // no hit: the pressure stays
        double other = 0.0;
        CHECK(!cache.find(key("H2O", 500.0, 0.0), other));
        CHECK(other == 0.0);
    }

    // the least recently used goes first; a hit counts as a use; a lookup does not evict
    {
        ResultCache<int> cache;
        double P = 1e5;
        cache.store(key("old"), 1, P, 2);
        cache.store(key("new"), 2, P, 2);
        CHECK(*cache.find(key("old"), P) == 1);
        CHECK(cache.size() == 2);
        cache.store(key("third"), 3, P, 2);
        CHECK(cache.size() == 2);
        CHECK(cache.find(key("old"), P).has_value());
        CHECK(!cache.find(key("new"), P).has_value());
        CHECK(cache.find(key("third"), P).has_value());
    }

    // a limit of 0 is no limit
    {
        ResultCache<int> cache;
        for (int i = 0; i < 1000; ++i)
            cache.store(key("S", i), i, 1e5, 0);
        CHECK(cache.size() == 1000);
    }

    // clear, and a copy starts empty and stays usable
    {
        ResultCache<int> cache;
        double P = 1e5;
        cache.store(key("A"), 1, P, 0);
        ResultCache<int> copy(cache);
        CHECK(copy.size() == 0);
        copy.store(key("B"), 2, P, 1);
        copy.store(key("C"), 3, P, 1);
        CHECK(copy.size() == 1);
        CHECK(cache.size() == 1);
        cache.clear();
        CHECK(cache.size() == 0);
        CHECK(!cache.find(key("A"), P));
    }

    if (failures)
    {
        std::printf("%d checks failed\n", failures);
        return 1;
    }
    std::printf("all checks passed\n");
    return 0;
}
