#ifndef SUBSTANCEDATAPT_H
#define SUBSTANCEDATAPT_H

#include <vector>
typedef std::vector<double> vd;

class SubstanceDataPT
{
public:
    SubstanceDataPT();

    double getCurrentPb() const;
    void setCurrentPb(double value);

    double getCurrentTc() const;
    void setCurrentTc(double value);

    vd getG0() const;
    void setG0(const vd &value);

    vd getH0() const;
    void setH0(const vd &value);

    vd getS0() const;
    void setS0(const vd &value);

    vd getV0() const;
    void setV0(const vd &value);

    vd getCp0() const;
    void setCp0(const vd &value);

    vd getFug() const;
    void setFug(const vd &value);

    vd getDVg() const;
    void setDVg(const vd &value);

    bool getIsProcessed() const;
    void setIsProcessed(bool value);

private:
    double CurrentPb,                    ///< current pressure, bar (10^5 Pa)
           CurrentTc;                    ///< current temperature, Celsius
        vd V0,              ///< Standard volume at Tst, Pst "V0" m3/mol; [1] ERROR
           G0,              ///< Standard Gibbs energy at Tst, Pst  "GO" J/mol; [1] ERROR
           H0,              ///< Standard enthalpy at Tst, Pst  "H0" J/mol; [1] ERROR
           S0,              ///< Standard entropy of formation from elements at st.states "S0f" J/K/mol; [1] ERROR
           aS0,             ///< Standard absolute entropy at Tst, Pst "S0" J/K/mol;[1] ERROR
           Cp0,             ///< Standard constant-pressure heat capacity at Tst, Pst "Cp0" J/K/mol; [1] ERROR
           A0,              ///< Standard Helmholtz energy at reference conditions J/mol
           U0,              ///< Standard internal energy at reference conditions J/mol
           Cv0,             ///< Standard Isochoric heat capacity at reference conditions J/(mol*K)
           Fug, dVg;        ///< molar fugacity at TP (reserved), dVm of gas (reserved); [1] ERROR
    bool isProcessed;       ///< indicator of processing the data

};

#endif // SUBSTANCEDATAPT_H
