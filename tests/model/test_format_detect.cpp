// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Tests of IO/FormatDetect.h: documents and records of both formats, and the edge cases.

#include <fstream>
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

#include "IO/FormatDetect.h"
#include "IO/detail/JsonDetect.h"
#include "check.h"

using namespace ThermoFun::v2;

namespace {

void testDocuments()
{
    CHECK(detectDocumentFormat(R"({"thermohub": {"schema": "2.0"}, "substances": {}})") == DataFormat::V2);
    CHECK(detectDocumentFormat(R"({"thermohub": {}})") == DataFormat::V2);
    // a legacy document: sections, an array of records, an empty object
    CHECK(detectDocumentFormat(R"({"elements": [], "substances": [], "reactions": []})") == DataFormat::Legacy);
    CHECK(detectDocumentFormat(R"([{"_label": "substance", "symbol": "Fe"}])") == DataFormat::Legacy);
    CHECK(detectDocumentFormat("{}") == DataFormat::Legacy);
    CHECK(detectDocumentFormat("[]") == DataFormat::Legacy);
    // "thermohub" in a nested object is not the key of the document
    CHECK(detectDocumentFormat(R"({"substances": [{"thermohub": 1}]})") == DataFormat::Legacy);
    // text that is not JSON
    CHECK(detectDocumentFormat("thermohub: 2.0\nsubstances: {}") == DataFormat::Auto);
    CHECK(detectDocumentFormat("") == DataFormat::Auto);
    CHECK(detectDocumentFormat("{\"a\": ") == DataFormat::Auto);
}

void testRecords()
{
    // v2 keys
    for (const char* key : {"standard_state_properties", "methods", "stoichiometry", "reaction_properties", "record_type"})
        CHECK(detectRecordFormat(std::string("{\"symbol\": \"X\", \"") + key + "\": 1}") == DataFormat::V2);

    // legacy keys
    for (const char* key : {"TPMethods", "sm_gibbs_energy", "sm_enthalpy", "sm_entropy_abs", "sm_heat_capacity_p",
                            "sm_volume", "logKr", "drsm_gibbs_energy", "reactants", "_label", "properties", "class_", "Tst"})
        CHECK(detectRecordFormat(std::string("{\"symbol\": \"X\", \"") + key + "\": 1}") == DataFormat::Legacy);

    // no key of either format: legacy, as today
    CHECK(detectRecordFormat(R"({"symbol": "Fe", "formula": "Fe"})") == DataFormat::Legacy);
    CHECK(detectRecordFormat("{}") == DataFormat::Legacy);
    // a key of the v2 format wins
    CHECK(detectRecordFormat(R"({"TPMethods": [], "methods": []})") == DataFormat::V2);
    // not an object
    CHECK(detectRecordFormat("[1, 2]") == DataFormat::Auto);
    CHECK(detectRecordFormat("5") == DataFormat::Auto);
    CHECK(detectRecordFormat("not json") == DataFormat::Auto);
}

void testJsonOverloads()
{
    using nlohmann::json;
    CHECK(detail::detectDocumentFormat(json::parse(R"({"thermohub": {}})")) == DataFormat::V2);
    CHECK(detail::detectDocumentFormat(json::parse("[]")) == DataFormat::Legacy);
    CHECK(detail::detectRecordFormat(json::parse(R"({"methods": []})")) == DataFormat::V2);
    CHECK(detail::detectRecordFormat(json::parse("3")) == DataFormat::Auto);
}

// The bundled databases and the records in them are all legacy
void testBundledFiles()
{
    for (const char* file : {"Resources/databases/aq17-thermofun.json", "pytests/test-database-thermofun.json",
                             "tests/extra_models/extra_models_db.json"})
    {
        std::ifstream in(file);
        CHECK(in.good());
        std::stringstream text;
        text << in.rdbuf();
        CHECK(detectDocumentFormat(text.str()) == DataFormat::Legacy);

        const auto j = nlohmann::json::parse(text.str());
        for (const char* section : {"substances", "reactions", "elements"})
            if (j.contains(section))
                for (const auto& record : j[section])
                    CHECK(detail::detectRecordFormat(record) == DataFormat::Legacy);
    }
}

}

void runFormatDetect()
{
    testDocuments();
    testRecords();
    testJsonOverloads();
    testBundledFiles();
}
