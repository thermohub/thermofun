#ifndef REACTIONDATAPT_H
#define REACTIONDATAPT_H

#include <vector>
typedef std::vector<double> vd;

class ReactionDataPT
{
public:
    ReactionDataPT();

    double getCurrentPb() const;
    void setCurrentPb(double value);

    double getCurrentTc() const;
    void setCurrentTc(double value);

    vd getK() const;
    void setK(const vd &value);

    vd getLogK() const;
    void setLogK(const vd &value);

    vd getDHr() const;
    void setDHr(const vd &value);

    vd getDSr() const;
    void setDSr(const vd &value);

    vd getDCpr() const;
    void setDCpr(const vd &value);

    vd getDVr() const;
    void setDVr(const vd &value);

    bool getIsProcessed() const;
    void setIsProcessed(bool value);

private:

    double CurrentPb,                                    ///< current pressure, bar (10^5 Pa)
           CurrentTc;                                    ///< current temperature, Celsius??
        vd K, logK,                               ///< equilibrium constant for TP, logK of reac, uncertainty; [1] ERROR
           dHr,                                      ///< dHr at TP, hT0  partial molal enthalpy, uncertainty; [1] ERROR
           dSr,                                     ///< dSr at TP, partial molal entropy of reaction
           dCpr,                                     ///< dCpr at TP, Cp0  partial molar heat capacity,uncertainty; [1] ERROR
           dVr;                                      ///< dVr at TP,Vm0  partial molar volume at TP,uncertainty; [1] ERROR
    bool isProcessed;                                   ///< indicator of processing the data

};

#endif // REACTIONDATAPT_H
