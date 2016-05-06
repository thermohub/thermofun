#include "tsubstancept.h"

TSubstancePT::TSubstancePT()
{
    // default values
    DataAtPrTr = new SubstanceDataPrTr;
    DataAtPT = new SubstanceDataPT;

    Nelem = 0;
    cfCharge = 0;
    cmMass = 0;
    betaT = 0;
    alphaP = 0;
    Symbol = "NULL";
    Formula = "NULL";
    Method_genEoS = MethodGenEoS_Thrift::CEM_OFF;
    Method_CorrP = MethodCorrP_Thrift::CPM_OFF;
    Method_CorrT = MethodCorrT_Thrift::CTM_DAS;

    StdConcentration_scale = ConcentrationScales::C_OTHER;
    Substance_class = SubstanceClass::OTHER_SC;
    Aggregate_state = AggregateState::OTHER_AS;
    Calculation_type = SubstanceThermoCalculationType::DCOMP;
}

int TSubstancePT::getNelem() const
{
    return Nelem;
}

void TSubstancePT::setNelem(int value)
{
    Nelem = value;
}

double TSubstancePT::getCfCharge() const
{
    return cfCharge;
}

void TSubstancePT::setCfCharge(double value)
{
    cfCharge = value;
}

double TSubstancePT::getCmMass() const
{
    return cmMass;
}

void TSubstancePT::setCmMass(double value)
{
    cmMass = value;
}

double TSubstancePT::getBetaT() const
{
    return betaT;
}

void TSubstancePT::setBetaT(double value)
{
    betaT = value;
}

double TSubstancePT::getAlphaP() const
{
    return alphaP;
}

void TSubstancePT::setAlphaP(double value)
{
    alphaP = value;
}

std::string TSubstancePT::getSymbol() const
{
    return Symbol;
}

void TSubstancePT::setSymbol(const std::string &value)
{
    Symbol = value;
}

std::string TSubstancePT::getFormula() const
{
    return Formula;
}

void TSubstancePT::setFormula(const std::string &value)
{
    Formula = value;
}

MethodGenEoS_Thrift::type TSubstancePT::getMethod_genEoS() const
{
    return Method_genEoS;
}

void TSubstancePT::setMethod_genEoS(const MethodGenEoS_Thrift::type &value)
{
    Method_genEoS = value;
}

MethodCorrT_Thrift::type TSubstancePT::getMethod_CorrT() const
{
    return Method_CorrT;
}

void TSubstancePT::setMethod_CorrT(const MethodCorrT_Thrift::type &value)
{
    Method_CorrT = value;
}

MethodCorrP_Thrift::type TSubstancePT::getMethod_CorrP() const
{
    return Method_CorrP;
}

void TSubstancePT::setMethod_CorrP(const MethodCorrP_Thrift::type &value)
{
    Method_CorrP = value;
}

SubstanceClass::type TSubstancePT::getSubstance_class() const
{
    return Substance_class;
}

void TSubstancePT::setSubstance_class(const SubstanceClass::type &value)
{
    Substance_class = value;
}

AggregateState::type TSubstancePT::getAggregate_state() const
{
    return Aggregate_state;
}

void TSubstancePT::setAggregate_state(const AggregateState::type &value)
{
    Aggregate_state = value;
}

ConcentrationScales::type TSubstancePT::getStandard_cscale() const
{
    return StdConcentration_scale;
}

void TSubstancePT::setStandard_cscale(const ConcentrationScales::type &value)
{
    StdConcentration_scale = value;
}

SubstanceThermoCalculationType::type TSubstancePT::getCalculation_type() const
{
    return Calculation_type;
}

void TSubstancePT::setCalculation_type(const SubstanceThermoCalculationType::type &value)
{
    Calculation_type = value;
}

SubstanceDataPT *TSubstancePT::getDataAtPT() const
{
    return DataAtPT;
}

void TSubstancePT::setDataAtPT(SubstanceDataPT *value)
{
    DataAtPT = value;
}

SubstanceDataPrTr *TSubstancePT::getDataAtPrTr() const
{
    return DataAtPrTr;
}

void TSubstancePT::setDataAtPrTr(SubstanceDataPrTr *value)
{
    DataAtPrTr = value;
}

void TSubstancePT::makeTCorrPT( )
{
    makeGenEoS( );
    makeCorrT( );
    makeCorrP( );
}

void TSubstancePT::setMethods(MethodGenEoS_Thrift::type &genEOS, MethodCorrT_Thrift::type &CorrT, MethodCorrP_Thrift::type &CorrP)
{
    Method_genEoS = genEOS;
    Method_CorrP = CorrP;
    Method_CorrT = CorrT;
}

void TSubstancePT::setTCorrPTParam()
{
    setGenEoSParam( );
    setCorrTParam( );
    setCorrPParam( );
}

void TSubstancePT::thermoCalc()
{


//    DCthermo( 0/*q*/, 0 ); // calc thermodynamic data using the thermo correction but at std conditions
}

vector<TCorrModSubstance *> TSubstancePT::getTCorr() const
{
    return TCorr;
}

void TSubstancePT::setTCorr(const vector<TCorrModSubstance *> &value)
{
    TCorr = value;
}

TCorrModSubstance *TSubstancePT::getTCorr( int ndx ) const
{
    return TCorr[ndx];
}

void TSubstancePT::setTCorr(int ndx, TCorrModSubstance *value )
{
    TCorr[ndx] = value;
}


void TSubstancePT::makeGenEoS( )
{
    TCorrModSubstance* myCORR = 0;

    switch( Method_genEoS )
    {
    case MethodGenEoS_Thrift::type::CTPM_CPT:
    {
        EmpCpIntegr* myPT = new EmpCpIntegr( DataAtPrTr, DataAtPT );
            myCORR = (EmpCpIntegr*)myPT;
            if(myCORR) { TCorr.push_back( myCORR ); myCORR = 0; };
            break;
        }
    }

}

void TSubstancePT::setGenEoSParam( )
{
//    switch( method_genEoS )
//    {
//        case MethodGenEoS_Thrift::type::CTPM_CPT:
//        {

//        }
//    }
}

void TSubstancePT::makeCorrP(  )
{

}

void TSubstancePT::makeCorrT( )
{

}

void TSubstancePT::setCorrTParam( )
{

}

void TSubstancePT::setCorrPParam( )
{

}


