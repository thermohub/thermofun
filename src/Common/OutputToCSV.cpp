#include "OutputToCSV.h"
#include "ThermoProperties.h"

namespace TCorrPT {


std::string OutputToCSV::getOutputFolderPath() const
{
    return outputFolderPath;
}

void OutputToCSV::setOutputFolderPath(const std::string &value)
{
    outputFolderPath = value;
}

std::string OutputToCSV::getFileThermoPropertiesSubstance() const
{
    return fileThermoPropertiesSubstance;
}

void OutputToCSV::setFileThermoPropertiesSubstance(const std::string &value)
{
    fileThermoPropertiesSubstance = value;
}

std::string OutputToCSV::getFilePropertiesSolvent() const
{
    return filePropertiesSolvent;
}

void OutputToCSV::setFilePropertiesSolvent(const std::string &value)
{
    filePropertiesSolvent = value;
}

std::string OutputToCSV::getFileElectroPropertiesSolvent() const
{
    return fileElectroPropertiesSolvent;
}

void OutputToCSV::setFileElectroPropertiesSolvent(const std::string &value)
{
    fileElectroPropertiesSolvent = value;
}

OutputToCSV::OutputToCSV(std::__cxx11::string path)
{
    std::size_t found = path.find_last_of("/\\");
    path = path.substr(0,found);

    outputFolderPath = path;
}

OutputToCSV::~OutputToCSV()
{
    if (fThermoPropertiesSubstance.is_open()) fThermoPropertiesSubstance.close();
    if (fPropertiesSolvent.is_open()) fPropertiesSolvent.close();
    if (fElectroPropertiesSolvent.is_open()) fElectroPropertiesSolvent.close();
}

auto OutputToCSV::openThermoPropertiesSubstanceFile (std::string fileName) ->void
{
    fileThermoPropertiesSubstance = fileName;

    string fullPath = outputFolderPath + "\/" + fileName;

    fThermoPropertiesSubstance.open( fullPath , ios::trunc );
}

auto OutputToCSV::openPropertiesSolventFile (std::string fileName) ->void
{
    filePropertiesSolvent = fileName;

    string fullPath = outputFolderPath + "\/" + fileName;

    fPropertiesSolvent.open( fullPath , ios::trunc );
}

auto OutputToCSV::openElectroPropertiesSolventFile (std::string fileName) ->void
{
    fileElectroPropertiesSolvent = fileName;

    string fullPath = outputFolderPath + "\/" + fileName;

    fElectroPropertiesSolvent.open( fullPath , ios::trunc );
}

auto OutputToCSV::writeThermoPropertiesSubstance (std::string symbol, double T, double P, ThermoPropertiesSubstance tps ) -> void
{
    string c = ",";

    if (!isHeaderThermoPropSubst)
    {
      fThermoPropertiesSubstance << headerThermoPropSubst;
      isHeaderThermoPropSubst = true;
    }

    fThermoPropertiesSubstance << symbol << c << T << c << P << c <<
                                  tps.gibbs_energy     << c << tps.enthalpy         << c << tps.entropy << c <<
                                  tps.heat_capacity_cp << c << tps.heat_capacity_cv << c << tps.volume  << c <<
                                  tps.helmholtz_energy << c << tps.internal_energy  << endl;
}

auto OutputToCSV::writePropertiesSolvent (std::string symbol, double T, double P, PropertiesSolvent ps ) -> void
{
    string c = ",";

    if (!isHeaderPropSolv)
    {
      fPropertiesSolvent << headerPropSolv;
      isHeaderPropSolv = true;
    }

    fPropertiesSolvent << symbol << c << T << c << P << c <<
                          ps.density   << c << ps.densityT  << c << ps.densityP << c << ps.densityTT << c <<
                          ps.densityTP << c << ps.densityPP << c << ps.Alpha    << c << ps.dAldT     << c << ps.Beta << endl;
}

auto OutputToCSV::writeElectroPropertiesSolvent (std::string symbol, double T, double P, ElectroPropertiesSolvent eps ) -> void
{
    string c = ",";

    if (!isHeaderElectroPropSolv)
    {
      fElectroPropertiesSolvent << headerElectroPropSolv;
      isHeaderElectroPropSolv = true;
    }

    fElectroPropertiesSolvent << symbol << c << T << c << P << c <<
                                 eps.bornN     << c << eps.bornQ     << c << eps.bornU     << c << eps.bornX     << c <<
                                 eps.bornX     << c << eps.bornY     << c << eps.bornZ     << c << eps.epsilon   << c <<
                                 eps.epsilonP  << c << eps.epsilonPP << c << eps.epsilonT  << c << eps.epsilonTP << c <<
                                 eps.epsilonTT << endl;
}

}

