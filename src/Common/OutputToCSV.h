#ifndef OUTPUTTOCSV_H
#define OUTPUTTOCSV_H

#include <fstream>
#include <string>

namespace TCorrPT {

struct ThermoPropertiesSubstance;
struct PropertiesSolvent;
struct ElectroPropertiesSolvent;

class OutputToCSV
{
    std::string outputFolderPath;

    std::string fileThermoPropertiesSubstance;
    std::string filePropertiesSolvent;
    std::string fileElectroPropertiesSolvent;

    bool isHeaderThermoPropSubst = false;
    bool isHeaderPropSolv = false;
    bool isHeaderElectroPropSolv = false;

    const std::string headerThermoPropSubst  = "Substance,T(C),P(bar),G0(J/mol),H0(J/mol),S0(J/molK),Cp0(J/molK),Cv0(J/molK),V0(J/bar),"
                                               "A0(J/mol),U0(J/mol)\n";
    const std::string headerElectroPropSolv  = "Substance,T(C),P(bar),bornN(Pa-2),bornQ(Pa-1),bornU(Pa-1K-1),bornX(K-2),bornY(K-1),bornZ,"
                                               "eps,eps_dP,eps_d2P,eps_dT,eps_dTdP,eps_d2T\n";
    const std::string headerPropSolv         = "Substance,T(C),P(bar),Rho(kg/m3),Rho_dT((kg/m3)/K),Rho_dP((kg/m3)/Pa),Rho_d2T((kg/m3)/K2),"
                                               "Rho_dTdP((kg/m3)/KPa),Rho_d2P((kg/m3)/Pa2,Aplha(1/K),Alpha_dT,Beta(1/Pa)\n";

public:
    OutputToCSV(std::string path);
    ~OutputToCSV();

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
    void makeOutputFolder( const char * path);

    auto openThermoPropertiesSubstanceFile (std::string fileName) -> void;
    auto openPropertiesSolventFile (std::string fileName) ->void;
    auto openElectroPropertiesSolventFile (std::string fileName) ->void;

    auto closeThermoPropertiesSubstanceFile () -> void;
    auto closePropertiesSolventFile () ->void;
    auto closeElectroPropertiesSolventFile () ->void;

    auto writeThermoPropertiesSubstance (std::string symbol, double T, double P, ThermoPropertiesSubstance tps ) -> void;
    auto writePropertiesSolvent (std::string symbol, double T, double P, PropertiesSolvent ps ) -> void;
    auto writeElectroPropertiesSolvent (std::string symbol, double T, double P, ElectroPropertiesSolvent eps ) -> void;
};

}

#endif // OUTPUTTOCSV_H
