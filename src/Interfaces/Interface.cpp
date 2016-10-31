#include "Interface.h"
#include <fstream>
#include <iostream>
#include <iomanip>

namespace TCorrPT {

struct Interface::Impl
{
    /// The thermo instance
    Thermo thermo;

    OutputOptions                       outOptions;

    std::vector<string>                 substSymbols;

    std::vector<std::vector<double>>    TempPres_pairs = {{25,1}};

    map<int, std::string>               propNames = {{1,"gibbs_energy"}};

    std::map<const std::string, std::string> propUnits  = defaultPropertyUnits;
    std::map<const std::string, int>         propDigits = defaultPropertyDigits;


//    std::string                         header = "";
//    std::vector<std::vector<double>>    results;
//    std::ofstream                       fThermoPropertiesSubstance;


    Impl(const Database& database)
    : thermo(Thermo(database))
    {}

};


//Interface::Interface()
//{

//}

}

