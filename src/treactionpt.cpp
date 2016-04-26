#include "treactionpt.h"

TReactionPT::TReactionPT()
{
    DataAtPrTr =  new ReactionDataPrTr();
    DataAtPT = new ReactionDataPT;

    Ndcomp = 0;
    Symbol = "NULL";
    Method_genEoS = MethodGenEoS_Thrift::CEM_OFF;
    Method_CorrP = MethodCorrP_Thrift::CPM_OFF;
    Method_CorrT = MethodCorrT_Thrift::CTM_EK0;
}

int TReactionPT::getNdcomp() const
{
    return Ndcomp;
}

void TReactionPT::setNdcomp(int value)
{
    Ndcomp = value;
}

std::string TReactionPT::getSymbol() const
{
    return Symbol;
}

void TReactionPT::setSymbol(const std::string &value)
{
    Symbol = value;
}

MethodGenEoS_Thrift::type TReactionPT::getMethod_genEoS() const
{
    return Method_genEoS;
}

void TReactionPT::setMethod_genEoS(const MethodGenEoS_Thrift::type &value)
{
    Method_genEoS = value;
}

MethodCorrT_Thrift::type TReactionPT::getMethod_CorrT() const
{
    return Method_CorrT;
}

void TReactionPT::setMethod_CorrT(const MethodCorrT_Thrift::type &value)
{
    Method_CorrT = value;
}

MethodCorrP_Thrift::type TReactionPT::getMethod_CorrP() const
{
    return Method_CorrP;
}

void TReactionPT::setMethod_CorrP(const MethodCorrP_Thrift::type &value)
{
    Method_CorrP = value;
}

std::vector<TSubstancePT> *TReactionPT::getRComponents() const
{
    return rComponents;
}

void TReactionPT::setRComponents(std::vector<TSubstancePT> *value)
{
    rComponents = value;
}

std::vector<int *> TReactionPT::getRCoefficients() const
{
    return rCoefficients;
}

void TReactionPT::setRCoefficients(const std::vector<int *> &value)
{
    rCoefficients = value;
}

std::vector<ReactionComponentType *> TReactionPT::getRComponentTypes() const
{
    return rComponentTypes;
}

void TReactionPT::setRComponentTypes(const std::vector<ReactionComponentType *> &value)
{
    rComponentTypes = value;
}

ReactionDataPrTr *TReactionPT::getDataAtPrTr() const
{
    return DataAtPrTr;
}

void TReactionPT::setDataAtPrTr(ReactionDataPrTr *value)
{
    DataAtPrTr = value;
}

ReactionDataPT *TReactionPT::getDataAtPT() const
{
    return DataAtPT;
}

void TReactionPT::setDataAtPT(ReactionDataPT *value)
{
    DataAtPT = value;
}


