#include "EmpiricalCpIntegration.h"
#include "Common/Exception.h"

namespace TCorrPT {

auto thermoPropertiesEmpCpIntegration(Reaktoro_::Temperature T, Reaktoro_::Pressure P, Substance substance) -> ThermoPropertiesSubstance
{
    ThermoPropertiesSubstance   thermo_properties_PT   = substance.thermoProperties();
    ThermoPropertiesSubstance   thermo_properties_PrTr = substance.thermoReferenceProperties();
    SubstanceClass::type        substance_class        = substance.substanceClass();
    ThermoParametersSubstance   thermo_parameters      = substance.thermoParameters();

    Reaktoro_::ThermoScalar V;
    int k=-1;
    vector<double> ac;
    ac.resize(16);
    ac = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};


    auto TK = Reaktoro_::Temperature(T.val /*+ C_to_K*/);
    auto Pb = Reaktoro_::Pressure(P);

    auto TrK = substance.referenceT()/* + C_to_K*/;

    auto S = thermo_properties_PrTr.entropy;
    auto G = thermo_properties_PrTr.gibbs_energy;
    auto H = thermo_properties_PrTr.enthalpy;

    if (thermo_parameters.Cp_coeff.size() == 0)
    {
        errorModelParameters("Cp empirical coefficients", "empicrical Cp integration", __LINE__);
    }

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
       if ((thermo_parameters.temperature_intervals[i][0] <= TK) && (thermo_parameters.temperature_intervals[i][1] > TK))
       {
           k = i;
           break;
       }
    }

    if (k<0)
    {
        Exception exception;
        exception.error << "The given temperature: "<< T <<" is not inside the specified interval/s for the Cp calculation.";
        exception.reason << "The temperature is not inside the specified interval for the substance "<< substance.symbol() << ".";
        exception.line = __LINE__;
        RaiseError(exception);
    }

    for (unsigned i=0; i<thermo_parameters.Cp_coeff[k].size(); i++)
    {
        ac[i] = thermo_parameters.Cp_coeff[k][i];
    }

//    ac = thermo_parameters.Cp_coeff[k];

    auto Cp = ( ac[0] + ac[1]*TK + ac[2]/(TK*TK) + ac[3]/(pow(TK,0.5)) + ac[4]*(TK*TK)
          + ac[5]*(TK*TK*TK) + ac[6]*(TK*TK*TK*TK) + ac[7]/(TK*TK*TK) + ac[8]/TK + ac[9]*(pow(TK,(1/2))) /*+ ac[10]*log(T)*/);

    // Phase transitions
    if (fabs(TK.val-TrK) > TEMPER_PREC)
    {
        for (unsigned j=0; j<=k; j++)
        {
            if ( j == k )
                TK = T.val/* + C_to_K*/;     // current T is the end T for phase transition Cp calculations
            else TK = thermo_parameters.temperature_intervals[j][1] /*+ C_to_K*/;        // takes the upper bound from the j-th Tinterval

            if( !j )
                TrK = substance.referenceT() /*+ C_to_K*/;            // if j=0 the first interval should contain the reference T (Tcr)
            else  TrK = thermo_parameters.temperature_intervals[j][0] /*+ C_to_K*/;    // if j>0 then we are in a different Tinterval and the reference T becomes the lower bound of the interval

            auto Tst2 = TrK * TrK;
            auto Tst3 = Tst2 * TrK;
            auto Tst4 = Tst3 * TrK;
            auto Tst05 = std::sqrt( TrK );

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

            G -= S * (TK - TrK);

            for (unsigned i=0; i<thermo_parameters.Cp_coeff[j].size(); i++)
            {
                ac[i] = thermo_parameters.Cp_coeff[j][i];
            }


            S += ( ac[0] * log( (TK/TrK) ) + ac[1] * (TK - TrK) + ac[2] * ( 1./Tst2 - 1./(TK*TK) ) / 2.
                 + ac[3] * 2. * ( 1./Tst05 - 1./(pow(TK,0.5)) ) + ac[4] * ( (TK*TK) - Tst2 ) / 2.
                 + ac[5] * ( (pow(TK,3)) - Tst3 ) / 3. + ac[6] * ( (pow(TK,4)) - Tst4 ) / 4.
                 + ac[7] * ( 1./ Tst3 - 1./ (pow(TK,3)) ) / 3. + ac[8] * (1. / TrK - 1. / TK )
                 + ac[9] * 2.* ( (pow(TK,0.5)) - Tst05 ) );

            G -= ( ac[0] * ( TK * log( (TK/TrK) ) - (TK - TrK) ) + ac[1] * (pow((TK - TrK),2)) / 2. + ac[2] * (pow((TK - TrK),2)) / TK / Tst2 / 2.
                 + ac[3] * 2. * ((pow(TK,0.5)) - Tst05)*((pow(TK,0.5)) - Tst05) / Tst05 + ac[4] * ( (pow(TK,3)) + 2.*Tst3 - 3.* TK * Tst2 ) / 6.
                 + ac[5] * ( (pow(TK,4)) + 3.*Tst4 - 4.*TK * Tst3 ) / 12. + ac[6] * ( (pow(TK,4))*TK + 4.*Tst4*TrK - 5.*TK*Tst4 ) / 20.
                 + ac[7] * ( Tst3 - 3.* (pow(TK,2)) * TrK + 2.*(pow(TK,3)) ) / 6./ (pow(TK,2)) / Tst3 + ac[8] * ( (TK/TrK) - 1. - log( (TK/TrK) ))
                 + ac[9] * 2.* ( 2.* TK * (pow(TK,0.5)) - 3.* TK * Tst05 + TrK * Tst05 ) / 3. );

            H += ( ac[0] * (TK - TrK) + ac[1] * ( (pow(TK,2)) - Tst2 ) / 2. + ac[2] * ( 1./TrK - 1./TK )
                 + ac[3] * 2. * ( (pow(TK,0.5)) - Tst05 ) + ac[4] * ( (pow(TK,3)) - Tst3 ) / 3.
                 + ac[5] * ( (pow(TK,4)) - Tst4 ) / 4. + ac[6] * ( (pow(TK,4)) * TK - Tst4 * TrK ) / 5
                 + ac[7] * ( 1./ Tst2 - 1./ (pow(TK,2)) ) / 2. + ac[8] * log( (TK/TrK) )
                 + ac[9] * 2.* ( TK * (pow(TK,0.5)) - TrK * Tst05 ) / 3. );
        }
    }

    thermo_properties_PT.heat_capacity_cp   = Cp;
    thermo_properties_PT.gibbs_energy       = G;
    thermo_properties_PT.enthalpy           = H;
    thermo_properties_PT.entropy            = S;
    thermo_properties_PT.volume             = V;

    return thermo_properties_PT;
}

}
