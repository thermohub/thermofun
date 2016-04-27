#ifndef SUBSTANCEDATAPRTR_H
#define SUBSTANCEDATAPRTR_H

#include <vector>
typedef std::vector<double> vd;

class SubstanceDataPrTr
{
public:
    SubstanceDataPrTr();

    vd getRV0() const;
    void setRV0(const vd &value);

    vd getRG0() const;
    void setRG0(const vd &value);

    vd getRH0() const;
    void setRH0(const vd &value);

    vd getRS0() const;
    void setRS0(const vd &value);

    vd getRaS0() const;
    void setRaS0(const vd &value);

    vd getRCp0() const;
    void setRCp0(const vd &value);

    double getReferenceTc() const;
    void setReferenceTc(const double &value);

    double getReferencePb() const;
    void setReferencePb(const double &value);

    bool getIsProcessed() const;
    void setIsProcessed(bool value);

private:
    vd rV0,              ///< Standard volume at Tst, Pst "V0" m3/mol; [1] ERROR
       rG0,              ///< Standard Gibbs energy at Tst, Pst  "GO" J/mol; [1] ERROR
       rH0,              ///< Standard enthalpy at Tst, Pst  "H0" J/mol; [1] ERROR
       rS0,              ///< Standard entropy of formation from elements at st.states "S0f" J/K/mol; [1] ERROR
       raS0,             ///< Standard absolute entropy at Tst, Pst "S0" J/K/mol;[1] ERROR
       rCp0,             ///< Standard constant-pressure heat capacity at Tst, Pst "Cp0" J/K/mol; [1] ERROR
       rA0,              ///< Standard Helmholtz energy at reference conditions J/mol
       rU0,              ///< Standard internal energy at reference conditions J/mol
       rCv0;             ///< Standard Isochoric heat capacity at reference conditions J/(mol*K)
    double ReferenceTc,                  ///< Reference temperature for standard state (default 25 C)
           ReferencePb;                  ///< Reference pressure (default 1 bar)
    bool isProcessed;           ///< indicator of processing the data

};

#endif // SUBSTANCEDATAPRTR_H
