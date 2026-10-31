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
#include "Compat/SubstanceCompat.h"
#include <cmath>
#include <map>
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

// The numbers of a line of a dump, without trailing zeros
auto numbersOfLine(const std::string& line) -> std::vector<double>
{
    const auto at = line.find_first_of("=") != std::string::npos ? line.find('=') : line.rfind(']');
    std::vector<double> out;
    if (at == std::string::npos)
        return out;
    std::istringstream in(line.substr(at + 1));
    double x;
    while (in >> x)
        out.push_back(x);
    while (!out.empty() && out.back() == 0.0)
        out.pop_back();
    return out;
}

// The getters answered from the record against the frozen parser. Accepted, with a reason:
// - the rounding of a value that passes through SI, up to 1e-13;
// - the end of an array that no method reads: the record does not hold it, and the migration report names it (LEG-QRK-010);
// - the sign of an error: an error is positive in the record;
// - the code of a method without code: the record does not hold it, and the migration report names it (LEG-MTH-002);
// - the empty marker of GEMS (the smallest float, and 1e5 times it for alpha0): the record holds no value;
// - the general code of a reaction: the reader merges logk_fpt_function into the method of log K, and no user reads it.
// Each difference that is left counts for its getter.
template <class P>
auto sameCoefficientErrors(const P& a, const P& b) -> bool
{
    auto value = [](const std::map<std::string, std::vector<std::vector<double>>>& m, const std::string& key, size_t r, size_t i) {
        const auto it = m.find(key);
        return it != m.end() && r < it->second.size() && i < it->second[r].size() ? std::fabs(it->second[r][i]) : 0.0;
    };
    auto covers = [&](const P& x, const P& y) {
        for (const auto& [key, rows] : x.coefficient_errors)
            for (size_t r = 0; r < rows.size(); ++r)
                for (size_t i = 0; i < rows[r].size(); ++i)
                    if (const double e = std::fabs(rows[r][i]); e > 1e-30 && std::fabs(e - value(y.coefficient_errors, key, r, i)) > 1e-12 * e)
                    {
                        std::printf("    coefficient error %s row %zu place %zu: %.17g and %.17g\n", key.c_str(), r, i, e, value(y.coefficient_errors, key, r, i));
                        return false;
                    }
        return true;
    };
    return covers(a, b) && covers(b, a);
}

void compareGetters(const std::string& kind, const std::string& frozen, const std::string& library, std::map<std::string, size_t>& differences)
{
    // the errors of the coefficients have their own comparison: a row of zeros and no row say the same
    auto linesOf = [](const std::string& text) {
        std::vector<std::string> lines;
        std::istringstream in(text);
        std::string line;
        while (std::getline(in, line))
            if (line.rfind("param.coefficient_errors", 0) != 0 && line.rfind("jsonString", 0) != 0)
                lines.push_back(line);
        return lines;
    };
    const auto fa = linesOf(frozen), fb = linesOf(library);
    if (fa.size() != fb.size())
    {
        if (differences[kind + " number of lines"]++ < 3)
            std::printf("    %s: %zu lines in the frozen parser, %zu in the library\n", kind.c_str(), fa.size(), fb.size());
        return;
    }
    for (size_t line = 0; line < fa.size(); ++line)
    {
        const auto& la = fa[line];
        const auto& lb = fb[line];
        if (la == lb)
            continue;
        const auto name = kind + " " + la.substr(0, la.find_first_of(" ["));
        if (name == "reaction methodGenEOS")
            continue;
        if (name == "substance method_T" && (la == "method_T = 203" || la == "method_T = 202"))
            continue;
        auto va = numbersOfLine(la), vb = numbersOfLine(lb);
        const bool isError = name.find(".err") != std::string::npos;
        const bool isArray = la.find('=') == std::string::npos;
        if (isArray && vb.size() < va.size() && (name.find("HKF_parameters") != std::string::npos || name.find("critical_parameters") != std::string::npos || name.find("volume_BirchM") != std::string::npos))
            va.resize(vb.size());
        if (va.size() == 1 && std::fabs(va[0]) < 1e-30)
            va.clear();
        bool same = va.size() == vb.size();
        for (size_t i = 0; same && i < va.size(); ++i)
            same = std::fabs((isError ? std::fabs(va[i]) : va[i]) - vb[i]) <= 1e-13 * std::max(std::fabs(va[i]), std::fabs(vb[i]));
        if (!same && differences[name]++ < 3)
            std::printf("    %s\n      frozen:  %.150s\n      library: %.150s\n", name.c_str(), la.c_str(), lb.c_str());
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
    std::map<std::string, size_t> getterDifferences;
    for (const auto& file : files)
    {
        std::ifstream in(file);
        const auto j = nlohmann::json::parse(in);
        for (const auto& rec : j.value("substances", nlohmann::json::array()))
        {
            const auto old = frozen_v073::parseSubstance(rec.dump());
            const ThermoFun::Substance now(rec.dump());
            compareGetters("substance", tfun_test::dumpSubstance(old), tfun_test::dumpSubstance(now), getterDifferences);
            if (!sameCoefficientErrors(old.thermoParameters(), now.thermoParameters()))
                ++getterDifferences["substance coefficient_errors"];
        }
        for (const auto& rec : j.value("reactions", nlohmann::json::array()))
        {
            const auto old = frozen_v073::parseReaction(rec.dump());
            const ThermoFun::Reaction now(rec.dump());
            compareGetters("reaction", tfun_test::dumpReaction(old), tfun_test::dumpReaction(now), getterDifferences);
            if (!sameCoefficientErrors(old.thermoParameters(), now.thermoParameters()))
                ++getterDifferences["reaction coefficient_errors"];
        }
        for (const auto& rec : j.value("elements", nlohmann::json::array()))
            compareGetters("element", tfun_test::dumpElement(frozen_v073::parseElement(rec.dump())), tfun_test::dumpElement(ThermoFun::Element(rec.dump())), getterDifferences);
    }
    for (const auto& [name, count] : getterDifferences)
        std::printf("  GETTER %-55s %zu\n", name.c_str(), count);
    CHECK(getterDifferences.empty());
    std::printf("  legacy equivalence: %zu files, %zu substances, %zu reactions, %zu elements\n", files.size(),
                substances, reactions, elements);
    // The golden databases hold thousands of records; a count of 0 means the test read nothing
    CHECK(substances > 5000);
    CHECK(reactions > 700);
    CHECK(elements > 400);
}
