// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// The parser of the library must give the same records as the frozen copy of the legacy parser.
// Every record of every golden database and of every synthetic fixture is parsed with both.

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Common/ParseJsonToData.h"
#include "check.h"
#include "dump.h"
#include "frozen/ParseJsonToData_v073.h"

namespace {

namespace fs = std::filesystem;

// The databases of the golden suite (paths relative to the source directory)
const char* const databases[] = {
    "Resources/databases/aq17-thermofun.json",
    "Resources/databases/cemdata18-thermofun.json",
    "Resources/databases/heracles-thermofun.json",
    "Resources/databases/mines16-thermofun.json",
    "Resources/databases/psinagra-12-07-thermofun.json",
    "Resources/databases/slop98-organic-thermofun.json",
    "Resources/databases/slop98-thermofun.json",
    "pytests/Fe-O_system.json",
    "pytests/PsiTDB2020-subset-thermofun.json",
    "pytests/Reactions/Co_Cu_aq-thermofun.json",
    "pytests/Reactions/test-logKfT-thermofun.json",
    "pytests/Substances/Gases/mines16-thermofun.json",
    "pytests/Substances/Solids/Mo(s).json",
    "pytests/Substances/Solute/test-hkf-thermofun.json",
    "pytests/Substances/Solvent/water-thermofun.json",
    "pytests/mines16-sub-thermofun.json",
    "pytests/test-aq17-gem-lma-thermofun.json",
    "pytests/test-database-thermofun.json",
    "pytests/test-thermoengine-thermofun.json",
    "tests/extra_models/extra_models_db.json",
};

// Every database in these folders is read: the synthetic fixtures and the model databases with published values
const char* const databaseDirectories[] = {"pytests/data/legacy-synthetic", "pytests/data/published"};

std::vector<std::string> allFiles()
{
    std::vector<std::string> files(std::begin(databases), std::end(databases));
    for (const auto* directory : databaseDirectories)
    {
        std::vector<std::string> found;
        for (const auto& entry : fs::directory_iterator(directory))
            if (entry.path().extension() == ".json")
                found.push_back(entry.path().generic_string());
        std::sort(found.begin(), found.end());
        files.insert(files.end(), found.begin(), found.end());
    }
    return files;
}

// Compares the dumps of one record; prints the first differing line
bool sameRecord(const std::string& kind, const std::string& file, size_t index, const std::string& a, const std::string& b)
{
    if (a == b)
        return true;
    std::istringstream sa(a), sb(b);
    std::string la, lb;
    std::printf("  DIFFERENT %s %zu of %s:\n", kind.c_str(), index, file.c_str());
    int shown = 0;
    while (std::getline(sa, la) && std::getline(sb, lb))
        if (la != lb && shown++ < 5)
            std::printf("    frozen:  %s\n    library: %s\n", la.c_str(), lb.c_str());
    return false;
}

}

void runLegacyEquivalence()
{
    size_t substances = 0, reactions = 0, elements = 0;
    const auto files = allFiles();
    for (const auto& file : files)
    {
        std::ifstream in(file);
        CHECK(in.good());
        nlohmann::json j = nlohmann::json::parse(in);

        size_t i = 0;
        for (const auto& rec : j.value("substances", nlohmann::json::array()))
        {
            const std::string text = rec.dump();
            CHECK(sameRecord("substance", file, i++, tfun_test::dumpSubstance(frozen_v073::parseSubstance(text)),
                             tfun_test::dumpSubstance(ThermoFun::parseSubstance(text))));
            ++substances;
        }
        i = 0;
        for (const auto& rec : j.value("reactions", nlohmann::json::array()))
        {
            const std::string text = rec.dump();
            CHECK(sameRecord("reaction", file, i++, tfun_test::dumpReaction(frozen_v073::parseReaction(text)),
                             tfun_test::dumpReaction(ThermoFun::parseReaction(text))));
            ++reactions;
        }
        i = 0;
        for (const auto& rec : j.value("elements", nlohmann::json::array()))
        {
            const std::string text = rec.dump();
            CHECK(sameRecord("element", file, i++, tfun_test::dumpElement(frozen_v073::parseElement(text)),
                             tfun_test::dumpElement(ThermoFun::parseElement(text))));
            ++elements;
        }
    }
    std::printf("  legacy equivalence: %zu files, %zu substances, %zu reactions, %zu elements\n", files.size(),
                substances, reactions, elements);
    // The golden databases hold thousands of records; a count of 0 means the test read nothing
    CHECK(substances > 5000);
    CHECK(reactions > 700);
    CHECK(elements > 400);
}
