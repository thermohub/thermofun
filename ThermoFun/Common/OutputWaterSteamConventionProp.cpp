#include "Common/OutputWaterSteamConventionProp.h"
#include "GlobalVariables.h"
#include <fstream>

namespace ThermoFun {

auto OutputSteamConventionH2OProp (std::string filename, const WaterThermoState wt) -> void
{
//    std::ofstream myfile;
////    filename = Outputpath + filename;
//    myfile.open (filename, std::ios::app);
//    string c=",";

//    if ( myfile.tellp() == 0 )
//    {
//        myfile <<"T,P,Cp,Cv,RHO,H,S,G,A,U,V\n";
//    }

//    myfile << wt.temperature << c << wt.pressure << c << wt.cp << c << wt.cv << c << wt.density << c <<
//              wt.enthalpy << c << wt.entropy << c << wt.gibbs << c << wt.helmholtz << c <<
//              wt.internal_energy << c << wt.volume << endl;
//    myfile.close();

}

}
