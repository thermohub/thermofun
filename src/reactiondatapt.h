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

    vd getDH() const;
    void setDH(const vd &value);

    vd getDS() const;
    void setDS(const vd &value);

    vd getDCp() const;
    void setDCp(const vd &value);

    vd getDV() const;
    void setDV(const vd &value);

    bool getIsProcessed() const;
    void setIsProcessed(bool value);

private:

    double CurrentPb,                                    ///< current pressure, bar (10^5 Pa)
           CurrentTc;                                    ///< current temperature, Celsius??
        vd K, logK,                               ///< equilibrium constant for TP, logK of reac, uncertainty; [1] ERROR
           dH,                                      ///< dHr at TP, hT0  partial molal enthalpy, uncertainty; [1] ERROR
           dS,                                     ///< dSr at TP, partial molal entropy of reaction
           dCp,                                     ///< dCpr at TP, Cp0  partial molar heat capacity,uncertainty; [1] ERROR
           dV,                                      ///< dVr at TP,Vm0  partial molar volume at TP,uncertainty; [1] ERROR
           dA,                                     ///< Standard Helmholtz energy of reaction at current conditions J/mol
           dU,                                     ///< Standard internal energy of reaction at current conditions J/mol
           dCv;                                    ///< Standard Isochoric heat capacity of reaction at current conditions J/(mol*K)
    bool isProcessed;                                   ///< indicator of processing the data
};

#endif // REACTIONDATAPT_H
