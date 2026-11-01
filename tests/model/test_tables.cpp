// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 ThermoFun contributors

#include <cmath>

#include "Batch/Tables.h"
#include "Database.h"
#include "ThermoEngine.h"
#include "check.h"

void runTables()
{
    using namespace ThermoFun;

    const std::string data = R"({"thermohub": "2.0.0-draft.1", "substances": {"X": {"formula": "Ca", "class": "component", "aggregate_state": "crystal",
        "standard_state_properties": {"G": -1000.0, "H": -900.0, "S": 10.0, "Cp": 10.0, "V": 1e-5}, "methods": [{"method": "ConstantCp"}, {"method": "ConstantVolume"}]}}})";
    Database database(data);

    // a template, changed with functions
    auto setup = TableSetup::fromTemplate("species-properties");
    setup.setDatabase(database);
    setup.setRecords({"X"});
    setup.setTemperatures({298.15, 400.0}, "K");
    setup.setPressures({1.0}, "bar");
    setup.setColumn("G", "J/mol", 2);
    setup.removeColumn("H");
    setup.addColumn("U", "kJ/mol", 1);
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
    setup.setLayout("transposed");
    CHECK(setup.run().at("species").lines().front().front() == "Property");
    setup.setLayout("grid");
    CHECK(setup.run().at("species").propertyGrids().size() == 5);

    // it reads back what it writes
    auto again = TableSetup::fromText(setup.toYaml());
    again.setDatabase(database);
    CHECK(again.run().at("species").toText() == setup.run().at("species").toText());

    // a wrong setup says why
    bool said = false;
    try
    {
        TableSetup::fromTemplate("no-such-template");
    }
    catch (const TableSetupError& error)
    {
        said = std::string(error.what()).find("species-properties") != std::string::npos;
    }
    CHECK(said);

    CHECK(TableSetup::templates().count("logk-grid") == 1);
}
