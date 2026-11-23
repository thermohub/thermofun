// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <cmath>

#include "Batch/BatchSetup.h"
#include "Database.h"
#include "ThermoEngine.h"
#include "check.h"

void runBatch()
{
    using namespace ThermoFun;

    const std::string data = R"({"thermohub": "2.0.0-draft.1", "substances": {"X": {"formula": "Ca", "class": "component", "aggregate_state": "crystal",
        "standard_state_properties": {"G": -1000.0, "H": -900.0, "S": 10.0, "Cp": 10.0, "V": 1e-5}, "methods": [{"method": "ConstantCp"}, {"method": "ConstantVolume"}]}}})";
    Database database(data);

    // a template, changed with functions
    auto setup = BatchSetup::fromTemplate("species-properties");
    setup.setDatabase(database);
    setup.setSubstances({"X"});
    setup.setTemperatures({298.15, 400.0}, "K");
    setup.setPressures({1.0}, "bar");
    setup.setProperty("G", "J/mol", 2);
    setup.removeProperty("H");
    setup.addProperty("U", "kJ/mol", 1);
    const auto results = setup.run();
    const auto& table = results.at("species");
    CHECK(table.rows().size() == 2);
    CHECK(table.columns().front().label == "G(J/mol)");
    CHECK(table.columns().back().property == "U");

    ThermoEngine engine(database);
    double P = 1e5;
    const double G = engine.thermoPropertiesSubstance(400.0, P, "X").gibbs_energy.val;
    CHECK(std::fabs(table.cell("X", 400.0, 1.0, "G").value - G) < 1e-9);
    CHECK(table.problems().empty());

    // the text, in each layout and format
    CHECK(table.toText("csv").rfind("Symbol,T(K),P(bar),G(J/mol)", 0) == 0);
    CHECK(table.toText("md").rfind("| Symbol |", 0) == 0);
    CHECK(table.toText("tex").find("\\begin{tabular}") != std::string::npos);
    setup.setLayout("wide");
    CHECK(setup.run().at("species").lines().front().front() == "Property");
    setup.setLayout("grid");
    CHECK(setup.run().at("species").propertyGrids().size() == 5);

    // it reads back what it writes
    auto again = BatchSetup::fromText(setup.toYaml());
    again.setDatabase(database);
    CHECK(again.run().at("species").toText() == setup.run().at("species").toText());

    // a wrong setup says why
    bool said = false;
    try
    {
        BatchSetup::fromTemplate("no-such-template");
    }
    catch (const BatchSetupError& error)
    {
        said = std::string(error.what()).find("species-properties") != std::string::npos;
    }
    CHECK(said);

    CHECK(BatchSetup::templates().count("logk-grid") == 1);

    // a value by its path, and the placeholders of blank name their paths
    auto blank = BatchSetup::fromTemplate("blank");
    bool placeholders = false;
    try
    {
        blank.run();
    }
    catch (const BatchSetupError& error)
    {
        placeholders = std::string(error.what()).find("/database (<path of the database>)") != std::string::npos;
    }
    CHECK(placeholders);
    blank.setDatabase(database).set("/conditions/T/values", "[298.15]").set("/conditions/T/unit", "\"K\"").set("/conditions/P/values", "[1]");
    blank.set("/results/0/name", "\"b\"").set("/results/0/substances", "[\"X\"]");
    CHECK(blank.get("/results/0/name") == "\"b\"");
    CHECK(blank.run().at("b").rows().size() == 1);

    // a rule instead of a list, from a template and settings of another
    const std::string more = R"({"thermohub": "2.0.0-draft.1", "substances": {
        "Y": {"formula": "Na", "class": "component", "aggregate_state": "crystal",
              "standard_state_properties": {"G": -2000.0, "H": -1900.0, "S": 20.0, "Cp": 20.0, "V": 2e-5}, "methods": [{"method": "ConstantCp"}, {"method": "ConstantVolume"}]},
        "X": {"formula": "Ca", "class": "component", "aggregate_state": "crystal",
              "standard_state_properties": {"G": -1000.0, "H": -900.0, "S": 10.0, "Cp": 10.0, "V": 1e-5}, "methods": [{"method": "ConstantCp"}, {"method": "ConstantVolume"}]}}})";
    Database two(more);
    CHECK(selectSubstances(two, "{elements: [Na]}") == std::vector<std::string>{"Y"});
    CHECK(selectSubstances(two, "{class: component}").size() == 2);
    bool empty = false;
    try
    {
        selectSubstances(two, "{elements: [K]}");
    }
    catch (const BatchSetupError& error)
    {
        empty = std::string(error.what()).find("chooses no substance") != std::string::npos;
    }
    CHECK(empty);

    auto chosen = BatchSetup::fromTemplate("species-properties");
    chosen.useTemplate("conditions-reference").setDatabase(two);
    chosen.setSubstanceRule("{match: 'Y'}");
    const auto chosenResults = chosen.run();
    const auto& one = chosenResults.at("species");
    CHECK(one.rows().size() == 1 && one.rows()[0].record == "Y");
    CHECK(one.provenance().templates == (std::vector<std::string>{"species-properties", "conditions-reference"}));
    CHECK(!one.provenance().calculations.empty() && one.provenance().calculations[0].second.content_hash.size() == 64);
    CHECK(one.toText("csv", true).rfind("# database: given in memory", 0) == 0);

    // the same setup as a ThermoBatch: the same number
    auto calculation = chosen.batch("species");
    CHECK(calculation.symbols() == std::vector<std::string>{"Y"});
    const auto batchValue = calculation.run().to2DVectorDouble();
    CHECK(batchValue.size() == 1);
    ThermoEngine engine2(two);
    double Pbar = 1e5;
    CHECK(std::fabs(batchValue[0][0] - engine2.thermoPropertiesSubstance(298.15, Pbar, "Y").gibbs_energy.val) < 1e-9);
    CHECK(calculation.batch().preferences().writeProvenance);
}
