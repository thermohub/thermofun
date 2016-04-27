#ifndef REACTIONDATAPRTR_H
#define REACTIONDATAPRTR_H


#include <vector>
typedef std::vector<double> vd;

/// Themrodynamic data for reaction at reference pressure (P) and temperature (T)
class ReactionDataPrTr
{
public:
    ReactionDataPrTr();

    vd getLogK0() const;
    void setLogK0(const vd &value);

    vd getDrG0() const;
    void setDrG0(const vd &value);

    vd getDrH0() const;
    void setDrH0(const vd &value);

    vd getDrS0() const;
    void setDrS0(const vd &value);

    vd getDrCp0() const;
    void setDrCp0(const vd &value);

    vd getDrV0() const;
    void setDrV0(const vd &value);

    double getReferenceTc() const;
    void setReferenceTc(double value);

    double getReferencePb() const;
    void setReferencePb(double value);

    bool getIsProcessed() const;
    void setIsProcessed(bool value);

private:

    vd logK0,                                    ///< Standard equilibrium constant "logK0" (set in log10 units); [1] ERROR
       drG0,                                     ///< Standard Gibbs energy effect per mole  "drGO" J/mol; [1] ERROR
       drH0,                                     ///< Standard enthalpy effect per mole "drH0" J/mol; [1] ERROR
       drS0,                                     ///< Standard entropy effect of reaction "drS0" J/K/mol; [1] ERROR
       drCp0,                                    ///< Standard heat capacity effect per mole "drCp0" J/K/mol; [1] ERROR
       drV0,                                     ///< Standard volume effect per mole "drV0" m3/mol; [1] ERROR
       drA0,                                     ///< Standard Helmholtz energy of reaction at reference conditions J/mol
       drU0,                                     ///< Standard internal energy of reaction at reference conditions J/mol
       drCv0;                                    ///< Standard Isochoric heat capacity of reaction J/(mol*K)
    double ReferenceTc,                                          ///< Reference temperature for standard state (default 25 C)
           ReferencePb;                                          ///< Reference pressure (default 1 bar)
    bool isProcessed;                                   ///< indicator of processing the data

};

#endif // REACTIONDATAPRTR_H
