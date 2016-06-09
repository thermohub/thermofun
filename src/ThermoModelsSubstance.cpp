#include "ThermoModelsSubstance.h"
#include "Solute/SoluteHKFreaktoro.h"
#include "Solute/SoluteHKFgems.h"

// TCorrPT includes
#include "Common/Exception.h"

namespace TCorrPT {

struct ThermoModelsSubstance::Impl
{
    /// The substance instance
    Substance substance;

    Impl()
    {}

    Impl(const Substance& substance)
    : substance(substance)
    {}
};

ThermoModelsSubstance::ThermoModelsSubstance(const Substance& substance)
: pimpl(new Impl(substance))
{}

auto ThermoModelsSubstance::thermoProperties(double T, double P) -> ThermoPropertiesSubstance
{
    MethodGenEoS_Thrift::type method_genEOS = pimpl->substance.methodGenEOS();

    switch( method_genEOS )
    {
        case MethodGenEoS_Thrift::type::CTPM_CPT:
        {
            EmpiricalCpIntegration CpInt ( pimpl->substance);
            return CpInt.thermoProperties(T, P);
            break;
        }
    }

    // Exception
    Exception exception;
    exception.error << "The calculation method was not found.";
    exception.reason << "The calculation method defined for the substance "<< pimpl->substance.name() << " is not available.";
    exception.line = __LINE__;
    RaiseError(exception);
}


//=======================================================================================================
// HKF equation of state (as implmeneted in GEMS);
// References:
// Added: DM 07.06.2016
//=======================================================================================================

struct SoluteHKFgems::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

SoluteHKFgems::SoluteHKFgems(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto SoluteHKFgems::thermoProperties(double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

//    checkTemperatureValidityHKF(t, p, pimpl->substance);

    FunctionG g = gShok2(t, p, wp);

    ElectroPropertiesSubstance aes = omeg92(g, pimpl->substance);

    return thermoPropertiesAqSoluteHKFgems(t, p, pimpl->substance, aes, wes);
}

//=======================================================================================================
// HKF equation of state (as implmeneted in reaktoro);
// References:
// Added: DM 07.06.2016
//=======================================================================================================

struct SoluteHKFreaktoro::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

SoluteHKFreaktoro::SoluteHKFreaktoro(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto SoluteHKFreaktoro::thermoProperties(double T, double P, PropertiesSolvent wp, ElectroPropertiesSolvent wes) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

//    checkTemperatureValidityHKF(t, p, pimpl->substance);

    FunctionG g = functionG(t, p, wp);

    ElectroPropertiesSubstance aes = speciesElectroStateHKF(g, pimpl->substance);

    return speciesThermoStateSoluteHKF(t, p, pimpl->substance, aes, wes);
}

//=======================================================================================================
// Integration of empirical heat capacity equation Cp=f(T);
// References:
// Added: DM 26.04.2016
//=======================================================================================================

struct EmpiricalCpIntegration::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

EmpiricalCpIntegration::EmpiricalCpIntegration(const Substance &substance)
: pimpl(new Impl(substance))
{}

// calculation
auto EmpiricalCpIntegration::thermoProperties(double T, double P) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance   thermo_properties_PT = pimpl->substance.thermoProperties();
    ThermoPropertiesSubstance   thermo_properties_PrTr = pimpl->substance.thermoReferenceProperties();
    SubstanceClass::type        substance_class = pimpl->substance.substanceClass();
    ThermoParametersSubstance   thermo_parameters = pimpl->substance.thermoParameters();

    double TK, TC, TrK, T_Tst, Ts2, TT, T2, T3, T4, T05,
           Tst2, Tst3, Tst4, Tst05,
           Pb;
    double    S, G, H, V, Cp;
    int k=-1;
    vector<double> ac;
    ac.resize(16);
    ac = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

    TC = T;              // get current T in Celsius
    TK = TC + C_to_K;                       // curent T in Kelvin
    Pb = P;              // current P in bar
    TrK = pimpl->substance.referenceT() + C_to_K;

    S = thermo_properties_PrTr.entropy.val;
    G = thermo_properties_PrTr.gibbs_energy.val;
    H = thermo_properties_PrTr.enthalpy.val;
    V = 0; // ???
    Cp = 0;

    // P correction - has to be moved from here!!!
    if(( substance_class == SubstanceClass::type::GASFLUID /*|| dc[q].pstate[0] == CP_GASI*/ )
            && Pb > 0.0 )
    { // molar volume from the ideal gas law
        V = TK / Pb * R_CONSTANT;
    }

//    aW.twp->devG = dc[q].Gs[1];
//    ??????????????????????????????????????'
//    if( !dc[q].Cp )
//    {
//        aW.twp->Cp = (double)dc[q].Cps[0];
//        goto NEXT;
//    }

    // get Cp interval
    for (unsigned i=0; i<thermo_parameters.temperature_intervals.size(); i++)
    {
       if ((thermo_parameters.temperature_intervals[i][0] <= TC) && (thermo_parameters.temperature_intervals[i][1] > TC))
       {
           k = i;
           break;
       }
    }

    if (k<0)
    {
        Exception exception;
        exception.error << "The given temperature: "<< T <<" is not inside the specified interval/s for the Cp calculation.";
        exception.reason << "The temperature is not inside the specified interval for the substance "<< pimpl->substance.name() << ".";
        exception.line = __LINE__;
        RaiseError(exception);
    }

    T2 = TK * TK;
    T3 = T2 * TK;
    T4 = T3 * TK;
    T05 = std::sqrt( TK );

    for (unsigned i=0; i<thermo_parameters.Cp_coeff.size(); i++)
    {
        ac[i] = thermo_parameters.Cp_coeff[k][i];
    }

//    ac = thermo_parameters.Cp_coeff[k];

    Cp = ( ac[0] + ac[1]*TK + ac[2]/T2 + ac[3]/T05 + ac[4]*T2
          + ac[5]*T3 + ac[6]*T4 + ac[7]/T3 + ac[8]/TK + ac[9]*T05 /*+ ac[10]*log(T)*/);

    // Phase transitions
    if (fabs(TK-TrK) > TEMPER_PREC)
    {
        for (unsigned j=0; j<=k; j++)
        {
            if ( j == k )
                TK = T + C_to_K;     // current T is the end T for phase transition Cp calculations
            else TK = thermo_parameters.temperature_intervals[j][1] + C_to_K;        // takes the upper bound from the j-th Tinterval
            T2 = TK * TK;
            T3 = T2 * TK;
            T4 = T3 * TK;
            T05 = sqrt( TK );
            if( !j )
                TrK = pimpl->substance.referenceT() + C_to_K;            // if j=0 the first interval should contain the reference T (Tcr)
            else  TrK = thermo_parameters.temperature_intervals[j][0] + C_to_K;    // if j>0 then we are in a different Tinterval and the reference T becomes the lower bound of the interval
            Tst2 = TrK * TrK;
            Tst3 = Tst2 * TrK;
            Tst4 = Tst3 * TrK;
            Tst05 = sqrt( TrK );
            T_Tst = TK - TrK;
            Ts2 = T_Tst * T_Tst;
            TT = TK / TrK;

            // going trough the phase transitions parameters in FtP
            for (unsigned ft = 0; ft < thermo_parameters.phase_transition_prop.size(); ft++)
            if ( j && thermo_parameters.phase_transition_prop[ft][0] <= TrK-C_to_K )
            {   // Adding parameters of phase transition
                if ( thermo_parameters.phase_transition_prop[ft].size() > 1 )  // dS
                    S += thermo_parameters.phase_transition_prop[ft][1];
                if ( thermo_parameters.phase_transition_prop[ft].size() > 2 )  // dH
                    H += thermo_parameters.phase_transition_prop[ft][2];
                if ( thermo_parameters.phase_transition_prop[ft].size() > 3 )  // dV
                    V += thermo_parameters.phase_transition_prop[ft][3];
                // More to be added ?
            }

            G -= S * T_Tst;

            for (unsigned i=0; i<thermo_parameters.Cp_coeff[j].size(); i++)
            {
                ac[i] = thermo_parameters.Cp_coeff[j][i];
            }

            S += ( ac[0] * log( TT ) + ac[1] * T_Tst + ac[2] * ( 1./Tst2 - 1./T2 ) / 2.
                 + ac[3] * 2. * ( 1./Tst05 - 1./T05 ) + ac[4] * ( T2 - Tst2 ) / 2.
                 + ac[5] * ( T3 - Tst3 ) / 3. + ac[6] * ( T4 - Tst4 ) / 4.
                 + ac[7] * ( 1./ Tst3 - 1./ T3 ) / 3. + ac[8] * (1. / TrK - 1. / TK )
                 + ac[9] * 2.* ( T05 - Tst05 ) );

            G -= ( ac[0] * ( TK * log( TT ) - T_Tst ) + ac[1] * Ts2 / 2. + ac[2] * Ts2 / TK / Tst2 / 2.
                 + ac[3] * 2. * (T05 - Tst05)*(T05 - Tst05) / Tst05 + ac[4] * ( T3 + 2.*Tst3 - 3.* TK * Tst2 ) / 6.
                 + ac[5] * ( T4 + 3.*Tst4 - 4.*TK * Tst3 ) / 12. + ac[6] * ( T4*TK + 4.*Tst4*TrK - 5.*TK*Tst4 ) / 20.
                 + ac[7] * ( Tst3 - 3.* T2 * TrK + 2.*T3 ) / 6./ T2 / Tst3 + ac[8] * ( TT - 1. - log( TT ))
                 + ac[9] * 2.* ( 2.* TK * T05 - 3.* TK * Tst05 + TrK * Tst05 ) / 3. );

            H += ( ac[0] * T_Tst + ac[1] * ( T2 - Tst2 ) / 2. + ac[2] * ( 1./TrK - 1./TK )
                 + ac[3] * 2. * ( T05 - Tst05 ) + ac[4] * ( T3 - Tst3 ) / 3.
                 + ac[5] * ( T4 - Tst4 ) / 4. + ac[6] * ( T4 * TK - Tst4 * TrK ) / 5
                 + ac[7] * ( 1./ Tst2 - 1./ T2 ) / 2. + ac[8] * log( TT )
                 + ac[9] * 2.* ( TK * T05 - TrK * Tst05 ) / 3. );
        }
    }

    thermo_properties_PT.heat_capacity_cp   = Cp;
    thermo_properties_PT.gibbs_energy       = G;
    thermo_properties_PT.enthalpy           = H;
    thermo_properties_PT.entropy            = S;
    thermo_properties_PT.volume             = V;

    return thermo_properties_PT;
}


} // namespace TCorrPT
