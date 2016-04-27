#include "reactiondataprtr.h"

ReactionDataPrTr::ReactionDataPrTr()
{
    logK0 = {0,0};
    drG0 = {0,0};
    drH0 = {0,0};
    drS0 = {0,0};
    drCp0 = {0,0};
    drV0 = {0,0};
    drA0 = {0,0};
    drU0 = {0,0};
    drCv0 = {0,0};
    ReferenceTc = 25;
    ReferencePb = 1;
    isProcessed = false;
}

vd ReactionDataPrTr::getLogK0() const
{
    return logK0;
}

void ReactionDataPrTr::setLogK0(const vd &value)
{
    logK0 = value;
}

vd ReactionDataPrTr::getDrG0() const
{
    return drG0;
}

void ReactionDataPrTr::setDrG0(const vd &value)
{
    drG0 = value;
}

vd ReactionDataPrTr::getDrH0() const
{
    return drH0;
}

void ReactionDataPrTr::setDrH0(const vd &value)
{
    drH0 = value;
}

vd ReactionDataPrTr::getDrS0() const
{
    return drS0;
}

void ReactionDataPrTr::setDrS0(const vd &value)
{
    drS0 = value;
}

vd ReactionDataPrTr::getDrCp0() const
{
    return drCp0;
}

void ReactionDataPrTr::setDrCp0(const vd &value)
{
    drCp0 = value;
}

vd ReactionDataPrTr::getDrV0() const
{
    return drV0;
}

void ReactionDataPrTr::setDrV0(const vd &value)
{
    drV0 = value;
}

double ReactionDataPrTr::getReferenceTc() const
{
    return ReferenceTc;
}

void ReactionDataPrTr::setReferenceTc(double value)
{
    ReferenceTc = value;
}

double ReactionDataPrTr::getReferencePb() const
{
    return ReferencePb;
}

void ReactionDataPrTr::setReferencePb(double value)
{
    ReferencePb = value;
}

bool ReactionDataPrTr::getIsProcessed() const
{
    return isProcessed;
}

void ReactionDataPrTr::setIsProcessed(bool value)
{
    isProcessed = value;
}

