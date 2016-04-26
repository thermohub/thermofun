#ifndef TCORRMODSUBSTANCE_H
#define TCORRMODSUBSTANCE_H

#include "tcorrpt_global.h"
#include "substancedataprtr.h"
#include "substancedatapt.h"

class TCorrModSubstance
{
public:
    TCorrModSubstance(SubstanceDataPrTr *AtPrTr_, SubstanceDataPT *AtPT_ );

    virtual long int PTparam()
    {
        return 0;
    };

    vvd getTCint() const;
    void setTCint(const vvd &value);

    vvd getCp() const;
    void setCp(const vvd &value);

private:

protected:

    // all TCorrMod objectts work on the same AtPrTr and AtPT data objects pointing to the
    SubstanceDataPrTr *AtPrTr;
    SubstanceDataPT *AtPT;

    int Type, Subst_class_;

    SubstanceClass::type Subst_class;

    vvd TCinterval,     /// Lower and upper T limits for Cp=f(T) equation
        PBinterval,     /// Lower and upper P limits for pressure correction
        CpCoeff,        /// Coeffs of Cp=f(T) (J,mole,K), one column per equation
        CpECoeff,       /// Empirical coefficients for nonelectrolyte aqueous solutes in Akinfiev etc.
        FtP,            /// Column: TCf- at Pr; DltS,DltH,DltV; dV of phase transitions
        FtBer;          /// Properties of phase transition (Berman): Tr; Tft; tilt; l1,l2 (reserved)
     vd HKFCoeff,       /// Empirical coefficients of HKF EOS
        VCoeff,         /// Coefficients of mV=f(T,P)
        CritParam,      /// Critical parameters (for FGL)?
        VCoeff_ODc,     /// Coeffs of V(T,P) Birch-Murnaghan 1947 Gottschalk
        EmpiricalCoeff; /// Array of empirical EOS coefficients (CG EOS: MAXCGPARAM = 13)
};

// Clases for calculation of thermodynamic parameters for a substance //

/// integration of empirical heat capacity equation Cp=f(T);
class EmpCpIntegr: public TCorrModSubstance
{
public:

    /// Constructor
    EmpCpIntegr (  );
        EmpCpIntegr (SubstanceDataPrTr *AtPrTr_, SubstanceDataPT *AtPT_ );

        /// Calculates P, T corrected themrodynamic parameters
        long int PTparam();

private:

};

#endif // TCORRMODSUBSTANCE_H
