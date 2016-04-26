#include "substancedataprtr.h"

SubstanceDataPrTr::SubstanceDataPrTr()
{
    ReferenceTc = 25;
    ReferencePb = 1;
    isProcessed = false;
    sV0 = {0,0};
    saS0 = {0,0};
    sCp0 = {0,0};
    sG0 = {0,0};
    sH0 = {0,0};
    sS0 = {0,0};
}

vd SubstanceDataPrTr::getSV0() const
{
    return sV0;
}

void SubstanceDataPrTr::setSV0(const vd &value)
{
    sV0 = value;
}

vd SubstanceDataPrTr::getSG0() const
{
    return sG0;
}

void SubstanceDataPrTr::setSG0(const vd &value)
{
    sG0 = value;
}

vd SubstanceDataPrTr::getSH0() const
{
    return sH0;
}

void SubstanceDataPrTr::setSH0(const vd &value)
{
    sH0 = value;
}

vd SubstanceDataPrTr::getSS0() const
{
    return sS0;
}

void SubstanceDataPrTr::setSS0(const vd &value)
{
    sS0 = value;
}

vd SubstanceDataPrTr::getSaS0() const
{
    return saS0;
}

void SubstanceDataPrTr::setSaS0(const vd &value)
{
    saS0 = value;
}

vd SubstanceDataPrTr::getSCp0() const
{
    return sCp0;
}

void SubstanceDataPrTr::setSCp0(const vd &value)
{
    sCp0 = value;
}

double SubstanceDataPrTr::getReferenceTc() const
{
    return ReferenceTc;
}

void SubstanceDataPrTr::setReferenceTc(const double &value)
{
    ReferenceTc = value;
}

double SubstanceDataPrTr::getReferencePb() const
{
    return ReferencePb;
}

void SubstanceDataPrTr::setReferencePb(const double &value)
{
    ReferencePb = value;
}

bool SubstanceDataPrTr::getIsProcessed() const
{
    return isProcessed;
}

void SubstanceDataPrTr::setIsProcessed(bool value)
{
    isProcessed = value;
}

