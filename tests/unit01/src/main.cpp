#include <iostream>
#include "tcorrpt.h"

using namespace std;

int main()
{
    /// variables substance ///
    vd sV0,              ///< Standard volume at Tst, Pst "V0" m3/mol; [1] ERROR
       sG0,              ///< Standard Gibbs energy at Tst, Pst  "GO" J/mol; [1] ERROR
       sH0,              ///< Standard enthalpy at Tst, Pst  "H0" J/mol; [1] ERROR
       sS0,              ///< Standard entropy of formation from elements at st.states "S0f" J/K/mol; [1] ERROR
       saS0,             ///< Standard absolute entropy at Tst, Pst "S0" J/K/mol;[1] ERROR
       sCp0;             ///< Standard constant-pressure heat capacity at Tst, Pst "Cp0" J/K/mol; [1] ERROR

    vd G0, H0, S0,         ///< partial molal gibbs energy, enthalpy, entropy at current TP; [1] ERROR
       V0, Cp0,               ///< partial molal volume, heat capacity at current TP; [1] ERROR
       Fug, dVg;              ///< molar fugacity at TP (reserved), dVm of gas (reserved); [1] ERROR

    double Tr = 298.15,                  ///< Reference temperature for standard state (usually 298.15 K)
           Pr = 1e05;                  ///< Reference pressure (usually 1 bar or 10^5 Pa, sometimes 1.013 bar = 1 atm)
    double currentP = 2000,                    ///<
           currentT = 400;                    ///<

   /// END variables substance ///

    cout << "Hello World!" << endl;


    TCorrPT Tcorr = TCorrPT(1,2);

    /// Test EmpCpIntegr ///
    vd Interval, Coeff;
    vvd TCpInterval,
    CpCoefficients;

    // Corundum
    // Reference properties
    V0 = {2.558, 0};
    G0 = {-1581808, 0};
    H0 = {-1675250, 0};
    S0 = {50.9, 0};
    Cp0 = {79.4529, 0};

    Interval = {0,2000};
    Coeff = {139.5, 0.00589, -2.4606e6, -589.2};
    TCpInterval.push_back(Interval);
    CpCoefficients.push_back(Coeff);

    TSubstancePT Corundum;

    SubstanceDataPrTr Corr_data_atPrTr;
    SubstanceDataPT Corr_data_atPT;

    Corr_data_atPrTr.setReferencePb(1);
    Corr_data_atPrTr.setReferenceTc(25);
    Corr_data_atPrTr.setSV0(V0);
    Corr_data_atPrTr.setSG0(G0);
    Corr_data_atPrTr.setSH0(H0);
    Corr_data_atPrTr.setSS0(S0);
    Corr_data_atPrTr.setSCp0(Cp0);

    Corr_data_atPT.setCurrentTc(currentT);
    Corr_data_atPT.setCurrentPb(currentP);

    Corundum.setDataAtPrTr(&Corr_data_atPrTr);
    Corundum.setDataAtPT(&Corr_data_atPT);
    Corundum.setMethod_genEoS(MethodGenEoS_Thrift::type::CTPM_CPT);
    Corundum.setMethod_CorrT(MethodCorrT_Thrift::type::CTM_CST);
    Corundum.setMethod_CorrP(MethodCorrP_Thrift::type::CPM_CEH);
    Corundum.makeTCorrPT();

    TCorrModSubstance * TCorr = Corundum.getTCorr(0);
    TCorr->setTCinterval(TCpInterval);
    TCorr->setCpCoeff(CpCoefficients);

    TCorr->PTparam();
//    TCorr->


//    TCorr[0].set



//    TCorr.PTparam();

    /// END test CP



    cout << "Bye World!" << endl;


    return 0;
}

