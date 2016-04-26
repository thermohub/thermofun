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
        vd G0, H0, S0,         ///< partial molal gibbs energy, enthalpy, entropy at current TP; [1] ERROR
           V0, Cp0,               ///< partial molal volume, heat capacity at current TP; [1] ERROR
           Fug, dVg;              ///< molar fugacity at TP (reserved), dVm of gas (reserved); [1] ERROR
    bool isProcessed;                   ///< indicator of processing the data

};

#endif // SUBSTANCEDATAPT_H
