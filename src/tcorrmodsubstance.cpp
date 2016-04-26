#include "tcorrmodsubstance.h"

TCorrModSubstance::TCorrModSubstance(SubstanceDataPrTr *AtPrTr_, SubstanceDataPT *AtPT_)
{
    AtPrTr = AtPrTr_;
    AtPT = AtPT_;
}
vvd TCorrModSubstance::getTCint() const
{
    return TCinterval;
}

void TCorrModSubstance::setTCint(const vvd &value)
{
    TCinterval = value;
}
vvd TCorrModSubstance::getCp() const
{
    return CpCoeff;
}

void TCorrModSubstance::setCp(const vvd &value)
{
    CpCoeff = value;
}

