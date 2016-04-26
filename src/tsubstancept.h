#ifndef TSUBSTANCEPT_H
#define TSUBSTANCEPT_H

#include <tcorrpt_global.h>
#include "substancedataprtr.h"
#include "substancedatapt.h"
#include "tcorrmodsubstance.h"
//#include "treactionpt.h"

class TSubstancePT
{
public:

    TSubstancePT();

//    void setTemperature(const double T);

    int getNelem() const;
    void setNelem(int value);

    double getCfCharge() const;
    void setCfCharge(double value);

    double getCmMass() const;
    void setCmMass(double value);

    double getBetaT() const;
    void setBetaT(double value);

    double getAlphaP() const;
    void setAlphaP(double value);

    std::string getSymbol() const;
    void setSymbol(const std::string &value);

    std::string getFormula() const;
    void setFormula(const std::string &value);

    MethodGenEoS_Thrift::type getMethod_genEoS() const;
    void setMethod_genEoS(const MethodGenEoS_Thrift::type &value);

    MethodCorrT_Thrift::type getMethod_CorrT() const;
    void setMethod_CorrT(const MethodCorrT_Thrift::type &value);

    MethodCorrP_Thrift::type getMethod_CorrP() const;
    void setMethod_CorrP(const MethodCorrP_Thrift::type &value);

    SubstanceClass::type getSubstance_class() const;
    void setSubstance_class(const SubstanceClass::type &value);

    AggregateState::type getAggregate_state() const;
    void setAggregate_state(const AggregateState::type &value);

    ConcentrationScales::type getStandard_cscale() const;
    void setStandard_cscale(const ConcentrationScales::type &value);

    SubstanceDataCalulationType::type getCalculation_type() const;
    void setCalculation_type(const SubstanceDataCalulationType::type &value);

    SubstanceDataPT *getDataAtPT() const;
    void setDataAtPT(SubstanceDataPT *value);

    SubstanceDataPrTr *getDataAtPrTr() const;
    void setDataAtPrTr(SubstanceDataPrTr *value);

//    void updateDataAtPT( ); /// updated automatically since there are pointers in TCorr object

    void makeTCorrPT(); ///< Initializes the models for P & T claculations

    ///< Sets the correction method types for the current substance
    void setMethods(MethodGenEoS_Thrift::type &genEOS, MethodCorrT_Thrift::type &CorrT, MethodCorrP_Thrift::type &CorrP);

    ///< Sets the parameters for each PT correaction method
    void setTCorrPTParam( );

    ///< Reacalulates the standard thermodynamic properties for the curent subtance
    void thermoCalc( ); // RecCalc in m_dcomp.h

//    void setThermoAtPrTr();

private:

    int Nelem;                                              ///< number of elements in the substance
    //    std::vector<ElementData>  elems;
    double cfCharge,        ///< Formula charge
           cmMass,          ///< Mass per 1 mole of this substance "mM" kg/mol
           betaT,           ///< Isothermal compressibility (for condensed substances)
           alphaP;          ///< Isobaric expansivity (for condensed substances)

//    int subsid;         ///<
    std::string Symbol;     ///< Name of the substance (chemical or mineral nomenclature)
    std::string Formula;    ///< Formula of substance in GEMS syntax, in elements, can include moiety symbols

    SubstanceDataPrTr *DataAtPrTr;    ///< structure holding thermodynamic data at reference temperature (Tr) and pressure (Pr)
    SubstanceDataPT *DataAtPT;                ///< structure holding thermodynamic data for current temperature (T) and pressure (P)

    MethodGenEoS_Thrift::type Method_genEoS;    ///< General method (or equation of state for both T and P correction)
    MethodCorrT_Thrift::type  Method_CorrT;     ///< Method of temperature correction of thermodynamic properties
    MethodCorrP_Thrift::type  Method_CorrP;     ///< Method of pressure correction of thermodynamic properties

    SubstanceClass::type Substance_class;
    AggregateState::type Aggregate_state;
    ConcentrationScales::type StdConcentration_scale;

    SubstanceDataCalulationType::type Calculation_type;

    void *Reaction;                      ///< The reaction which defines the properties of this substance (cast into ReactionData when used - "ReactionData *rd = reinterpret_cast<ReactionData*>(Reaction);")

    vector<TCorrModSubstance *> TCorr;   ///< vector of pointers to substance PT correction models
//    SubstanceData Sdata;

    void makeGenEoS();
    void makeCorrT();
    void makeCorrP();

    void setGenEoSParam();
    void setCorrTParam();
    void setCorrPParam();
};

#endif // TSUBSTANCEPT_H
