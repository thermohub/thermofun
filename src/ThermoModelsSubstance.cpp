#include "ThermoModelsSubstance.h"
#include "Solute/SoluteHKFreaktoro.h"
#include "Solute/SoluteHKFgems.h"
#include "Solvent/WaterIdealGasWolley.h"
#include "Solute/SoluteADgems.h"
#include "Solids/SolidMurnaghanHP98.h"
#include "Solids/SolidBerman88.h"
#include "Solids/SolidBMGottschalk.h"

// TCorrPT includes
#include "Common/Exception.h"

namespace TCorrPT {


auto checkModelValidity(double T, double P, double Tmax, /*double Tmin,*/ double Pmax, /*double Pmin,*/ Substance species, string model) -> void
{
    // Check if given temperature is within the allowed range
    if(T < 0 /*Tmin*/ || T > Tmax)
    {
        Exception exception;
        exception.error << "Out of T bound in model "
              << model << " for substance " << species.symbol();
        exception.reason << "The provided temperature, " << T << " K,"  << "is either negative "
              "or greater than the maximum allowed, " << Tmax << " K.";
        RaiseError(exception);
    }

    // Check if given pressure is within the allowed range
    if(P < 0/*Pmin*/ || P > Pmax)
    {
        Exception exception;
        exception.error << "Out of P bound in model "
              << model << " for substance " << species.symbol();
        exception.reason << "The provided pressure, " << P << " Pa,"  << "is either negative "
              "or greater than the maximum allowed, " << Pmax << " Pa.";
        RaiseError(exception);
    }
}

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
    exception.reason << "The calculation method defined for the substance "<< pimpl->substance.symbol() << " is not available.";
    exception.line = __LINE__;
    RaiseError(exception);
}

//=======================================================================================================
// Akinfiev & Diamond EOS for neutral species
// References: Akinfiev & Diamond (2003)
// Added: DM 13.06.2016
//=======================================================================================================

struct SoluteAkinfievDiamondEOS::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

SoluteAkinfievDiamondEOS::SoluteAkinfievDiamondEOS(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto SoluteAkinfievDiamondEOS::thermoProperties(double T, double P, ThermoPropertiesSubstance tps, const ThermoPropertiesSubstance& wtp, const ThermoPropertiesSubstance& wigp, const PropertiesSolvent& wp) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    return thermoPropertiesAqSoluteAD(t, p, pimpl->substance, tps, wtp, wigp, wp);
}

//=======================================================================================================
// Calculates the ideal gas properties of pure H2O
// References: Woolley (1979)
// Added: DM 13.06.2016
//=======================================================================================================

struct WaterIdealGasWoolley::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

WaterIdealGasWoolley::WaterIdealGasWoolley(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto WaterIdealGasWoolley::thermoProperties(double T, double P) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    return waterIdealGas(t, p);
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

    checkModelValidity(t.val, p.val, 1273.15, 5e08, pimpl->substance, "HKFgems");

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

    checkModelValidity(t.val, p.val, 1273.15, 5e08, pimpl->substance, "HKFreaktoro");

    FunctionG g = functionG(t, p, wp);

    ElectroPropertiesSubstance aes = speciesElectroStateHKF(g, pimpl->substance);

    return thermoPropertiesAqSoluteHKFreaktoro(t, p, pimpl->substance, aes, wes);
}

//=======================================================================================================
// .....
// References: .....
// Added: DM 24.06.2016
//=======================================================================================================

struct MinMurnaghanEOSHP98::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

MinMurnaghanEOSHP98::MinMurnaghanEOSHP98(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto MinMurnaghanEOSHP98::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    return thermoPropertiesMinMurnaghanEOSHP98(t, p, pimpl->substance, tps);
}

//=======================================================================================================
// .....
// References: .....
// Added: DM 24.06.2016
//=======================================================================================================

struct MinBerman88::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

MinBerman88::MinBerman88(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto MinBerman88::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    return thermoPropertiesMinBerman88(t, p, pimpl->substance, tps);
}

//=======================================================================================================
// .....
// References: .....
// Added: DM 24.06.2016
//=======================================================================================================

struct MinBMGottschalk::Impl
{
    /// the substance instance
   Substance substance;

   Impl()
   {}

   Impl(const Substance& substance)
   : substance(substance)
   {}
};

MinBMGottschalk::MinBMGottschalk(const Substance &substance)
: pimpl(new Impl(substance))
{}


auto MinBMGottschalk::thermoProperties(double T, double P, ThermoPropertiesSubstance tps) -> ThermoPropertiesSubstance
{
    auto t = Reaktoro::Temperature(T + C_to_K);
    auto p = Reaktoro::Pressure(P * bar_to_Pa);

    return thermoPropertiesMinBMGottschalk(t, p, pimpl->substance, tps);
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

//    double  T_Tst, Ts2, TT,
//           Tst2, Tst3, Tst4, Tst05;
    Reaktoro::ThermoScalar V;
    int k=-1;
    vector<double> ac;
    ac.resize(16);
    ac = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

//    auto TC = Reaktoro::Temperature(T);
    auto TK = Reaktoro::Temperature(T + C_to_K);
    auto Pb = Reaktoro::Pressure(P);

//    TC = T;              // get current T in Celsius
//    TK = TC + C_to_K;                       // curent T in Kelvin
//    Pb = P;              // current P in bar
    auto TrK = Reaktoro::Temperature(pimpl->substance.referenceT())/* + C_to_K*/;

    auto S = thermo_properties_PrTr.entropy;
    auto G = thermo_properties_PrTr.gibbs_energy;
    auto H = thermo_properties_PrTr.enthalpy;
//    autoV = 0; // ???
//    Cp = 0;

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
        exception.reason << "The temperature is not inside the specified interval for the substance "<< pimpl->substance.symbol() << ".";
        exception.line = __LINE__;
        RaiseError(exception);
    }

//    auto T2 = Reaktoro::Temperature(TK.val * TK.val);
//    auto T3 = Reaktoro::Temperature(T2.val * TK.val);
//    auto T4 = Reaktoro::Temperature(T3.val * TK.val);
//    auto T05 = Reaktoro::Temperature(std::sqrt( TK.val ));

    for (unsigned i=0; i<thermo_parameters.Cp_coeff[k].size(); i++)
    {
        ac[i] = thermo_parameters.Cp_coeff[k][i];
    }

//    ac = thermo_parameters.Cp_coeff[k];

    auto Cp = ( ac[0] + ac[1]*TK + ac[2]/(TK*TK) + ac[3]/(pow(TK,0.5)) + ac[4]*(TK*TK)
          + ac[5]*(TK*TK*TK) + ac[6]*(TK*TK*TK*TK) + ac[7]/(TK*TK*TK) + ac[8]/TK + ac[9]*(pow(TK,(1/2))) /*+ ac[10]*log(T)*/);

    // Phase transitions
    if (fabs(TK.val-TrK.val) > TEMPER_PREC)
    {
        for (unsigned j=0; j<=k; j++)
        {
            if ( j == k )
                TK = T + C_to_K;     // current T is the end T for phase transition Cp calculations
            else TK = thermo_parameters.temperature_intervals[j][1] /*+ C_to_K*/;        // takes the upper bound from the j-th Tinterval
//            T2 = Reaktoro::Temperature(TK.val * TK.val);
//            T3 = Reaktoro::Temperature(T2.val * TK.val);
//            T4 = Reaktoro::Temperature(T3.val * TK.val);
//            T05 = Reaktoro::Temperature(std::sqrt( TK.val ));
            if( !j )
                TrK = pimpl->substance.referenceT() /*+ C_to_K*/;            // if j=0 the first interval should contain the reference T (Tcr)
            else  TrK = thermo_parameters.temperature_intervals[j][0] /*+ C_to_K*/;    // if j>0 then we are in a different Tinterval and the reference T becomes the lower bound of the interval
            auto Tst2 = TrK * TrK;
            auto Tst3 = Tst2 * TrK;
            auto Tst4 = Tst3 * TrK;
            auto Tst05 = std::sqrt( TrK.val );
//            auto T_Tst = TK - TrK;
//            auto Ts2 = T_Tst * T_Tst;
//            auto TT = Reaktoro::Temperature(TK.val / TrK.val);

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
                 + ac[3] * 2. * ( 1./Tst05 - 1./(pow(TK,(1/2))) ) + ac[4] * ( (TK*TK) - Tst2 ) / 2.
                 + ac[5] * ( (TK*TK*TK) - Tst3 ) / 3. + ac[6] * ( (TK*TK*TK*TK) - Tst4 ) / 4.
                 + ac[7] * ( 1./ Tst3 - 1./ (TK*TK) ) / 3. + ac[8] * (1. / TrK - 1. / TK )
                 + ac[9] * 2.* ( (pow(TK,(1/2))) - Tst05 ) );

            G -= ( ac[0] * ( TK * log( (TK/TrK) ) - (TK - TrK) ) + ac[1] * ((TK-TrK)*(TK-TrK)) / 2. + ac[2] * ((TK-TrK)*(TK-TrK)) / TK / Tst2 / 2.
                 + ac[3] * 2. * ((pow(TK,(1/2))) - Tst05)*((pow(TK,(1/2))) - Tst05) / Tst05 + ac[4] * ( (TK*TK*TK) + 2.*Tst3 - 3.* TK * Tst2 ) / 6.
                 + ac[5] * ( (TK*TK*TK*TK) + 3.*Tst4 - 4.*TK * Tst3 ) / 12. + ac[6] * ( (TK*TK*TK*TK)*TK + 4.*Tst4*TrK - 5.*TK*Tst4 ) / 20.
                 + ac[7] * ( Tst3 - 3.* (TK*TK) * TrK + 2.*(TK*TK*TK) ) / 6./ (TK*TK) / Tst3 + ac[8] * ( (TK/TrK) - 1. - log( (TK/TrK) ))
                 + ac[9] * 2.* ( 2.* TK * (pow(TK,(1/2))) - 3.* TK * Tst05 + TrK * Tst05 ) / 3. );

            H += ( ac[0] * (TK - TrK) + ac[1] * ( (TK*TK) - Tst2 ) / 2. + ac[2] * ( 1./TrK - 1./TK )
                 + ac[3] * 2. * ( (pow(TK,(1/2))) - Tst05 ) + ac[4] * ( (TK*TK*TK) - Tst3 ) / 3.
                 + ac[5] * ( (TK*TK*TK*TK) - Tst4 ) / 4. + ac[6] * ( (TK*TK*TK*TK) * TK - Tst4 * TrK ) / 5
                 + ac[7] * ( 1./ Tst2 - 1./ (TK*TK) ) / 2. + ac[8] * log( (TK/TrK) )
                 + ac[9] * 2.* ( TK * (pow(TK,(1/2))) - TrK * Tst05 ) / 3. );
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
