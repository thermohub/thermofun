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

int main()
{
    referenceStaysValidAfterAnEdit();
    threadsShareRecords();
    if (failures)
    {
        std::printf("%d checks failed\n", failures);
        return 1;
    }
    std::printf("records stay immutable\n");
    return 0;
}
