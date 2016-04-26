#ifndef TREACTIONPT_H
#define TREACTIONPT_H

#include <tsubstancept.h>
#include "reactiondataprtr.h"
#include "reactiondatapt.h"
#include "tcorrmodreaction.h"


class TReactionPT
{
public:

    TReactionPT();

    int getNdcomp() const;
    void setNdcomp(int value);

    std::string getSymbol() const;
    void setSymbol(const std::string &value);

    MethodGenEoS_Thrift::type getMethod_genEoS() const;
    void setMethod_genEoS(const MethodGenEoS_Thrift::type &value);

    MethodCorrT_Thrift::type getMethod_CorrT() const;
    void setMethod_CorrT(const MethodCorrT_Thrift::type &value);

    MethodCorrP_Thrift::type getMethod_CorrP() const;
    void setMethod_CorrP(const MethodCorrP_Thrift::type &value);

    std::vector<TSubstancePT> *getRComponents() const;
    void setRComponents(std::vector<TSubstancePT> *value);

    std::vector<int *> getRCoefficients() const;
    void setRCoefficients(const std::vector<int *> &value);

    std::vector<ReactionComponentType *> getRComponentTypes() const;
    void setRComponentTypes(const std::vector<ReactionComponentType *> &value);

    ReactionDataPrTr *getDataAtPrTr() const;
    void setDataAtPrTr(ReactionDataPrTr *value);

    ReactionDataPT *getDataAtPT() const;
    void setDataAtPT(ReactionDataPT *value);

private:

    int Ndcomp;                                             ///< number of DComps/substance involved in the reaction

    ReactionDataPrTr *DataAtPrTr;                             ///< structure holding thermodynamic data at reference temperature (Tr) and pressure (Pr)
    ReactionDataPT *DataAtPT;                                 ///< structure holding thermodynamic data for current temperature (T) and pressure (P)

    std::string Symbol;                                     ///< Unique name/ID of this reaction (e.g. product species name)

    MethodGenEoS_Thrift::type Method_genEoS;                ///< General method (or equation of state for both T and P correction)
    MethodCorrT_Thrift::type  Method_CorrT;                 ///< Method of temperature correction of thermodynamic properties
    MethodCorrP_Thrift::type  Method_CorrP;                 ///< Method of pressure correction of thermodynamic properties

    std::vector<TSubstancePT> *rComponents;                 ///< Vector of DComps / substances involved in the reaction
    std::vector<int *> rCoefficients;                         ///< Stoichiometry coefficients of DC in reaction (reagents < 0)
    std::vector<ReactionComponentType *> rComponentTypes;     ///< Classes (types) of reaction components

    vector<TCorrModReaction *> TCorr;                          ///< vector of pointers to reaction PT correction models

};

#endif // TREACTIONPT_H
