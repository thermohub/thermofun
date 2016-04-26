#include "reactiondatapt.h"

ReactionDataPT::ReactionDataPT()
{
    CurrentPb = 0;
    CurrentTc = 0;
    K = {0,0};
    logK = {0,0};
    dHr = {0,0};
    dCpr = {0,0};
    dSr = {0,0};
    dVr = {0,0};
    isProcessed = false;
}

double ReactionDataPT::getCurrentPb() const
{
    return CurrentPb;
}

void ReactionDataPT::setCurrentPb(double value)
{
    CurrentPb = value;
}

double ReactionDataPT::getCurrentTc() const
{
    return CurrentTc;
}

void ReactionDataPT::setCurrentTc(double value)
{
    CurrentTc = value;
}

vd ReactionDataPT::getK() const
{
    return K;
}

void ReactionDataPT::setK(const vd &value)
{
    K = value;
}

vd ReactionDataPT::getLogK() const
{
    return logK;
}

void ReactionDataPT::setLogK(const vd &value)
{
    logK = value;
}

vd ReactionDataPT::getDHr() const
{
    return dHr;
}

void ReactionDataPT::setDHr(const vd &value)
{
    dHr = value;
}

vd ReactionDataPT::getDSr() const
{
    return dSr;
}

void ReactionDataPT::setDSr(const vd &value)
{
    dSr = value;
}

vd ReactionDataPT::getDCpr() const
{
    return dCpr;
}

void ReactionDataPT::setDCpr(const vd &value)
{
    dCpr = value;
}

vd ReactionDataPT::getDVr() const
{
    return dVr;
}

void ReactionDataPT::setDVr(const vd &value)
{
    dVr = value;
}

bool ReactionDataPT::getIsProcessed() const
{
    return isProcessed;
}

void ReactionDataPT::setIsProcessed(bool value)
{
    isProcessed = value;
}

