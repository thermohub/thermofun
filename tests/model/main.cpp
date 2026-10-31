// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Runs the unit tests of the model layer and returns the number of failed checks.

#include <cstdio>

#include "check.h"

// One runAll() for each test_*.cpp
void runSha256();
void runCatalog();
void runSubstanceMethods();
void runDiagnostics();
void runLegacyEquivalence();
void runNormalize();
void runVocabulary();
void runUcum();
void runRecords();
void runFormatDetect();
void runLegacyReader();

int main()
{
    runDiagnostics();
    runSha256();
    runCatalog();
    runVocabulary();
    runUcum();
    runRecords();
    runFormatDetect();
    runLegacyReader();
    runSubstanceMethods();
    runLegacyEquivalence();
    runNormalize();

    if (tfun_test::failures() != 0)
        std::printf("tfun-unit: %d check(s) failed\n", tfun_test::failures());
    else
        std::printf("tfun-unit: all checks passed\n");
    return tfun_test::failures() != 0 ? 1 : 0;
}
