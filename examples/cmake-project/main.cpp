// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2019-2026 ThermoFun contributors

// Reads a database and calculates G of one substance. For example:
//     ./app Resources/databases/aq17-thermofun.json Quartz

#include <iostream>

#include <ThermoFun/ThermoFun.h>

int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        std::cout << "Give a database file and the symbol of a substance." << std::endl;
        return 1;
    }
    ThermoFun::ThermoEngine engine(argv[1]);
    double P = 1e5;
    std::cout << "G at 298.15 K and 1 bar: " << engine.thermoPropertiesSubstance(298.15, P, argv[2]).gibbs_energy.val << " J/mol" << std::endl;
    return 0;
}
