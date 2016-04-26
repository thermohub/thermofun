#ifndef SUBSTANCEDATAPRTR_H
#define SUBSTANCEDATAPRTR_H

#include <vector>
typedef std::vector<double> vd;

class SubstanceDataPrTr
{
public:
    SubstanceDataPrTr();

    vd getSV0() const;
    void setSV0(const vd &value);

    vd getSG0() const;
    void setSG0(const vd &value);

    vd getSH0() const;
    void setSH0(const vd &value);

    vd getSS0() const;
    void setSS0(const vd &value);

    vd getSaS0() const;
    void setSaS0(const vd &value);

    vd getSCp0() const;
    void setSCp0(const vd &value);

    double getReferenceTc() const;
    void setReferenceTc(const double &value);

    double getReferencePb() const;
    void setReferencePb(const double &value);

    bool getIsProcessed() const;
    void setIsProcessed(bool value);

private:
    vd sV0,              ///< Standard volume at Tst, Pst "V0" m3/mol; [1] ERROR
       sG0,              ///< Standard Gibbs energy at Tst, Pst  "GO" J/mol; [1] ERROR
       sH0,              ///< Standard enthalpy at Tst, Pst  "H0" J/mol; [1] ERROR
       sS0,              ///< Standard entropy of formation from elements at st.states "S0f" J/K/mol; [1] ERROR
       saS0,             ///< Standard absolute entropy at Tst, Pst "S0" J/K/mol;[1] ERROR
       sCp0;             ///< Standard constant-pressure heat capacity at Tst, Pst "Cp0" J/K/mol; [1] ERROR
    double ReferenceTc,                  ///< Reference temperature for standard state (default 25 C)
           ReferencePb;                  ///< Reference pressure (default 1 bar)
    bool isProcessed;           ///< indicator of processing the data

};

#endif // SUBSTANCEDATAPRTR_H
