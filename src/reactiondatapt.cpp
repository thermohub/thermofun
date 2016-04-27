#include "reactiondatapt.h"

ReactionDataPT::ReactionDataPT()
{
    CurrentPb = 0;
    CurrentTc = 0;
    K = {0,0};
    logK = {0,0};
    dH = {0,0};
    dCp = {0,0};
    dS = {0,0};
    dV = {0,0};
    dA = {0,0};
    dU = {0,0};
    dCv = {0,0};
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

vd ReactionDataPT::getDH() const
{
    return dH;
}

void ReactionDataPT::setDH(const vd &value)
{
    dH = value;
}

vd ReactionDataPT::getDS() const
{
    return dS;
}

void ReactionDataPT::setDS(const vd &value)
{
    dS = value;
}

vd ReactionDataPT::getDCp() const
{
    return dCp;
}

void ReactionDataPT::setDCp(const vd &value)
{
    dCp = value;
}

vd ReactionDataPT::getDV() const
{
    return dV;
}

void ReactionDataPT::setDV(const vd &value)
{
    dV = value;
}

bool ReactionDataPT::getIsProcessed() const
{
    return isProcessed;
}

void ReactionDataPT::setIsProcessed(bool value)
{
    isProcessed = value;
}

