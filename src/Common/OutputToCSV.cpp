#include "OutputToCSV.h"

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

OutputToCSV::OutputToCSV()
{

}

}

