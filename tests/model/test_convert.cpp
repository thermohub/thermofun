// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

// Tests of RecordBuilder.h and Convert.h: the builders, the dataset folder, the shortened numbers, the report and
// convertFile.

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Convert.h"
#include "RecordBuilder.h"
#include "check.h"

using namespace ThermoFun;

namespace {

const std::string AQ17 = "Resources/databases/aq17-thermofun.json";

auto contains(const std::string& text, const std::string& part) -> bool
{
    return text.find(part) != std::string::npos;
}

auto scratch(const std::string& name) -> std::filesystem::path
{
    const auto path = std::filesystem::temp_directory_path() / ("tfun-unit-convert-" + name);
    std::filesystem::remove_all(path);
    return path;
}

void testBuilders()
{
    auto quartz = RecordBuilder::substance("Q", {{"formula", "SiO2"}, {"aggregate_state", "crystal"}, {"class", "component"}, {"charge", 0}});
    quartz.standardState("G", -856288.0).standardState("H", -910700.0).standardState("S", 41.46).standardState("Cp", 44.6)
        .standardState("V", "22.688 cm3/mol");
    RecordMethod cp;
    cp.method = "CpIntegration";
    cp.segments = std::vector<RecordSegment>{{298.15, 844.0, {{"a0", 44.6}, {"a1", 0.0378}, {"a2", -1.0e6}}}};
    quartz.method(cp).method("MurnaghanHP98", {{"alpha0", 1.24e-5}, {"kappa0", "730 kbar"}});
    CHECK(quartz.kind() == "substance" && quartz.section() == "substances" && quartz.symbol() == "Q");
    CHECK(contains(quartz.toJson(), R"("segments":[{"t_min":298.15,"t_max":844.0,"parameters":{"a0":44.6)"));
    CHECK(contains(quartz.toJson(), R"("symbol":"Q")"));
    const auto record = quartz.substanceRecord();
    CHECK(std::abs(record.standard_state_properties.at(model::PropertyName::V).value - 2.2688e-5) < 1e-12);
    CHECK(record.methods.size() == 2 && record.methods[1].parameters.at("kappa0").value == 730e8);
    CHECK(quartz.check().all().empty());

    // a copy is a builder of its own
    auto other = quartz;
    other.set("name", "quartz");
    CHECK(!contains(quartz.toJson(), "quartz") && contains(other.toJson(), R"("name":"quartz")"));

    auto bad = RecordBuilder::substance("B", {{"formula", "SiO2"}, {"aggregate_state", "crystal"}, {"class", "component"}});
    bad.method("NoSuchMethod");
    CHECK(!bad.check().ok());

    auto reaction = RecordBuilder::reaction("R", {{"equation", "A = B"}});
    reaction.properties("logK", RecordValue::object({{"value", -1.0}, {"uncertainty", 0.1}})).method("ConstantReactionProperties");
    CHECK(reaction.reactionRecord().reaction_properties.at(model::ReactionPropertyName::logK).uncertainty->standard == 0.1);
    CHECK(reaction.check().ok());

    bool thrown = false;
    try { reaction.standardState("G", 0.0); } catch (const std::logic_error&) { thrown = true; }
    CHECK(thrown);
    thrown = false;
    try { reaction.substanceRecord(); } catch (const std::logic_error&) { thrown = true; }
    CHECK(thrown);
    thrown = false;
    try { RecordValue::json("{not json"); } catch (const std::invalid_argument&) { thrown = true; }
    CHECK(thrown);
    CHECK(RecordValue::list({1, "a", true, RecordValue()}).text() == R"([1,"a",true,null])");

    auto fe = RecordBuilder::element("Fe", {{"number", 26}, {"molar_mass", 55.845}, {"class", "element"}});
    CHECK(fe.elementRecord().number == 26);
}

void testDatasetFolder()
{
    const Database database(AQ17);
    const auto folder = scratch("folder");
    database.saveDataset(folder.string(), false, false);
    CHECK(std::filesystem::exists(folder / "thermodataset.json"));
    CHECK(std::filesystem::exists(folder / "substances" / "Ca+2.json"));
    const Database back(folder.string());
    CHECK(back.numberOfSubstances() == database.numberOfSubstances());
    CHECK(back.numberOfElements() == database.numberOfElements());
    CHECK(back.toJson(false) == database.toJson(false));
    std::filesystem::remove_all(folder);

    // the record files have no dataset block, so they do not replace the dataset of thermodataset.json: its name and a
    // convention other than the default stay
    const Database nea(R"({"thermohub": "2.0.0-draft.1", "thermodataset": {"symbol": "t", "name": "test",
        "conventions": {"water_triple_point_reference": "NEA_IAPWS"}}, "elements": {"O": {"number": 8, "class": "element"}}})");
    const auto neaFolder = scratch("nea");
    nea.saveDataset(neaFolder.string());
    const Database neaBack(neaFolder.string());
    CHECK(neaBack.conventions().water_triple_point_reference == "NEA_IAPWS");
    CHECK(neaBack.thermoDataSet().name == "test" && neaBack.thermoDataSet().symbol == "t");
    std::ifstream element(neaFolder / "elements" / "O.json");
    std::stringstream elementText;
    elementText << element.rdbuf();
    CHECK(!contains(elementText.str(), "thermodataset"));
    std::filesystem::remove_all(neaFolder);

    // a character that a file name cannot have is one "_", also one of several UTF-8 bytes
    const Database odd(R"({"thermohub": "2.0.0-draft.1", "elements": {"A/é": {"number": 1, "class": "element"}}})");
    const auto yaml = scratch("odd");
    odd.saveDataset(yaml.string(), true);
    CHECK(std::filesystem::exists(yaml / "elements" / "A__.yaml"));
    CHECK(std::filesystem::exists(yaml / "thermodataset.yaml"));
    std::filesystem::remove_all(yaml);
}

void testReportAndConvert()
{
    const Database database(AQ17);
    const auto shortened = shortenedNumbers(database);
    CHECK(shortened && shortened->count > 0 && shortened->largest > 0.0 && shortened->largest < 1e-6);

    ConvertOptions readable;
    readable.readable = true;
    const std::string report = conversionReport(database, AQ17, readable);
    for (const char* heading : {"# Conversion report", "## Summary", "## Quirks applied", "## Unread legacy keys", "## Unresolved references",
                                "## Other problems of the data", "## Methods used", "## Uncertainty convention notice"})
        CHECK(contains(report, heading));
    CHECK(contains(report, "numbers written as short decimals (--readable): " + std::to_string(shortened->count)));
    CHECK(contains(conversionReport(database, AQ17), "numbers written exactly"));

    const auto out = scratch("convert");
    ConvertOptions options;
    options.report = (out / "report.md").string();
    convertFile(AQ17, (out / "aq17.json").string(), options);
    CHECK(Database((out / "aq17.json").string()).toJson(false) == database.toJson(false));
    std::ifstream in(out / "report.md");
    std::stringstream text;
    text << in.rdbuf();
    CHECK(text.str() == conversionReport(database, AQ17) );

    ConvertOptions legacy;
    legacy.to = model::DataFormat::Legacy;
    legacy.yaml = true;
    bool thrown = false;
    try { convertFile(AQ17, (out / "x.json").string(), legacy); } catch (const std::invalid_argument&) { thrown = true; }
    CHECK(thrown);
    std::filesystem::remove_all(out);
}

}

void runConvert()
{
    testBuilders();
    testDatasetFolder();
    testReportAndConvert();
}
