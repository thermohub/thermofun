#include "substancedataprtr.h"

SubstanceDataPrTr::SubstanceDataPrTr()
{
    ReferenceTc = 25;
    ReferencePb = 1;
    isProcessed = false;
    rV0 = {0,0};
    raS0 = {0,0};
    rCp0 = {0,0};
    rG0 = {0,0};
    rH0 = {0,0};
    rS0 = {0,0};
    rA0 = {0,0};
    rU0 = {0,0};
    rCv0 = {0,0};
}

vd SubstanceDataPrTr::getRV0() const
{
    return rV0;
}

void SubstanceDataPrTr::setRV0(const vd &value)
{
    rV0 = value;
}

vd SubstanceDataPrTr::getRG0() const
{
    return rG0;
}

void SubstanceDataPrTr::setRG0(const vd &value)
{
    rG0 = value;
}

vd SubstanceDataPrTr::getRH0() const
{
    return rH0;
}

void SubstanceDataPrTr::setRH0(const vd &value)
{
    rH0 = value;
}

vd SubstanceDataPrTr::getRS0() const
{
    return rS0;
}

void SubstanceDataPrTr::setRS0(const vd &value)
{
    rS0 = value;
}

vd SubstanceDataPrTr::getRaS0() const
{
    return raS0;
}

void SubstanceDataPrTr::setRaS0(const vd &value)
{
    raS0 = value;
}

vd SubstanceDataPrTr::getRCp0() const
{
    return rCp0;
}

void SubstanceDataPrTr::setRCp0(const vd &value)
{
    rCp0 = value;
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

