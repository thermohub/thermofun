#ifndef OUTPUTTOCSV_H
#define OUTPUTTOCSV_H

#include <fstream>
#include <string>

namespace TCorrPT {

class OutputToCSV
{
    std::string outputFolderPath;

    std::string fileThermoPropertiesSubstance;
    std::string filePropertiesSolvent;
    std::string fileElectroPropertiesSolvent;

public:
    OutputToCSV();

    std::ofstream fThermoPropertiesSubstance;
    std::ofstream fPropertiesSolvent;
    std::ofstream fElectroPropertiesSolvent;

    std::string getOutputFolderPath() const;
    void setOutputFolderPath(const std::string &value);
    std::string getFileThermoPropertiesSubstance() const;
    void setFileThermoPropertiesSubstance(const std::string &value);
    std::string getFilePropertiesSolvent() const;
    void setFilePropertiesSolvent(const std::string &value);
    std::string getFileElectroPropertiesSolvent() const;
    void setFileElectroPropertiesSolvent(const std::string &value);

    void deleteOutputFolder( const char * path);
    void makeOutputFolder(const char * path);
};

}

#endif // OUTPUTTOCSV_H
