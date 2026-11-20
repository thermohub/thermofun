// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Records never change: an edit makes a new record. So a reference to a record stays valid after an edit, and threads can
// share records. The ThreadSanitizer job of the CI runs this program.

#include <cstdio>
#include <string>
#include <thread>
#include <vector>

#include "Database.h"
#include "Model/Records.h"
#include "Substance.h"
#include "ThermoEngine.h"
#include "ThermoProperties.h"

using namespace ThermoFun;

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

static const char* const database = "pytests/test-aq17-gem-lma-thermofun.json";

static void referenceStaysValidAfterAnEdit()
{
    Database db(database);
    Substance s = db.getSubstance("Ca+2");
    const auto before = s.record();
    const std::string name = before->name;
    const auto methods = before->methods;

    s.setName("changed");
    s.setMethodGenEoS(MethodGenEoS_Thrift::type::CTPM_CON);

    CHECK(s.record() != before);
    CHECK(before->name == name);
    CHECK(before->methods == methods);
    CHECK(db.getSubstance("Ca+2").name() == name);
    CHECK(s.name() == "changed");

    // the record of a substance dropped from its object stays alive for the one who holds it
    {
        Substance copy = s;
        const auto held = copy.record();
        copy = Substance();
        CHECK(held->name == "changed");
    }
}

// One thread edits copies of a substance while others read the record of the same substance and calculate with it
static void threadsShareRecords()
{
    const Database db(database);
    const Substance shared = db.getSubstance("Ca+2");
    const auto record = shared.record();
    double firstP = 1e6;
    const double expected = ThermoEngine(db).thermoPropertiesSubstance(373.15, firstP, std::string("Ca+2")).gibbs_energy.val;

    std::vector<std::thread> threads;
    std::vector<int> wrong(4, 0);
    for (int t = 0; t < 2; ++t)
        threads.emplace_back([&, t] {
            for (int i = 0; i < 200; ++i)
            {
                Substance copy = shared;
                copy.setName("thread " + std::to_string(t));
                auto reference = copy.thermoReferenceProperties();
                reference.gibbs_energy.val += i;
                copy.setThermoReferenceProperties(reference);
                if (copy.name() != "thread " + std::to_string(t) || record->name == copy.name())
                    ++wrong[t];
            }
        });
    for (int t = 2; t < 4; ++t)
        threads.emplace_back([&, t] {
            // an engine for each thread, on a copy of the database: the copies share the records
            ThermoEngine engine{Database(db)};
            for (int i = 0; i < 50; ++i)
            {
                double P = 1e6;
                if (engine.thermoPropertiesSubstance(373.15, P, std::string("Ca+2")).gibbs_energy.val != expected)
                    ++wrong[t];
                if (engine.database().getSubstance("Ca+2").record() != record)
                    ++wrong[t];
            }
        });
    for (auto& thread : threads)
        thread.join();
    for (const int w : wrong)
        CHECK(w == 0);
}

// A copy of an engine, by construction or by assignment, has its own settings and caches (review of v2, S2)
static void engineCopiesAreIndependent()
{
    const Database db(database);
    ThermoEngine a(db);
    ThermoEngine constructed(a);
    ThermoEngine assigned(db);
    assigned = a;
    constructed.preferences().round_to_uncertainty = true;
    assigned.preferences().round_to_uncertainty = true;
    CHECK(!a.preferences().round_to_uncertainty);
    a.preferences().enable_memoize = false;
    CHECK(constructed.preferences().enable_memoize && assigned.preferences().enable_memoize);
    ThermoEngine moved(std::move(constructed));
    CHECK(moved.preferences().round_to_uncertainty);
}

// The rule for threads: one engine for each thread, copied from a shared one. Threads that calculate different species
// get the values of a serial calculation (a shared engine is not safe yet: review of v2, S1)
static void oneEngineCopyForEachThread()
{
    const ThermoEngine shared{Database(database)};
    double P = 1e7;
    const double water = ThermoEngine(shared).thermoPropertiesSubstance(473.15, P, std::string("H2O@")).gibbs_energy.val;
    P = 1e7;
    const double calcium = ThermoEngine(shared).thermoPropertiesSubstance(473.15, P, std::string("Ca+2")).gibbs_energy.val;
    std::vector<std::thread> threads;
    std::vector<int> wrong(4, 0);
    for (int t = 0; t < 4; ++t)
        threads.emplace_back([&, t] {
            ThermoEngine mine{Database(database)};
            mine = shared;  // the copy of this thread
            for (int i = 0; i < 200; ++i)
            {
                double p = 1e7;
                const bool even = (i + t) % 2 == 0;
                const double g = mine.thermoPropertiesSubstance(473.15, p, std::string(even ? "H2O@" : "Ca+2")).gibbs_energy.val;
                if (g != (even ? water : calcium))
                    ++wrong[t];
            }
        });
    for (auto& thread : threads)
        thread.join();
    for (const int w : wrong)
        CHECK(w == 0);
}

int main()
{
    referenceStaysValidAfterAnEdit();
    threadsShareRecords();
    engineCopiesAreIndependent();
    oneEngineCopyForEachThread();
    if (failures)
    {
        std::printf("%d checks failed\n", failures);
        return 1;
    }
    std::printf("records stay immutable\n");
    return 0;
}
