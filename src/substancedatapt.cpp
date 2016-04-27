#include "substancedatapt.h"

SubstanceDataPT::SubstanceDataPT()
{
    CurrentPb = 0;
    CurrentTc = 0;
    isProcessed = false;
    G0 = {0,0};
    S0 = {0,0};
    H0 = {0,0};
    V0 = {0,0};
    Cp0 = {0,0};
    A0 = {0,0};
    U0 = {0,0};
    Cv0 = {0,0};
    Fug = {0,0};
    dVg = {0,0};
}

double SubstanceDataPT::getCurrentPb() const
{
    return CurrentPb;
}

void SubstanceDataPT::setCurrentPb(double value)
{
    CurrentPb = value;
}

double SubstanceDataPT::getCurrentTc() const
{
    return CurrentTc;
}

void SubstanceDataPT::setCurrentTc(double value)
{
    CurrentTc = value;
}

vd SubstanceDataPT::getG0() const
{
    return G0;
}

void SubstanceDataPT::setG0(const vd &value)
{
    G0 = value;
}

vd SubstanceDataPT::getH0() const
{
    return H0;
}

void SubstanceDataPT::setH0(const vd &value)
{
    H0 = value;
}

vd SubstanceDataPT::getS0() const
{
    return S0;
}

void SubstanceDataPT::setS0(const vd &value)
{
    S0 = value;
}

vd SubstanceDataPT::getV0() const
{
    return V0;
}

void SubstanceDataPT::setV0(const vd &value)
{
    V0 = value;
}

vd SubstanceDataPT::getCp0() const
{
    return Cp0;
}

void SubstanceDataPT::setCp0(const vd &value)
{
    Cp0 = value;
}

vd SubstanceDataPT::getFug() const
{
    return Fug;
}

void SubstanceDataPT::setFug(const vd &value)
{
    Fug = value;
}

vd SubstanceDataPT::getDVg() const
{
    return dVg;
}

void SubstanceDataPT::setDVg(const vd &value)
{
    dVg = value;
}

bool SubstanceDataPT::getIsProcessed() const
{
    return isProcessed;
}

void SubstanceDataPT::setIsProcessed(bool value)
{
    isProcessed = value;
}

